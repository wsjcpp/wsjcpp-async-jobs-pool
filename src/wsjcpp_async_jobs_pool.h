#ifndef WSJCPP_ASYNC_JOBS_POOL_H
#define WSJCPP_ASYNC_JOBS_POOL_H

#include <string>
#include <mutex>
#include <deque>
#include <thread>
#include <vector>

class WSJCppAsyncJob {
    public:
        WSJCppAsyncJob(const std::string &sName);
        const std::string &name();
        virtual bool run(const std::string &sWorkerId) = 0;

    private:
        std::string m_sName;
};

// ---------------------------------------------------------------------

class WSJCppAsyncJobSchedule {
    public:
        // TODO
};

// ---------------------------------------------------------------------

class WSJCppAsyncJobDeque {
    public:
        WSJCppAsyncJob *pop();
        void push(WSJCppAsyncJob *pJobAsync);
        void cleanup();
        bool isEmpty();

    private:
        std::string TAG;

        std::mutex m_mtxJobsAsyncDeque;
        std::deque<WSJCppAsyncJob *> m_dequeJobsAsync;
};

// ---------------------------------------------------------------------

class WSJCppAsyncJobsThreadWorker {
    public:

        WSJCppAsyncJobsThreadWorker(const std::string &sName, WSJCppAsyncJobDeque *pDeque);

        void start();
        void stop();
        bool isBuzy();
        void run();    
    private:
        std::string TAG;
        std::string m_sName;
        WSJCppAsyncJobDeque *m_pDeque;
        bool m_bBuzy;
        bool m_bStop;
        pthread_t m_threadWorker;
};

// ---------------------------------------------------------------------

extern WSJCppAsyncJobDeque *g_pWSJCppAsyncJobsFastPool;
extern std::vector<WSJCppAsyncJobsThreadWorker *> *g_vWSJCppAsyncJobsFastWorkers;
extern int g_nMaxWSJCppAsyncJobFastWorker;

extern WSJCppAsyncJobDeque *g_pWSJCppAsyncJobsLongPool; // TODO
extern WSJCppAsyncJobDeque *g_pWSJCppAsyncJobsDelay; // TODO
extern WSJCppAsyncJobDeque *g_pWSJCppAsyncJobsCron; // TODO
// TODO control thread will be add delay and cron jobs to long

// TODO statistics
// TODO max thread different workers

class WSJCppAsyncJobsPool {
    public:
        static void initGlobalVariables();

        static void addJobSlow(WSJCppAsyncJob *pJobAsync);
        static void addJobFast(WSJCppAsyncJob *pJobAsync);
        static void addJobDelay(int nMilliseconds, WSJCppAsyncJob *pJobAsync);
        static void addJobShedule(
            WSJCppAsyncJobSchedule *pJobSchedule, 
            WSJCppAsyncJob *pJobAsync
        );

        static void stop();
        static void start();
        static void waitForDone();
        static void cleanup();
};

// ---------------------------------------------------------------------

#endif // WSJCPP_ASYNC_JOBS_POOL_H
