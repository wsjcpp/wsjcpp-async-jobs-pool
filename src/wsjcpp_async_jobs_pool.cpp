#include "wsjcpp_async_jobs_pool.h"
#include <wsjcpp_core.h>

// ---------------------------------------------------------------------

WsjcppAsyncJob::WsjcppAsyncJob(const std::string &sName) {
    m_sName = sName;
}

// ---------------------------------------------------------------------

const std::string &WsjcppAsyncJob::name() {
    return m_sName;
}

// ---------------------------------------------------------------------

WsjcppAsyncJob *WsjcppAsyncJobDeque::pop() {
    std::lock_guard<std::mutex> guard(this->m_mtxJobsAsyncDeque);
    WsjcppAsyncJob *pJobAsync = nullptr;
    int nSize = m_dequeJobsAsync.size();
    if (nSize > 0) {
        pJobAsync = m_dequeJobsAsync.back();
        m_dequeJobsAsync.pop_back();
    }
    return pJobAsync;
}

// ----------------------------------------------------------------------

void WsjcppAsyncJobDeque::push(WsjcppAsyncJob *pJobAsync) {
    std::lock_guard<std::mutex> guard(this->m_mtxJobsAsyncDeque);
    if (m_dequeJobsAsync.size() > 20) {
        WsjcppLog::warn(TAG, " deque more than " + std::to_string(m_dequeJobsAsync.size()));
    }
    m_dequeJobsAsync.push_front(pJobAsync);
}

// ----------------------------------------------------------------------

void WsjcppAsyncJobDeque::cleanup() {
    std::lock_guard<std::mutex> guard(this->m_mtxJobsAsyncDeque);
    while (m_dequeJobsAsync.size() > 0) {
        delete m_dequeJobsAsync.back();
        m_dequeJobsAsync.pop_back();
    }
}

// ----------------------------------------------------------------------

bool WsjcppAsyncJobDeque::isEmpty() {
    std::lock_guard<std::mutex> guard(this->m_mtxJobsAsyncDeque);
    bool bRet = m_dequeJobsAsync.size() == 0;
    return bRet;
}

// ----------------------------------------------------------------------

void* processJobsThreadWorker(void *arg) {
    WsjcppAsyncJobsThreadWorker *pWorker = (WsjcppAsyncJobsThreadWorker *)arg;
    pthread_detach(pthread_self());
    pWorker->run();
    return 0;
}

// ----------------------------------------------------------------------

WsjcppAsyncJobsThreadWorker::WsjcppAsyncJobsThreadWorker(const std::string &sName, WsjcppAsyncJobDeque *pDeque) {
    TAG = "WsjcppAsyncJobsThreadWorker-" + sName;
    m_pDeque = pDeque;
    m_bStop = false;
    m_bBuzy = false;
    m_sName = sName;
}

// ----------------------------------------------------------------------

void WsjcppAsyncJobsThreadWorker::start() {
    m_bStop = false;
    pthread_create(&m_threadWorker, NULL, &processJobsThreadWorker, (void *)this);
}

// ----------------------------------------------------------------------

void WsjcppAsyncJobsThreadWorker::stop() {
    m_bStop = true;
}

// ----------------------------------------------------------------------

bool WsjcppAsyncJobsThreadWorker::isBuzy() {
    return m_bBuzy;
}

// ----------------------------------------------------------------------

void WsjcppAsyncJobsThreadWorker::run() {
    while (1) {
        if (m_bStop) {
            m_bBuzy = false;
            return;
        }
        WsjcppAsyncJob *pJobAsync = m_pDeque->pop();
        while (pJobAsync != nullptr) {
            m_bBuzy = true;
            pJobAsync->run(TAG);
            delete pJobAsync;
            pJobAsync = m_pDeque->pop();
            if (m_bStop) {
                m_bBuzy = false;
                return;
            }
        }
        std::this_thread::sleep_for(std::chrono::milliseconds(100));
        if (m_bStop) {
            m_bBuzy = false;
            return;
        }
        m_bBuzy = false;
    }
}

// ----------------------------------------------------------------------

WsjcppAsyncJobDeque *g_pWsjcppAsyncJobsFastPool = nullptr;
std::vector<WsjcppAsyncJobsThreadWorker *> *g_vWsjcppAsyncJobsFastWorkers = nullptr;
int g_nMaxWsjcppAsyncJobFastWorker = 2;

