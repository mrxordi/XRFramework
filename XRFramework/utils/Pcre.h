#pragma once
#include "IJobCallback.h"
#include "PcreJob.h"
#include "utils/JobManager.h"

class CPcre : public IJobCallback
{
public:
	CPcre();
	virtual ~CPcre();
	virtual void OnMatchFound(const CPcreJob::JobContext& context, unsigned int progress, const CPcreJob* job) {};
};

