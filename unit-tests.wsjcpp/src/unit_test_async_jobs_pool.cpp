#include "unit_test_async_jobs_pool.h"
#include <vector>
#include <wsjcpp_core.h>
#include <wsjcpp_async_jobs_pool.h>


REGISTRY_UNIT_TEST(UnitTestJobsPool)

UnitTestJobsPool::UnitTestJobsPool()
    : UnitTestBase("UnitTestJobsPool") {
    //
}

// ----------------------------------------------------------------------

void UnitTestJobsPool::init() {
    // nothing
}

// ----------------------------------------------------------------------

class JobWaiterResult {
    public:
        JobWaiterResult() {
            m_nFinished = 0;
        }
        
        void onDone() {
            m_nFinished++;
        };

        void onFail(const std::string &sError) {
            WSJCppLog::err("JobWaiterResult", "Failed job");
        };

        int finishedJobs() {
            return m_nFinished;
        };
    private:
        int m_nFinished;
};

// ----------------------------------------------------------------------

class JobAsyncWaiter : public WSJCppAsyncJob {
    public:
        JobAsyncWaiter(int n, int ms, JobWaiterResult* pJobWaiterResult) : WSJCppAsyncJob("job-example") { 
            m_nNumber = n;
            m_nMilliseconds = ms;
            m_pJobWaiterResult = pJobWaiterResult;
        };

        virtual bool run(const std::string &sWorkerId) {
            WSJCppLog::info(sWorkerId, "begin job " + std::to_string(m_nNumber));
            std::this_thread::sleep_for(std::chrono::milliseconds(m_nMilliseconds));
            WSJCppLog::info(sWorkerId, "end job  " + std::to_string(m_nNumber));
            m_pJobWaiterResult->onDone();
        }

    private:
        int m_nMilliseconds;
        JobWaiterResult* m_pJobWaiterResult;
        int m_nNumber;
};

// ----------------------------------------------------------------------

bool UnitTestJobsPool::run() {
    WSJCppAsyncJobsPool::start();

    // TEST waitForDone
    int nCountJobs = 5;
    WSJCppLog::info(TAG, "Check waitForDone...");
    JobWaiterResult *pJobWaiterResult = new JobWaiterResult();
    for (int i = 0; i < nCountJobs; i++) {
        WSJCppAsyncJobsPool::addJobFast(new JobAsyncWaiter(i, 500, pJobWaiterResult));
    }

    WSJCppAsyncJobsPool::waitForDone();
    if (pJobWaiterResult->finishedJobs() != nCountJobs) {
        WSJCppLog::err(TAG, "Test waitForDone FAILED expected " + std::to_string(nCountJobs) + ", but got " + std::to_string(pJobWaiterResult->finishedJobs()));
        return false;
    }

    return true;
}

