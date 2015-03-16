#include "stdafxf.h"
#include "FileOpsTests.h"
#include "Base.h"



FileOpsTests::FileOpsTests()
{
	LOGINFO("File Ops. test running");


	FileItemList list;
	list.Add(FileItemPtr(new FileItem("http://joemonster.org/images/logo2.gif", false)));
	m_job = new FileOperationJob(FileOperationJob::ActionCopy, list, "special://app/logo.png");
	//CJobManager::GetInstance().AddJob(m_job, NULL);
}


FileOpsTests::~FileOpsTests()
{
	SAFE_DELETE(m_job);
}
