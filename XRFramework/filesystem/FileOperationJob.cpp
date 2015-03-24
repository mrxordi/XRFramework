#include "stdafxf.h"
#include "FileOperationJob.h"
#include "Directory.h"
#include "File.h"
#include "utils/StringUtils.h"


FileOperationJob::FileOperationJob()
{
	m_fileaction = ActionCopy;
	m_bDisplayProgress = false;
}

FileOperationJob::FileOperationJob(FileAction action, CFileItemList & items,
									const std::string& strDestFile,
									bool displayProgress) 
{
	m_bDisplayProgress = displayProgress;
	SetFileOperation(action, items, strDestFile);
}

void FileOperationJob::SetFileOperation(FileAction action, CFileItemList &items, const std::string &strDestFile) 
{
  m_fileaction = action;
  m_strDestFile = strDestFile;

  m_items.Clear();
  for (int i = 0; i < items.Size(); i++)
    m_items.Add(CFileItemPtr(new CFileItem(*items[i])));
}

bool FileOperationJob::DoWork() 
{
	FileOperationList ops;
	double totalTime = 0.0;

	if (m_bDisplayProgress) 
	{
		//m_hStatusBar = g_iApplication->GetStatusBar();
	}

	bool success = DoProcess(m_fileaction, m_items, m_strDestFile, ops, totalTime);

	unsigned int size = ops.size();

	double opWeight = 100.0 / totalTime;
	double current = 0.0;

	for (unsigned int i = 0; i < size && success; i++)
		success &= ops[i].ExecuteOperation(this, current, opWeight);

	//if (m_hStatusBar)
	//	m_hStatusBar->SetText("");

	return success;
}

std::string FileOperationJob::GetActionString(FileAction action) 
{
	std::string result;
	switch (action) 
	{
	case ActionCopy:
	case ActionReplace:
		result = "Kopiuje";
		break;
	case ActionMove:
		result = "Przenosze";
		break;
	case ActionDelete:
	case ActionDeleteFolder:
		result = "Kasuje";
		break;
	case ActionCreateFolder:
		result = "Nowy folder";
		break;
	}

	return result;
}

bool FileOperationJob::operator==(const CJob* job) const 
{
	if (strcmp(job->GetType(), GetType()) == 0) 
	{

		const FileOperationJob* rjob = dynamic_cast<const FileOperationJob*>(job);
		if (rjob) 
		{
			if (GetAction() == rjob->GetAction() &&
					m_strDestFile == rjob->m_strDestFile &&
					m_items.Size() == rjob->m_items.Size()) 
			{
				for (int i = 0; i<m_items.Size(); ++i) 
				{
					if (m_items[i]->GetPath() != rjob->m_items[i]->GetPath())
						return false;
				}
				return true;
			}
		}
	}
	return false;
}

bool FileOperationJob::DoProcess(FileAction action, CFileItemList & items, const std::string& strDestFile, FileOperationList &fileOperations, double &totalTime)
{
	for (int iItem = 0; iItem < items.Size(); ++iItem) 
	{
		CFileItemPtr pItem = items[iItem];

		std::string strNoSlash = pItem->GetPath();
		UrlUtils::RemoveSlashAtEnd(strNoSlash);
		std::string strFileName = UrlUtils::GetFileName(strNoSlash);

		std::string strnewDestFile;
		if (!strDestFile.empty()) // only do this if we have a destination
			strnewDestFile = UrlUtils::ChangeBasePath(pItem->GetPath(), strFileName, strDestFile); // Convert (URL) encoding + slashes (if source / target differ)

		if (pItem->m_bIsFolder) 
		{
			// in ActionReplace mode all subdirectories will be removed by the below
			// DoProcessFolder(ActionDelete) call as well, so ActionCopy is enough when
			// processing those
			FileAction subdirAction = (action == ActionReplace) ? ActionCopy : action;
			// create folder on dest. drive
			if (action != ActionDelete && action != ActionDeleteFolder)
				DoProcessFile(ActionCreateFolder, strnewDestFile, "", fileOperations, totalTime);
			if (action == ActionReplace && CDirectory::Exists(strnewDestFile))
				DoProcessFolder(ActionDelete, strnewDestFile, "", fileOperations, totalTime);
			if (!DoProcessFolder(subdirAction, pItem->GetPath(), strnewDestFile, fileOperations, totalTime))
				return false;
			if (action == ActionDelete || action == ActionDeleteFolder)
				DoProcessFile(ActionDeleteFolder, pItem->GetPath(), "", fileOperations, totalTime);
		}
		else 
			DoProcessFile(action, pItem->GetPath(), strnewDestFile, fileOperations, totalTime);
	}
	return true;
}

