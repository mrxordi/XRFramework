#pragma once

#include "XRThreads/CriticalSection.h"
#include "XRThreads/SingleLock.h"
#include "XRCommon/log/Log.h"

#define CURL CURL_HANDLE
#include "curl/curl.h"
#undef CURL

//Exception class for curl exception

class CCurlException : std::exception
{
public:
	CCurlException(std::string message) :m_message(message) 
	{ 
		LOGFATAL(m_message.c_str());
	}
	CCurlException(CURLcode error)
	{
		m_message = curl_easy_strerror(error);
		LOGFATAL("Libcurl exception, error: %s", m_message.c_str());
	}
	const char* what() throw()
	{
		return m_message.c_str();
	}
	~CCurlException() throw() { }

private:
	std::string m_message;
};

class CCurlGlobal
{
	static bool Load();
	static void Unload();
public:
	CCurlGlobal();
	virtual ~CCurlGlobal();
	/* extend interface with buffered functions */
	void easy_aquire(const char *protocol, const char *hostname, CURL_HANDLE** easy_handle, CURLM** multi_handle);
	void easy_release(CURL_HANDLE** easy_handle, CURLM** multi_handle);
	void easy_duplicate(CURL_HANDLE* easy, CURLM* multi, CURL_HANDLE** easy_out, CURLM** multi_out);
	CURL_HANDLE* easy_duphandle(CURL_HANDLE* easy_handle);

	/*Checking for dead sessions*/
	static void CheckIdle();
	static void UnloadAll();


private:
	/* structure holding a session info*/
	typedef struct SSession
	{
		unsigned int  m_idletimestamp;  // timestamp of when this object when idle
		std::string    m_protocol;
		std::string    m_hostname;
		bool          m_busy;
		CURL_HANDLE*  m_easy;
		CURLM*        m_multi;
		CCurlGlobal*  m_parent;
	} SSession;

	/* Vector of sessions*/
	typedef std::vector<SSession*> VSessions;

private:
	SSession* m_session;

	static long s_curlReferences;
	static VSessions s_sessions;
	static XR::CCriticalSection s_critSessions;
};

