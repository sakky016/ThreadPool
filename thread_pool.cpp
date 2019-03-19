#include "thread_pool.h"
#include "job.h"
#include <Windows.h>

//******************************************************************************************
// @name                    : staticEntryPoint
//
// @description             : This is the static thread entry point. This is used as a wrapper
//                            to call the ThreadPool member function entryPoint().
//
// @returns                 : Null
//******************************************************************************************
void * ThreadPool::staticEntryPoint(void * threadPoolObj)
{
    ((ThreadPool *)threadPoolObj)->entryPoint();
    return NULL;
}

//******************************************************************************************
// @name                    : entryPoint
//
// @description             : This is the thread entry point. This is responsible for
//                            execution of the tasks getting assigned to the work queue
//                            of ThreadPool. It waits for job to get assigned to 
//                            m_readyJobPool. On receiving it, moves into from IDLE to
//                            RUNNING state. Executes the function pointer registred for 
//                            this task. It is to be noted that the function that is 
//                            to be executed using this thread of execution should be
//                            independent of each other (so as not to introduce any
//                            deadlock condition.
//
//
// @returns                 : Nothing
//******************************************************************************************
void ThreadPool::entryPoint()
{
    thread::id threadId = std::this_thread::get_id();

    // Thread executes continuosly
    while (1)
    {
        // This thread will wait till we have a job
        // ready for execution.
        long long idleStart = getCurrentTimestampInMilliseconds();
        cout << "Thread " << threadId << " waiting...\n";
        m_threadStates[threadId] = THREAD_STATE_IDLE;
        unique_lock<std::mutex> lck(m_mtx);
        m_conditionVar.wait(lck, [&]()
        {
            return (m_readyJobPool.size() > 0);
        });

        // Thread wakes up, do the job of the thread.
        long long idleEnd = getCurrentTimestampInMilliseconds();
        Job *job = m_readyJobPool.front();

        printf("Thread #%u started executing Job [ %lu ].\n", threadId, job->getJobId());  totalJobTime
        m_threadStates[threadId] = THREAD_STATE_RUNNING;
        long long executionStart = getCurrentTimestampInMilliseconds();
        if (!m_bExecutionStarted)
        {
            m_bExecutionStarted = true;
            m_tsExecutionStart = executionStart;
        }

        job->executeJob();
        long long executionEnd = getCurrentTimestampInMilliseconds();

        m_threadDetails[threadId].totalJobTime += executionEnd - executionStart;
        m_threadDetails[threadId].jobsCompleted++;
        m_threadDetails[threadId].idleTime += (idleEnd - idleStart);
        
        m_readyJobPool.pop_front();
    } // End of while (1)
}

//******************************************************************************************
// @name                    : ThreadPool
//
// @description             : Constructor
//
// @returns                 : Nothing
//******************************************************************************************
ThreadPool::ThreadPool(int numThreads)
{
    printf("\nCreating Thread Pool with %d threads\n", numThreads);
    m_numThreads = numThreads;
    m_totalJobsInflow = 0;
    m_tsExecutionStart = getCurrentTimestampInMilliseconds();
    m_bExecutionStarted = false;
}

ThreadPool::~ThreadPool()
{

}

//******************************************************************************************
// @name                    : addToReadyQueue
//
// @param job                : Job that needs to be added.
//
// @description             : Adds the specified job to the ready queue of the ThreadPool.
//                            Jobs get added to the queue via this API continuously from
//                            the job creater into the pending jobs list. 
//
// @returns                 : Nothing
//********************************************************************************************
bool ThreadPool::addToReadyQueue(Job* job)
{
    m_mtx.lock();
    m_readyJobPool.push_back(job);
    m_totalJobsInflow++;

    // Notify to all the threads that an event is waiting
    // to be processed.
    m_conditionVar.notify_all();
    m_mtx.unlock();

    return true;
}