bool FileOperationJob::DoProcessFolder(FileAction action, const std::string& strPath, const std::string& strDestFile, FileOperationList &fileOperations, double &totalTime) 
{
	LOGDEBUG("FileManager, processing folder: %s", strPath.c_str());
	CFileItemList items;
	//m_rootDir.GetDirectory(strPath, items);
	CDirectory::GetDirectory(strPath, items, "", DIR_FLAG_NO_FILE_DIRS | DIR_FLAG_GET_HIDDEN);

	for (int i = 0; i < items.Size(); i++) 
	{
		CFileItemPtr pItem = items[i];
		LOGDEBUG("  -- %s", pItem->GetPath().c_str());
	}

	if (!DoProcess(action, items, strDestFile, fileOperations, totalTime)) 
		return false;

	if (action == ActionMove) 
	{
		fileOperations.push_back(FileOperation(ActionDeleteFolder, strPath, "", 1));
		totalTime += 1.0;
	}

	return true;
}

bool FileOperationJob::DoProcessFile(FileAction action, const std::string& strFileA, const std::string& strFileB, FileOperationList &fileOperations, double &totalTime) 
{
	int64_t time = 1;

	if (action == ActionCopy || action == ActionReplace || (action == ActionMove && !CanBeRenamed(strFileA, strFileB))) 
	{
		struct __stat64 data;
		if (CFile::Stat(strFileA, &data) == 0)
			time += data.st_size;
	}

	fileOperations.push_back(FileOperation(action, strFileA, strFileB, time));

	totalTime += time;

	return true;
}

inline bool FileOperationJob::CanBeRenamed(const std::string &strFileA, const std::string &strFileB) 
{
	if (UrlUtils::IsHD(strFileA) && UrlUtils::IsHD(strFileB))
		return true;

	return false;
}

////////////////////////////////////////////////////////////////////////////////////
// FileOperation
////////////////////////////////////////////////////////////////////////////////////

FileOperationJob::FileOperation::FileOperation(FileAction action, const std::string &strFileA, const std::string &strFileB, int64_t time) : m_action(action), m_strFileA(strFileA), m_strFileB(strFileB), m_time(time)
{}

struct DataHolder 
{
	FileOperationJob *base;
	double current;
	double opWeight;
};

bool FileOperationJob::FileOperation::ExecuteOperation(FileOperationJob *base, double &current, double opWeight) 
{
	bool bResult = true;

	base->m_currentFile = CURL(m_strFileA).GetFileNameWithoutPath();
	base->m_currentOperation = GetActionString(m_action);

	if (base->ShouldCancel((unsigned)current, 100))
		return false;

	LOGINFO("%s...%f %%", base->GetCurrentFile().c_str(), (float)current);

	DataHolder data = { base, current, opWeight };

	switch (m_action) 
	{
	case FileOperationJob::ActionCopy:
	case FileOperationJob::ActionReplace: 
	{
		LOGDEBUG("FileManager: copy %s -> %s\n", m_strFileA.c_str(), m_strFileB.c_str());
		bResult = CFile::Copy(m_strFileA, m_strFileB, this, &data);
		break;
	}
	case ActionMove: 
	{
		LOGDEBUG("FileManager: move %s -> %s\n", m_strFileA.c_str(), m_strFileB.c_str());

		if (CanBeRenamed(m_strFileA, m_strFileB))
			bResult = CFile::Rename(m_strFileA, m_strFileB);
		else if (CFile::Copy(m_strFileA, m_strFileB, this, &data))
			bResult = CFile::Delete(m_strFileA);
		else
			bResult = false;
		break;
	}
	case FileOperationJob::ActionDelete: 
	{
			LOGDEBUG("FileManager: delete %s\n", m_strFileA.c_str());
			bResult = CFile::Delete(m_strFileA);
			break;
	}
	case FileOperationJob::ActionDeleteFolder: 
	{
		LOGDEBUG("FileManager: delete folder %s\n", m_strFileA.c_str());
		bResult = CDirectory::Remove(m_strFileA);
		break;
	}
	case FileOperationJob::ActionCreateFolder: 
	{
			LOGDEBUG("FileManager: create folder %s\n", m_strFileA.c_str());
			bResult = CDirectory::Create(m_strFileA);
			break;
	}
	}

	current += (double)m_time * opWeight;
	return bResult;
}

void FileOperationJob::FileOperation::Debug() 
{
	printf("%i | %s > %s\n", m_action, m_strFileA.c_str(), m_strFileB.c_str());
}

bool FileOperationJob::FileOperation::OnFileCallback(void* pContext, int ipercent, float avgSpeed) 
{
	DataHolder *data = (DataHolder *)pContext;
	double current = data->current + ((double)ipercent * data->opWeight * (double)m_time) / 100.0;

	if (avgSpeed > 1000000.0f)
		data->base->m_avgSpeed = StringUtils::Format("%.1f MB/s", avgSpeed / 1000000.0f);
	else
		data->base->m_avgSpeed = StringUtils::Format("%.1f KB/s", avgSpeed / 1000.0f);

/*
	if (data->base->m_hStatusBar) 
	{
		std::string line;
		line = StringUtils::Format("%s...(%s) %f",
			data->base->GetCurrentFile().c_str(),
			data->base->GetAverageSpeed().c_str(), 
			(float)current);
		data->base->m_hStatusBar->SetText(line);	
	}*/
	return !data->base->ShouldCancel((unsigned)current, 100);
}


