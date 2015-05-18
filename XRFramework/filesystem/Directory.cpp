#include "stdafxf.h"
#include "Directory.h"
#include "FileItem.h"
#include "Win32Directory.h"
#include "SpecialProtocolDirectory.h"
#include "log/Log.h"
#include "utils/Job.h"
#include "utils/JobManager.h"
#include "utils/StringUtils.h"
#include "XRThreads/Event.h"
#include "XRFwx/main.h"
#include "XRFwx/MainFrame.h"

extern MyApp& wxGetApp();

#define TIME_TO_BUSY_DIALOG 500

class GetListDirectory
{
private:

	struct Result
	{
		Result(const CURL& dir, const CURL& listDir) : m_event(true), m_dir(dir), m_listDir(listDir), m_result(false) {}
		CEvent        m_event;
		CFileItemList m_list;
		CURL          m_dir;
		CURL          m_listDir;
		bool          m_result;
	};

	struct GetJob
		: CJob
	{
		GetJob(std::shared_ptr<IDirectory>& imp
			, std::shared_ptr<Result>& result)
			: m_result(result)
			, m_imp(imp)
		{}
	public:
		virtual bool DoWork()
		{
			m_result->m_list.SetURL(m_result->m_listDir);
			m_result->m_result = m_imp->GetDirectory(m_result->m_dir, m_result->m_list);
			m_result->m_event.Set();
			return m_result->m_result;
		}

		std::shared_ptr<Result>    m_result;
		std::shared_ptr<IDirectory> m_imp;
	};

public:

	GetListDirectory(std::shared_ptr<IDirectory>& imp, const CURL& dir, const CURL& listDir)
		: m_result(new Result(dir, listDir))
	{
		m_id = CJobManager::GetInstance().AddJob(new GetJob(imp, m_result)
			, NULL
			, CJob::PRIORITY_HIGH);
	}
	~GetListDirectory()
	{
		CJobManager::GetInstance().CancelJob(m_id);
	}

	bool Wait(unsigned int timeout)
	{
		return m_result->m_event.WaitMSec(timeout);
	}

	bool GetFileItemList(CFileItemList& list)
	{
		/* if it was not finished or failed, return failure */
		if (!m_result->m_event.WaitMSec(0) || !m_result->m_result)
		{
			list.Clear();
			return false;
		}

		list.Copy(m_result->m_list);
		return true;
	}
	std::shared_ptr<Result> m_result;
	unsigned int               m_id;
};


CDirectory::CDirectory()
{}


CDirectory::~CDirectory()
{}

bool CDirectory::GetDirectory(const std::string& strPath, CFileItemList &items, const std::string &strMask /*=""*/, int flags /*=DIR_FLAG_DEFAULTS*/, bool allowThreads /* = false */)
{
	Hints hints;
	hints.flags = flags;
	hints.mask = strMask;
	return GetDirectory(strPath, items, hints, allowThreads);
}

bool CDirectory::GetDirectory(const std::string& strPath, CFileItemList &items, const Hints &hints, bool allowThreads)
{
	const CURL pathToUrl(strPath);
	return GetDirectory(pathToUrl, items, hints, allowThreads);
}

bool CDirectory::GetDirectory(const CURL& url, CFileItemList &items, const std::string &strMask /*=""*/, int flags /*=DIR_FLAG_DEFAULTS*/, bool allowThreads /* = false */)
{
	Hints hints;
	hints.flags = flags;
	hints.mask = strMask;
	return GetDirectory(url, items, hints, allowThreads);
}