//******************************************************************************************
// @name                    : startThreads
//
// @description             : Creates number of threads as specified in the constructor. 
//                            The threads are created and they start in IDLE mode waiting
//                            for jobs to arrive in the ready queue.
//
// @returns                 : 0 on SUCCESS
//********************************************************************************************
int ThreadPool::startThreads()
{
    thread* th = nullptr;

    // Spawn threads
    long long threadCreationStartTime = getCurrentTimestampInMilliseconds();
    for (int i = 0; i < getNumThreads(); i++)
    {
        th = new thread(ThreadPool::staticEntryPoint, this);
        thread::id threadId = th->get_id();
        printf("Spawned thread #%u\n", threadId);

        // Initialize thread details
        m_threadData[threadId] = th;
        m_threadDetails[threadId].jobsCompleted = 0;
        m_threadDetails[threadId].startTime = threadCreationStartTime;
        m_threadDetails[threadId].idleTime = 0;
        m_threadDetails[threadId].totalJobTime = 0;
    }

    long long threadCreationEndTime = getCurrentTimestampInMilliseconds();
    printf("Time taken to spawn %d threads: %lld ms.\n", getNumThreads(), (threadCreationEndTime - threadCreationStartTime));

    // Wait for joining
    for (auto it = m_threadData.begin(); it != m_threadData.end(); it++)
    {
        thread *th = it->second;
        th->join();
    }

    return 0;
}

//******************************************************************************************
// @name                    : displayStats
//
// @description             : Displays statistics for the Thread Pool.
//
// @returns                 : Nothing
//********************************************************************************************
void ThreadPool::displayStats()
{
    printf("+---------------------------------------------------------------------------+\n");
    printf("|               Job completion statistics (using ThreadPool)                |\n");
    printf("+---------------------------------------------------------------------------+\n");

    // Global details
    long long totalTimeElapsed = getCurrentTimestampInMilliseconds() - m_tsExecutionStart;
    double globalThroughput = 0;
    if (m_totalJobsInflow != 0)
    {
        globalThroughput = ((double)m_totalJobsInflow) / totalTimeElapsed;
    }

    printf(">> Global details:\n\n");
    printf("Threads used              : %d\n", m_numThreads);
    printf("Total jobs completed      : %llu\n",  m_totalJobsInflow);
    printf("Total time elapsed        : %lld ms\n", totalTimeElapsed);
    printf("Global Throughput         : %lf per ms.\n", globalThroughput);

    printf("+---------------------------------------------------------------------------+\n");
    printf(">> Thread level  details:\n\n");
    for (auto it = m_threadDetails.begin(); it != m_threadDetails.end(); it++)
    {
        // Calculations
        threadDetails_t threadDetails = it->second;
        long long elapsedTime = getCurrentTimestampInMilliseconds() - threadDetails.startTime;
        double throughput = 0;
        if (threadDetails.jobsCompleted != 0)
        {
            throughput = ((double)threadDetails.jobsCompleted) / (elapsedTime - threadDetails.idleTime);
        }

        double avgTimeRequiredByJob = 0;
        if (threadDetails.jobsCompleted != 0)
        {
            avgTimeRequiredByJob = ((double)(threadDetails.totalJobTime)) / threadDetails.jobsCompleted;
        }

        // Thread level details
        printf("Thread ID                 : TID_%u\n", it->first);
        printf("Jobs Completed            : %llu\n", threadDetails.jobsCompleted);
        printf("Elapsed time              : %lld ms.\n", elapsedTime);
        printf("Idle time                 : %lld ms.\n", threadDetails.jobsCompleted ? threadDetails.idleTime: elapsedTime);
        printf("Running time              : %lld ms.\n", threadDetails.totalJobTime);
        printf("Avg time requred by job   : %lf ms.\n", avgTimeRequiredByJob),
        printf("Throughput                : %lf per ms.\n", throughput);
        printf("\n");
    }
    printf("+---------------------------------------------------------------------------+\n");

}