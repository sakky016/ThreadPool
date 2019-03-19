#ifndef _THREAD_POOL_H
#define _THREAD_POOL_H

#include <condition_variable>
#include <iostream>
#include<list>
#include <mutex>
#include<vector>
#include <stdio.h>
#include <thread>
#include<unordered_map>
#include "job.h"

using namespace std;

//---------------------------------------------------------------------------------
// Global functions
//---------------------------------------------------------------------------------
long long getCurrentTimestampInMilliseconds();

//---------------------------------------------------------------------------------
// User Defined Datatypes
//---------------------------------------------------------------------------------
typedef enum
{
    THREAD_STATE_IDLE,
    THREAD_STATE_RUNNING
}thread_states_en;

typedef struct threadDetails_tag
{
    unsigned long long jobsCompleted;
    long long idleTime;
    long long startTime;
    long long totalJobTime;
}threadDetails_t;

//---------------------------------------------------------------------------------
// ThreadPool class
//---------------------------------------------------------------------------------
class ThreadPool
{
private:
    long long                                          m_tsExecutionStart;         // Timestamp of first execution start
    bool                                               m_bExecutionStarted;
    int                                                m_numThreads;
    unordered_map<thread::id, thread*>                 m_threadData;
    unordered_map<thread::id, thread_states_en>        m_threadStates;
    unordered_map<thread::id, threadDetails_t>         m_threadDetails;
    condition_variable                                 m_conditionVar;
    mutex                                              m_mtx;

    list<Job*>                                         m_readyJobPool;             // List of jobs currently present in Ready queue
    list<Job*>                                         m_pendingJobPool;           // List of all the pending jobs
    unordered_map<unsigned long, Job*>                 m_completedJobPool;         // Using map as this can be huge.
    unsigned long long                                 m_totalJobsInflow;

public:
    ThreadPool(int numThreads);
    ~ThreadPool();

    int getNumThreads() { return m_numThreads; }

    bool addToReadyQueue(Job* job);
    int startThreads();
    static void * staticEntryPoint(void * threadPoolObj);
    void entryPoint();
    void displayStats();
};

#endif