bool CDirectory::GetDirectory(const CURL& url, CFileItemList &items, const Hints &hints, bool allowThreads)
{
	try
	{
		CURL realURL = CURL(url);

		std::shared_ptr<IDirectory> pDirectory(DirectoryFactory(realURL));
		if (!pDirectory.get())
			return false;

		pDirectory->SetFlags(hints.flags);
		bool result = false, cancel = false;

		while (!result && !cancel)
		{
			const std::string pathToUrl(url.Get());
			if (/*wxGetApp().IsCurrentThread() && */allowThreads && UrlUtils::IsSpecial(pathToUrl))
			{

				GetListDirectory get(pDirectory, realURL, url);
				if (!get.Wait(20))
				{
					while (!get.Wait(10))
					{
						// update progress
						float progress = pDirectory->GetProgress();
						/*if (progress > 0)*/
							/*wxGetApp().m_mainFrame->SetStatusText(StringUtils::Format("Ladowanie katalogu.... %i", progress));*/

							//g_iApplication->GetStatusBar()->SetText(StringUtils::Format("Ladowanie katalogu.... %i", progress));
					}
				}

				result = get.GetFileItemList(items);
			}
			else
			{
				items.SetURL(url);
				result = pDirectory->GetDirectory(realURL, items);
			}

			if (!result)
			{
				if (!cancel /*&& wxGetApp().IsCurrentThread()*/)
					continue;
				LOGERR("Error getting %s.", url.GetRedacted().c_str());
				return false;
			}
		}
		if (!pDirectory->AllowAll()) 
		{
			pDirectory->SetMask(hints.mask);
			for (int i = 0; i < items.Size(); ++i)
			{
				CFileItemPtr item = items[i];
				if (!item->m_bIsFolder && !pDirectory->IsAllowed(item->GetURL()))
				{
					items.Remove(i);
					i--; // don't confuse loop
				}
			}
		}
		// filter hidden files
		// TODO: we shouldn't be checking the gui setting here, callers should use getHidden instead
		if (!(hints.flags & DIR_FLAG_GET_HIDDEN))
			{
			for (int i = 0; i < items.Size(); ++i)
			{
				if (items[i]->GetProperty("file:hidden").asBoolean())
				{
					items.Remove(i);
					i--; // don't confuse loop
				}
			}
		}

		return true;

	}
	catch (...) 
	{
		LOGERR("Unhandled exception!");
	}
	LOGERR("Error getting %s", url.GetRedacted().c_str());
	return false;
}


bool CDirectory::Create(const std::string& strPath)
{
	const CURL pathToUrl(strPath);
	return Create(pathToUrl);
}

bool CDirectory::Create(const CURL& url)
{
	try
	{
		CURL realURL(url);
		std::auto_ptr<IDirectory> pDirectory(DirectoryFactory(realURL));
		if (pDirectory.get())
			if (pDirectory->Create(realURL))
				return true;
	}
	catch (...) 
	{
		LOGERR("Unhandled exception!");
	}
	LOGERR("Error creating %s", url.GetRedacted().c_str());
	return false;
}

bool CDirectory::Exists(const std::string& strPath, bool bUseCache /* = true */)
{
	const CURL pathToUrl(strPath);
	return Exists(pathToUrl, bUseCache);
}

bool CDirectory::Exists(const CURL& url, bool bUseCache /* = true */)
{
	try
	{
		CURL realURL(url);
		std::auto_ptr<IDirectory> pDirectory(DirectoryFactory(realURL));
		if (pDirectory.get())
			return pDirectory->Exists(realURL);
	}
	catch (...) 
	{
		LOGERR("Unhandled exception!");
	}
	LOGERR("Error checking for %s", url.GetRedacted().c_str());
	return false;
}

bool CDirectory::Remove(const std::string& strPath)
{
	const CURL pathToUrl(strPath);
	return Remove(pathToUrl);
}

bool CDirectory::Remove(const CURL& url)
{
	try
	{
		CURL realURL(url);
		std::auto_ptr<IDirectory> pDirectory(DirectoryFactory(realURL));

		if (pDirectory.get())
			if (pDirectory->Remove(realURL))
				return true;
	}
	catch (...)	
	{
		LOGERR("Unhandled exception!");
	}
	LOGERR("Error removing %s folder.", url.GetRedacted().c_str());

	return false;
}

IDirectory* DirectoryFactory(const CURL& url) 
{

	CFileItem item(url.Get(), false);
	if (url.GetProtocol().empty() || url.IsProtocol("file")) return new Win32Directory();
	if (url.IsProtocol("special")) return new SpecialProtocolDirectory();

	LOGFATAL("Unsupported protocol for directory!");

	return nullptr;
}