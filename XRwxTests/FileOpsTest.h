#pragma once
/*!
* \file FileOpsTests.h
* \date 2015/03/15 21:33
*
* \author XoRdi
* Contact: user@company.com
*
* \brief
*
* TODO: long description
*
* \note
*/

#include "log/Log.h"
#include "filesystem/File.h"
#include "filesystem/FileItem.h"
#include "XRFramework/filesystem/FileOperationJob.h"
#include "utils/JobManager.h"

class FileOpsTests : public IJobCallback
{
public:
	FileOpsTests();
	virtual ~FileOpsTests();
	virtual void OnJobComplete(unsigned int jobID, bool success, CJob *job) {
		LOGINFO("Job completed");
	}

	virtual void OnJobProgress(unsigned int jobID, unsigned int progress, unsigned int total, const CJob *job) {
		LOGINFO("Job ID:%u Progress:%u Total:%u", jobID, progress, total);
	}

	CFileItemPtr m_item;
	FileOperationJob* m_job;
};