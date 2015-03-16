#include "stdafxf.h"
#include "CurlGlobalWLocSession.h"
#include "XRThreads/SystemClock.h"


CCurlGlobalWLocSession::CCurlGlobalWLocSession()
{
	m_session = nullptr;
}

CCurlGlobalWLocSession::~CCurlGlobalWLocSession()
{
	XR::CSingleLock lock(s_critSessions);
	if (m_session && m_session->m_easy)
	{
		/* reset session so next caller doesn't reuse options, only connections */
		/* will reset verbose too so it won't print that it closed connections on cleanup*/
		curl_easy_reset(m_session->m_easy);
		m_session->m_busy = false;
		m_session->m_idletimestamp = XR::SystemClockMillis();
	}
}


void CCurlGlobalWLocSession::easy_aquire(const char *protocol, const char *hostname, CURL_HANDLE** easy_handle, CURLM** multi_handle)
{
	assert(easy_handle != NULL);
	XR::CSingleLock lock(s_critSessions);
	
	if (m_session && !m_session->m_busy)
	{
		/* allow reuse of requester is trying to connect to same host */
		/* curl will take care of any differences in username/password */
		if (m_session->m_protocol.compare(protocol) == 0 && m_session->m_hostname.compare(hostname) == 0)
		{
			m_session->m_busy = true;
			if (easy_handle)
			{
				if (!m_session->m_easy)
					m_session->m_easy = curl_easy_init();

				*easy_handle = m_session->m_easy;
			}

			if (multi_handle)
			{
				if (!m_session->m_multi)
					m_session->m_multi = curl_multi_init();

				*multi_handle = m_session->m_multi;
			}
			return;
		}
		else {
			for (auto& it : s_sessions)
			{
				if (!it->m_busy)
				{
					/* allow reuse of requester is trying to connect to same host */
					/* curl will take care of any differences in username/password */
					if (it->m_protocol.compare(protocol) == 0 && it->m_hostname.compare(hostname) == 0)
					{
						it->m_busy = true;
						if (easy_handle)
						{
							if (!it->m_easy)
								it->m_easy = curl_easy_init();

							*easy_handle = it->m_easy;
						}

						if (multi_handle)
						{
							if (!it->m_multi)
								it->m_multi = curl_multi_init();

							*multi_handle = it->m_multi;
						}
						m_session = it;
						m_session->m_parent = this;
						return;
					}
				}
			}
		}
	}
	XR::CSingleLock lock2(s_critSessions);

	SSession* session = new SSession;
	session->m_busy = true;
	session->m_protocol = protocol;
	session->m_hostname = hostname;
	session->m_parent = this;
	session->m_multi = nullptr;
	session->m_easy = nullptr;

	/* count up global interface counter */
	Load();

	if (easy_handle)
	{
		session->m_easy = curl_easy_init();
		*easy_handle = session->m_easy;
	}

	if (multi_handle)
	{
		session->m_multi = curl_multi_init();
		*multi_handle = session->m_multi;
	}

	m_session = session;
	s_sessions.push_back(session);


	LOGINFO("Created session to %s://%s (easy=%p, multi=%p)", protocol, hostname, (void*)session->m_easy, (void*)session->m_multi);
}

void CCurlGlobalWLocSession::easy_release(CURL_HANDLE** easy_handle, CURLM** multi_handle)
{
	XR::CSingleLock lock(s_critSessions);

	CURL_HANDLE* easy = NULL;
	CURLM*       multi = NULL;

	if (easy_handle)
	{
		easy = *easy_handle;
		*easy_handle = NULL;
	}

	if (multi_handle)
	{
		multi = *multi_handle;
		*multi_handle = NULL;
	}

	if (m_session && m_session->m_easy == easy && (multi == nullptr || m_session->m_multi == multi))
	{
		/* reset session so next caller doesn't reuse options, only connections */
		/* will reset verbose too so it won't print that it closed connections on cleanup*/
		curl_easy_reset(easy);
		m_session->m_busy = false;
		m_session->m_idletimestamp = XR::SystemClockMillis();
		m_session = nullptr;
		return;
	}

	for(auto& it : s_sessions)
	{
		if (it->m_easy == easy && (multi == NULL || it->m_multi == multi))
		{
			/* reset session so next caller doesn't reuse options, only connections */
			/* will reset verbose too so it won't print that it closed connections on cleanup*/
			curl_easy_reset(easy);
			it->m_busy = false;
			it->m_idletimestamp = XR::SystemClockMillis();
			return;
		}
	}

}

