#pragma once

#include "FileItem.h"
#include "utils/Job.h"
#include "filesystem/File.h"
#include "utils/IJobCallback.h"


class FileOperationJob : public CJob, public IJobCallback
{
public:	
	enum FileAction
	{
		ActionCopy = 1,
		ActionMove,
		ActionDelete,
		ActionReplace, ///< Copy, emptying any existing destination directories first
		ActionCreateFolder,
		ActionDeleteFolder,
	};

	virtual void OnJobComplete(unsigned int jobID, bool success, CJob *job) {};

	FileOperationJob();

	FileOperationJob(FileAction action, CFileItemList & items,
		const std::string& strDestFile,
		bool displayProgress = false);

	void SetFileOperation(FileAction action, CFileItemList &items, const std::string &strDestFile);

	virtual bool DoWork();

	static std::string GetActionString(FileAction action);
	const char* GetType() const { return m_bDisplayProgress ? "filemanager" : ""; }
	const std::string &GetAverageSpeed()     const { return m_avgSpeed; }
	const std::string &GetCurrentOperation() const { return m_currentOperation; }
	const std::string &GetCurrentFile()      const { return m_currentFile; }
	const CFileItemList &GetItems()         const { return m_items; }
	FileAction GetAction() const            { return m_fileaction; }


	virtual bool operator==(const CJob *job) const;
	
private:
	class FileOperation : public IFileCallback
	{
	public:
		FileOperation(FileAction action, const std::string &strFileA, const std::string &strFileB, int64_t time);
		bool ExecuteOperation(FileOperationJob *base, double &current, double opWeight);
		void Debug();
		virtual bool OnFileCallback(void* pContext, int ipercent, float avgSpeed);
	private:
		FileAction m_action;
		std::string m_strFileA, m_strFileB;
		int64_t m_time;
	};

	friend class FileOperation;
	typedef std::vector<FileOperation> FileOperationList;

	bool DoProcess(FileAction action, CFileItemList& items, const std::string& strDestFile, FileOperationList &fileOperations, double &totalTime);
	bool DoProcessFolder(FileAction action, const std::string& strPath, const std::string& strDestFile, FileOperationList &fileOperations, double &totalTime);
	bool DoProcessFile(FileAction action, const std::string& strFileA, const std::string& strFileB, FileOperationList &fileOperations, double &totalTime);

	static inline bool CanBeRenamed(const std::string &strFileA, const std::string &strFileB);

private:
	//StatusBarPtr m_hStatusBar;
	bool		m_bDisplayProgress;
	FileAction	m_fileaction;

	std::string m_strDestFile,
				m_avgSpeed, 
				m_currentOperation, 
				m_currentFile;
	CFileItemList m_items;
};