void WsjcppAsyncJobsPool::initGlobalVariables() {
    if (g_pWsjcppAsyncJobsFastPool == nullptr) {
        g_pWsjcppAsyncJobsFastPool = new WsjcppAsyncJobDeque();
    }
    if (g_vWsjcppAsyncJobsFastWorkers == nullptr) {
        g_vWsjcppAsyncJobsFastWorkers = new std::vector<WsjcppAsyncJobsThreadWorker *>();
        // 2 threads default
        for (int i = 0; i < g_nMaxWsjcppAsyncJobFastWorker; i++) {
            g_vWsjcppAsyncJobsFastWorkers->push_back(new WsjcppAsyncJobsThreadWorker("fast-worker" + std::to_string(i), g_pWsjcppAsyncJobsFastPool));
        }
    }
}

// ----------------------------------------------------------------------

 // Two queue for fast jobs 
void WsjcppAsyncJobsPool::addJobFast(WsjcppAsyncJob *pJobAsync) {
    WsjcppAsyncJobsPool::initGlobalVariables();
    g_pWsjcppAsyncJobsFastPool->push(pJobAsync);
}

// ----------------------------------------------------------------------

// Long time jobs like a send a lot of emails and etc...
// better keep this list of jobs to storage
void WsjcppAsyncJobsPool::addJobSlow(WsjcppAsyncJob *pJobAsync) {
    WsjcppAsyncJobsPool::initGlobalVariables();
    WsjcppLog::warn("WsjcppAsyncJobsPool", "addJobSlow not implemented yet");
    // g_pJobsLong->push(pJobAsync);
}

// ----------------------------------------------------------------------

// jobs with delay
void WsjcppAsyncJobsPool::addJobDelay(int nMilliseconds, WsjcppAsyncJob *pJobAsync) {
    WsjcppAsyncJobsPool::initGlobalVariables();
    WsjcppLog::warn("WsjcppAsyncJobsPool", "addJobDelay not implemented yet");
    // g_pJobsShort->push(pJobAsync);
}

// ----------------------------------------------------------------------

// jobs by cron, for example every 5 minutes execute some job
void WsjcppAsyncJobsPool::addJobShedule(WsjcppAsyncJobSchedule *pJobSchedule, WsjcppAsyncJob *pJobAsync) {
    WsjcppAsyncJobsPool::initGlobalVariables();
    WsjcppLog::warn("WsjcppAsyncJobsPool", "addJobShedule not implemented yet");
    // g_pJobsShort->push(pJobAsync);
}

// ----------------------------------------------------------------------

void WsjcppAsyncJobsPool::stop() {
    WsjcppAsyncJobsPool::initGlobalVariables();
    for (int i = 0; i < g_vWsjcppAsyncJobsFastWorkers->size(); i++) {
        g_vWsjcppAsyncJobsFastWorkers->at(i)->stop();
    }
}

// ----------------------------------------------------------------------

void WsjcppAsyncJobsPool::start() {
    WsjcppAsyncJobsPool::initGlobalVariables();
    for (int i = 0; i < g_vWsjcppAsyncJobsFastWorkers->size(); i++) {
        g_vWsjcppAsyncJobsFastWorkers->at(i)->start();
    }
    // TODO slow
    // TODO thread for cron and delay
}

// ----------------------------------------------------------------------

void WsjcppAsyncJobsPool::waitForDone() {
    // TODO when job in progress need wait deque for progress jobs ?
    WsjcppAsyncJobsPool::initGlobalVariables();
    bool bBuzy = true;
    while (bBuzy) {
        bBuzy = false;
        std::this_thread::sleep_for(std::chrono::milliseconds(100));
        if (!g_pWsjcppAsyncJobsFastPool->isEmpty()) {
            bBuzy = true;
        }
        for (int i = 0; i < g_vWsjcppAsyncJobsFastWorkers->size(); i++) {
            if (g_vWsjcppAsyncJobsFastWorkers->at(i)->isBuzy()) {
                bBuzy = true;
            }
        }
    }
}

// ----------------------------------------------------------------------

void WsjcppAsyncJobsPool::cleanup() {
    g_pWsjcppAsyncJobsFastPool->cleanup();
    // TODO slow
    // TODO thread for cron and delay
}

// ----------------------------------------------------------------------