void CCurlGlobalWLocSession::easy_duplicate(CURL_HANDLE* easy, CURLM* multi, CURL_HANDLE** easy_out, CURLM** multi_out)
{
	XR::CSingleLock lock(s_critSessions);

	if (easy_out && easy)
		*easy_out = curl_easy_duphandle(easy);

	if (multi_out && multi)
		*multi_out = curl_multi_init();

	for (auto& it : s_sessions)
	{
		if (it->m_easy == easy)
		{
			SSession* session = new SSession(*it);
			if (easy_out && easy)
				session->m_easy = *easy_out;
			else
				session->m_easy = NULL;

			if (multi_out && multi)
				session->m_multi = *multi_out;
			else
				session->m_multi = NULL;

			session->m_parent = this;
			Load();
			s_sessions.push_back(session);
			m_session = session;
			return;
		}
	}
	return;
}

CURL_HANDLE* CCurlGlobalWLocSession::easy_duphandle(CURL_HANDLE* easy_handle)
{
	XR::CSingleLock lock(s_critSessions);

	if (!easy_handle)
		throw CCurlException("No handle to dup");

	for (auto& it : s_sessions)
	{
		if (it->m_easy == easy_handle)
		{
			SSession* session = new SSession(*it);

			session->m_easy = curl_easy_duphandle(easy_handle);
			Load();
			s_sessions.push_back(session);
			session->m_parent = this;
			m_session = session;
			return session->m_easy;
		}
	}
	return curl_easy_duphandle(easy_handle);
}


void CCurlGlobalWLocSession::CheckIdle()
{
	/* avoid locking section here, to avoid stalling gfx thread on loads*/
	if (s_curlReferences == 0)
		return;

	XR::CSingleLock lock(s_critSessions);
	/* 20 seconds idle time before closing handle */
	const unsigned int idletime = 30000;


	VSessions::iterator it = s_sessions.begin();
	while (it != s_sessions.end())
	{
		//unsigned int tickcount = XR::SystemClockMillis() - it->m_idletimestamp;
		if (! (*it)->m_busy && (XR::SystemClockMillis() - (*it)->m_idletimestamp > idletime))
		{
			LOGINFO("Closing session to %s://%s (easy=%p, multi=%p)\n", (*it)->m_protocol.c_str(), (*it)->m_hostname.c_str(), (void*)((*it)->m_easy), (void*)((*it)->m_multi));

			// It's important to clean up multi *before* cleaning up easy, because the multi cleanup
			// code accesses stuff in the easy's structure.
			if ((*it)->m_multi)
				curl_multi_cleanup((*it)->m_multi);
			if ((*it)->m_easy)
				curl_easy_cleanup((*it)->m_easy);

			if ((*it)->m_parent)
				(*it)->m_parent->m_session = nullptr;

			Unload();
			delete (*it);

			it = s_sessions.erase(it);
			continue;
		}
		it++;
	}
}

bool CCurlGlobalWLocSession::Load()
{
	XR::CSingleLock lock(s_critSessions);
	if (s_curlReferences > 0)
	{
		s_curlReferences++;
		return true;
	}

	if (curl_global_init(CURL_GLOBAL_ALL))
	{
		LOGERR("Error initializing libcurl");
		return false;
	}

	/* check idle will clean up the last one */
	s_curlReferences = 1;

	return true;
}

void CCurlGlobalWLocSession::Unload()
{
	XR::CSingleLock lock(s_critSessions);

	if (--s_curlReferences == 0)
		curl_global_cleanup();  // close libcurl
}

void CCurlGlobalWLocSession::UnloadAll()
{
	XR::CSingleLock lock(s_critSessions);

	CURL_HANDLE* easy = NULL;
	CURLM*       multi = NULL;
	VSessions::iterator it;

	for (auto& it = s_sessions.begin(); it < s_sessions.end();)
	{
		if ((*it)->m_easy)
		{
			curl_easy_reset((*it)->m_easy);
			(*it)->m_busy = false;
			// It's important to clean up multi *before* cleaning up easy, because the multi cleanup
			// code accesses stuff in the easy's structure.
			if ((*it)->m_multi)
				curl_multi_cleanup((*it)->m_multi);
			if ((*it)->m_easy)
				curl_easy_cleanup((*it)->m_easy);
			LOGINFO("Closing session to %s://%s (easy=%p, multi=%p)", (*it)->m_protocol.c_str(), (*it)->m_hostname.c_str(), (void*)(*it)->m_easy, (void*)(*it)->m_multi);

			if ((*it)->m_parent)
				(*it)->m_parent->m_session = nullptr;

			delete (*it);
			it = s_sessions.erase(it);

			Unload();
		}
	}

	for (int i = s_curlReferences; i > 0; i--)
		Unload();


}

XR::CCriticalSection CCurlGlobalWLocSession::s_critSessions;
CCurlGlobalWLocSession::VSessions CCurlGlobalWLocSession::s_sessions;
long CCurlGlobalWLocSession::s_curlReferences = 0;
