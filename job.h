#ifndef _JOB_H_
#define _JOB_H_

#include "random.h"
#include<mutex>

using namespace std;

//---------------------------------------------------------------------------------------------------
// Globals
//---------------------------------------------------------------------------------------------------
long long getCurrentTimestampInMilliseconds();
//---------------------------------------------------------------------------------------------------
// Enums and structures
//---------------------------------------------------------------------------------------------------
typedef enum
{
    STATE_INVALID,
    STATE_READY,
    STATE_RUNNING,
    STATE_BLOCKED,
    STATE_PAUSED,
    STATE_COMPLETED,

    // This should be last
    STATE_MAX
}jobState_en;


//---------------------------------------------------------------------------------------------------
// Job/Task class
//---------------------------------------------------------------------------------------------------
class Job
{
private:
    bool                    m_showJobStatus;          // Show details of job running status
    mutex                   m_jobMutex;               // Mutex for Job class
    unsigned long           m_jobId;                  // Unique Job ID
    jobState_en             m_state;                  // Current state of the job
    long long               m_tsCreated;              // timestamp value when this job was created.
    void                   (*m_funcPtr)();
    //void*                   m_param;

public:
    Job(void(*funcPtr)(), unsigned long jobId);
    ~Job();

    bool isDebugEnabled() { return m_showJobStatus; }
    unsigned long getJobId() { return m_jobId; }
    long long getJobTimeCreated() { return m_tsCreated; }
    jobState_en getJobState() { return m_state; }
    void setJobState(jobState_en state) { m_state = state; }
    bool isJobComplete() { return (m_state == STATE_COMPLETED); }

    void displayJobDetails();
    void executeJob() { m_funcPtr(); };

};

#endif
