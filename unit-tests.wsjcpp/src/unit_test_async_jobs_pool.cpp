#include <wsjcpp_unit_tests.h>
#include <wsjcpp_async_jobs_pool.h>

class UnitTestAsyncJobsPool : public WsjcppUnitTestBase {
    public:
        UnitTestAsyncJobsPool();
        virtual bool doBeforeTest();
        virtual void executeTest();
        virtual bool doAfterTest();
};

REGISTRY_WSJCPP_UNIT_TEST(UnitTestAsyncJobsPool)

UnitTestAsyncJobsPool::UnitTestAsyncJobsPool()
    : WsjcppUnitTestBase("UnitTestAsyncJobsPool") {
    //
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
            WsjcppLog::err("JobWaiterResult", "Failed job");
        };

        int finishedJobs() {
            return m_nFinished;
        };
    private:
        int m_nFinished;
};

// ----------------------------------------------------------------------

class JobAsyncWaiter : public WsjcppAsyncJob {
    public:
        JobAsyncWaiter(int n, int ms, JobWaiterResult* pJobWaiterResult) : WsjcppAsyncJob("job-example") { 
            m_nNumber = n;
            m_nMilliseconds = ms;
            m_pJobWaiterResult = pJobWaiterResult;
        };

        virtual bool run(const std::string &sWorkerId) {
            WsjcppLog::info(sWorkerId, "begin job " + std::to_string(m_nNumber));
            std::this_thread::sleep_for(std::chrono::milliseconds(m_nMilliseconds));
            WsjcppLog::info(sWorkerId, "end job  " + std::to_string(m_nNumber));
            m_pJobWaiterResult->onDone();
            return true;
        }

    private:
        int m_nMilliseconds;
        JobWaiterResult* m_pJobWaiterResult;
        int m_nNumber;
};

// ----------------------------------------------------------------------

bool UnitTestAsyncJobsPool::doBeforeTest() {
    // do nothing
    return true;
}

// ----------------------------------------------------------------------

void UnitTestAsyncJobsPool::executeTest() {
    WsjcppAsyncJobsPool::start();

    // TEST waitForDone
    int nCountJobs = 5;
    WsjcppLog::info(TAG, "Check waitForDone...");
    JobWaiterResult *pJobWaiterResult = new JobWaiterResult();
    for (int i = 0; i < nCountJobs; i++) {
        WsjcppAsyncJobsPool::addJobFast(new JobAsyncWaiter(i, 500, pJobWaiterResult));
    }

    WsjcppAsyncJobsPool::waitForDone();
    compare("Test waitForDone FAILED", pJobWaiterResult->finishedJobs(), nCountJobs);
}

// ----------------------------------------------------------------------

bool UnitTestAsyncJobsPool::doAfterTest() {
    // do nothing
    return true;
}

