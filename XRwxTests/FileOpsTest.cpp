#include "stdafxf.h"
#include "FileOpsTest.h"
#include "Base.h"



FileOpsTests::FileOpsTests()
{
	LOGINFO("File Ops. test running");


	CFileItemList list;
	list.Add(CFileItemPtr(new CFileItem("http://mirrors.kodi.tv/releases/win32/kodi-14.2-Helix_rc1.exe", false)));
	m_job = new FileOperationJob(FileOperationJob::ActionCopy, list, "special://app/", true);
	CJobManager::GetInstance().AddJob(m_job, m_job);
}


FileOpsTests::~FileOpsTests()
{

}