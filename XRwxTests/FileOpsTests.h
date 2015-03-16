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
#include "filesystem/FileOperationJob.h"
#include "utils/JobManager.h"

class FileOpsTests
{
public:
	FileOpsTests();
	virtual ~FileOpsTests();

	FileItemPtr m_item;
	FileOperationJob* m_job;
};

