#pragma once 

class CJob;
/*!
\ingroup jobs
\brief Callback interface for asynchronous jobs.

Used by clients of the CJobManager to receive progress and completion notification of jobs.
Clients of small jobs wishing to perform actions on job completion should implement the
IJobCallback::OnJobComplete() function.  Clients of larger jobs may choose to implement the
IJobCallback::OnJobProgress() function in order to be kept informed of progress.

\sa CJobManager and CJob
*/
class IJobCallback
{
public:
	/*!
	\brief Destructor for job call back objects.

	\sa CJobManager and CJob
	*/
	virtual ~IJobCallback() {};

	/*!
	\brief The callback used when a job completes.

	OnJobComplete is called at the completion of the job's DoWork() function, and is used
	to return information to the caller on the result of the job.  On returning form this function
	the CJobManager will destroy this job.

	\param jobID the unique id of the job (as retrieved from CJobManager::AddJob)
	\param success the result from the DoWork call
	\param job the job that has been processed.  The job will be destroyed after this function returns
	\sa CJobManager and CJob
	*/
	virtual void OnJobComplete(unsigned int jobID, bool success, CJob *job) = 0;

	/*!
	\brief An optional callback function that a job may call while processing.

	OnJobProgress may be called periodically by a job during it's DoWork() function.  It is used
	by the job to report on progress.

	\param jobID the unique id of the job (as retrieved from CJobManager::AddJob)
	\param progress the current progress of the job, out of total.
	\param total the total amount of work to be processed.
	\param job the job that has been processed.
	\sa CJobManager and CJob
	*/
	virtual void OnJobProgress(unsigned int jobID, unsigned int progress, unsigned int total, const CJob *job) {};
};