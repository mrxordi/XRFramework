#include "stdafxf.h"
#include "Job.h"
#include "JobManager.h"

bool CJob::ShouldCancel(unsigned int progress, unsigned int total) const
{
	if (m_callback)
		return m_callback->OnJobProgress(progress, total, this);
	return false;
}
