#ifndef WSJCPP_ASYNC_JOBS_POOL_H
#define WSJCPP_ASYNC_JOBS_POOL_H

#include <string>
#include <mutex>
#include <deque>
#include <thread>
#include <vector>

class WsjcppAsyncJob {
    public:
        WsjcppAsyncJob(const std::string &sName);
        const std::string &name();
        virtual bool run(const std::string &sWorkerId) = 0;

    private:
        std::string m_sName;
};

// ---------------------------------------------------------------------

class WsjcppAsyncJobSchedule {
    public:
        // TODO
};

// ---------------------------------------------------------------------

class WsjcppAsyncJobDeque {
    public:
        WsjcppAsyncJob *pop();
        void push(WsjcppAsyncJob *pJobAsync);
        void cleanup();
        bool isEmpty();

    private:
        std::string TAG;

        std::mutex m_mtxJobsAsyncDeque;
        std::deque<WsjcppAsyncJob *> m_dequeJobsAsync;
};

// ---------------------------------------------------------------------

class WsjcppAsyncJobsThreadWorker {
    public:

        WsjcppAsyncJobsThreadWorker(const std::string &sName, WsjcppAsyncJobDeque *pDeque);

        void start();
        void stop();
        bool isBuzy();
        void run();    
    private:
        std::string TAG;
        std::string m_sName;
        WsjcppAsyncJobDeque *m_pDeque;
        bool m_bBuzy;
        bool m_bStop;
        pthread_t m_threadWorker;
};

// ---------------------------------------------------------------------

extern WsjcppAsyncJobDeque *g_pWsjcppAsyncJobsFastPool;
extern std::vector<WsjcppAsyncJobsThreadWorker *> *g_vWsjcppAsyncJobsFastWorkers;
extern int g_nMaxWsjcppAsyncJobFastWorker;

extern WsjcppAsyncJobDeque *g_pWsjcppAsyncJobsLongPool; // TODO
extern WsjcppAsyncJobDeque *g_pWsjcppAsyncJobsDelay; // TODO
extern WsjcppAsyncJobDeque *g_pWsjcppAsyncJobsCron; // TODO
// TODO control thread will be add delay and cron jobs to long

// TODO statistics
// TODO max thread different workers

class WsjcppAsyncJobsPool {
    public:
        static void initGlobalVariables();

        static void addJobSlow(WsjcppAsyncJob *pJobAsync);
        static void addJobFast(WsjcppAsyncJob *pJobAsync);
        static void addJobDelay(int nMilliseconds, WsjcppAsyncJob *pJobAsync);
        static void addJobShedule(
            WsjcppAsyncJobSchedule *pJobSchedule, 
            WsjcppAsyncJob *pJobAsync
        );

        static void stop();
        static void start();
        static void waitForDone();
        static void cleanup();
};

// ---------------------------------------------------------------------

#endif // WSJCPP_ASYNC_JOBS_POOL_H
