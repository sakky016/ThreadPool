#include "job.h"
#include "thread_pool.h"
#include "random.h" 
#include<Windows.h>

//---------------------------------------------------------------------------------------------------
// CONFIGURATIONS - Alter to vary simulation results
//---------------------------------------------------------------------------------------------------

// Threads to create for Job pool
const int THREADS_TO_SPAWN = 8;

// Jobs to simulate
const unsigned long long JOBS_TO_EXECUTE = 10000;

// this is the maximum value till which the dummy function
// will count.
const unsigned long long COUNT_MAX = ULLONG_MAX;

// Max sleep (ms.) duration of a job (NOT USED)
const unsigned long SLEEP_MAX = 1000;

// Use random sleep duration for creation thread
const bool USE_RANDOM_JOB_CREATION_SLEEP = false;

// Milliseconds before proceeding to create another job
const int JOB_CREATION_SLEEP_CONST = 10;

// Max time duration (ms) before proceeding to 
// create another job. This means that before creating
// a new job, the system will wait for any value in b/w
// 0 to JOB_CREATION_SLEEP_MAX-1 seconds.
const int JOB_CREATION_SLEEP_MAX = 10;


//---------------------------------------------------------------------------------------------------
// Globals
//---------------------------------------------------------------------------------------------------
unsigned long long g_totalJobs = 0;

//---------------------------------------------------------------------------------------------------
// Function that needs to be processed by the thread pool
//---------------------------------------------------------------------------------------------------
void func()
{
    static RandomGenerator rng;
    unsigned long long max = rng.generateRandomNumber(COUNT_MAX);
    for (unsigned long long i = 0; i < max; i++)
    {
        // DO NOTHING
        //Sleep(rng.generateRandomNumber(SLEEP_MAX));
    }
}

//******************************************************************************************
// @name                    : createJob
//
// @description             : Create a a job with a jobID and random priority.
//
// @returns                 : Pointer to created job
//********************************************************************************************
Job* createJob()
{
    Job *job = new Job(func, ++g_totalJobs);
    return job;
}

//******************************************************************************************
// @name                    : jobCreationThread
//
// @description             : This thread is spawned from main(). Creates jobs and adds it to
//                            the ready queue of the thread pool.
//
// @returns                 : Nothing
//********************************************************************************************
void jobCreationThread(ThreadPool *threadPool)
{
    static RandomGenerator rng;
    unsigned long long i = 0;

    while (i < JOBS_TO_EXECUTE)
    {
        Job *j = createJob();
        threadPool->addToReadyQueue(j);

        if (USE_RANDOM_JOB_CREATION_SLEEP)
        {
            // Wait for some random time duration before proceeding to
            // create another job
            Sleep(rng.generateRandomNumber(JOB_CREATION_SLEEP_MAX));
        }
        else
        {
            Sleep(rng.generateRandomNumber(JOB_CREATION_SLEEP_CONST));
        }

        i++;
    } // simulation complete

    printf("\n\n ** Simulation complete **\n\n");
    threadPool->displayStats();
}

//----------------------------------------------------------------------------------------
// M A I N 
// Simulates the ThreadPool with random jobs
// Instead of starting a new thread for every task to execute concurrently, the task can 
// be passed to a thread pool. As soon as the pool has any idle threads the task is assigned 
// to one of them and executed. Internally the tasks are inserted into a List which the threads 
// in the pool are retrieving from. When a new task is inserted into the list one of the idle 
// threads will fetch it successfully and execute it. The rest of the idle threads in the 
// pool will be blocked waiting tasks to arrive in the list.
//----------------------------------------------------------------------------------------
int main()
{
    ThreadPool *threadPool = new ThreadPool(THREADS_TO_SPAWN);

    // Spawn a thread to create jobs randomly. This should be done
    // before starting threads for ThreadPool.
    thread jobCreationThreadId = thread(jobCreationThread, threadPool);

    threadPool->startThreads();

    jobCreationThreadId.join();
    getchar();
}