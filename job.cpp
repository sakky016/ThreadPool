#include <chrono>
#include "job.h"

RandomGenerator rng;                            // Random Generator

                                                // Get time stamp in milliseconds.
long long getCurrentTimestampInMilliseconds()
{
    long long ts_us = chrono::duration_cast<std::chrono::milliseconds>(std::chrono::high_resolution_clock::
                      now().time_since_epoch()).count();
    return ts_us;
}

//******************************************************************************************
// @name                    : Job
//
// @description             : Constructor
//
// @returns                 : Nothing
//******************************************************************************************
Job::Job(void (*funcPtr)(), unsigned long jobId)
{
    m_funcPtr = funcPtr;
    //m_param = param;
    m_jobId = jobId;
    m_state = STATE_READY;
    m_tsCreated = getCurrentTimestampInMilliseconds();

}

//******************************************************************************************
// @name                    : ~Job
//
// @description             : Destructor
//
// @returns                 : Nothing
//******************************************************************************************
Job::~Job()
{

}

//******************************************************************************************
// @name                    : displayJobDetails
//
// @description             : Display details of this job.
//
// @returns                 : Nothing
//******************************************************************************************
void Job::displayJobDetails()
{
    printf("\n");
    printf("Job ID             : %lu\n", m_jobId);
    printf("State              : %d\n", m_state);
    printf("Created at         : %lld\n", m_tsCreated);
    printf("Complete           : %s\n", isJobComplete() ? "YES" : "NO");
}
