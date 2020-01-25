#include "wsjcpp_async_jobs_pool.h"
#include <wsjcpp_core.h>

// ---------------------------------------------------------------------

WSJCppAsyncJob::WSJCppAsyncJob(const std::string &sName) {
    m_sName = sName;
}

// ---------------------------------------------------------------------

const std::string &WSJCppAsyncJob::name() {
    return m_sName;
}

// ---------------------------------------------------------------------

WSJCppAsyncJob *WSJCppAsyncJobDeque::pop() {
    std::lock_guard<std::mutex> guard(this->m_mtxJobsAsyncDeque);
    WSJCppAsyncJob *pJobAsync = nullptr;
    int nSize = m_dequeJobsAsync.size();
    if (nSize > 0) {
        pJobAsync = m_dequeJobsAsync.back();
        m_dequeJobsAsync.pop_back();
    }
    return pJobAsync;
}

// ----------------------------------------------------------------------

void WSJCppAsyncJobDeque::push(WSJCppAsyncJob *pJobAsync) {
    std::lock_guard<std::mutex> guard(this->m_mtxJobsAsyncDeque);
    if (m_dequeJobsAsync.size() > 20) {
        WSJCppLog::warn(TAG, " deque more than " + std::to_string(m_dequeJobsAsync.size()));
    }
    m_dequeJobsAsync.push_front(pJobAsync);
}

// ----------------------------------------------------------------------

void WSJCppAsyncJobDeque::cleanup() {
    std::lock_guard<std::mutex> guard(this->m_mtxJobsAsyncDeque);
    while (m_dequeJobsAsync.size() > 0) {
        delete m_dequeJobsAsync.back();
        m_dequeJobsAsync.pop_back();
    }
}

// ----------------------------------------------------------------------

bool WSJCppAsyncJobDeque::isEmpty() {
    std::lock_guard<std::mutex> guard(this->m_mtxJobsAsyncDeque);
    bool bRet = m_dequeJobsAsync.size() == 0;
    return bRet;
}

// ----------------------------------------------------------------------

void* processJobsThreadWorker(void *arg) {
    WSJCppAsyncJobsThreadWorker *pWorker = (WSJCppAsyncJobsThreadWorker *)arg;
    pthread_detach(pthread_self());
    pWorker->run();
    return 0;
}

// ----------------------------------------------------------------------

WSJCppAsyncJobsThreadWorker::WSJCppAsyncJobsThreadWorker(const std::string &sName, WSJCppAsyncJobDeque *pDeque) {
    TAG = "WSJCppAsyncJobsThreadWorker-" + sName;
    m_pDeque = pDeque;
    m_bStop = false;
    m_bBuzy = false;
    m_sName = sName;
}

// ----------------------------------------------------------------------

void WSJCppAsyncJobsThreadWorker::start() {
    m_bStop = false;
    pthread_create(&m_threadWorker, NULL, &processJobsThreadWorker, (void *)this);
}

// ----------------------------------------------------------------------

void WSJCppAsyncJobsThreadWorker::stop() {
    m_bStop = true;
}

// ----------------------------------------------------------------------

bool WSJCppAsyncJobsThreadWorker::isBuzy() {
    return m_bBuzy;
}

// ----------------------------------------------------------------------

void WSJCppAsyncJobsThreadWorker::run() {
    while (1) {
        if (m_bStop) {
            m_bBuzy = false;
            return;
        }
        WSJCppAsyncJob *pJobAsync = m_pDeque->pop();
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

WSJCppAsyncJobDeque *g_pWSJCppAsyncJobsFastPool = nullptr;
std::vector<WSJCppAsyncJobsThreadWorker *> *g_vWSJCppAsyncJobsFastWorkers = nullptr;
int g_nMaxWSJCppAsyncJobFastWorker = 2;

void WSJCppAsyncJobsPool::initGlobalVariables() {
    if (g_pWSJCppAsyncJobsFastPool == nullptr) {
        g_pWSJCppAsyncJobsFastPool = new WSJCppAsyncJobDeque();
    }
    if (g_vWSJCppAsyncJobsFastWorkers == nullptr) {
        g_vWSJCppAsyncJobsFastWorkers = new std::vector<WSJCppAsyncJobsThreadWorker *>();
        // 2 threads default
        for (int i = 0; i < g_nMaxWSJCppAsyncJobFastWorker; i++) {
            g_vWSJCppAsyncJobsFastWorkers->push_back(new WSJCppAsyncJobsThreadWorker("fast-worker" + std::to_string(i), g_pWSJCppAsyncJobsFastPool));
        }
    }
}

// ----------------------------------------------------------------------

 // Two queue for fast jobs 
void WSJCppAsyncJobsPool::addJobFast(WSJCppAsyncJob *pJobAsync) {
    WSJCppAsyncJobsPool::initGlobalVariables();
    g_pWSJCppAsyncJobsFastPool->push(pJobAsync);
}

// ----------------------------------------------------------------------

// Long time jobs like a send a lot of emails and etc...
// better keep this list of jobs to storage
void WSJCppAsyncJobsPool::addJobSlow(WSJCppAsyncJob *pJobAsync) {
    WSJCppAsyncJobsPool::initGlobalVariables();
    WSJCppLog::warn("WSJCppAsyncJobsPool", "addJobSlow not implemented yet");
    // g_pJobsLong->push(pJobAsync);
}

// ----------------------------------------------------------------------

// jobs with delay
void WSJCppAsyncJobsPool::addJobDelay(int nMilliseconds, WSJCppAsyncJob *pJobAsync) {
    WSJCppAsyncJobsPool::initGlobalVariables();
    WSJCppLog::warn("WSJCppAsyncJobsPool", "addJobDelay not implemented yet");
    // g_pJobsShort->push(pJobAsync);
}

// ----------------------------------------------------------------------

// jobs by cron, for example every 5 minutes execute some job
void WSJCppAsyncJobsPool::addJobShedule(WSJCppAsyncJobSchedule *pJobSchedule, WSJCppAsyncJob *pJobAsync) {
    WSJCppAsyncJobsPool::initGlobalVariables();
    WSJCppLog::warn("WSJCppAsyncJobsPool", "addJobShedule not implemented yet");
    // g_pJobsShort->push(pJobAsync);
}

// ----------------------------------------------------------------------

void WSJCppAsyncJobsPool::stop() {
    WSJCppAsyncJobsPool::initGlobalVariables();
    for (int i = 0; i < g_vWSJCppAsyncJobsFastWorkers->size(); i++) {
        g_vWSJCppAsyncJobsFastWorkers->at(i)->stop();
    }
}

// ----------------------------------------------------------------------

void WSJCppAsyncJobsPool::start() {
    WSJCppAsyncJobsPool::initGlobalVariables();
    for (int i = 0; i < g_vWSJCppAsyncJobsFastWorkers->size(); i++) {
        g_vWSJCppAsyncJobsFastWorkers->at(i)->start();
    }
    // TODO slow
    // TODO thread for cron and delay
}

// ----------------------------------------------------------------------

void WSJCppAsyncJobsPool::waitForDone() {
    // TODO when job in progress need wait deque for progress jobs ?
    WSJCppAsyncJobsPool::initGlobalVariables();
    bool bBuzy = true;
    while (bBuzy) {
        bBuzy = false;
        std::this_thread::sleep_for(std::chrono::milliseconds(100));
        if (!g_pWSJCppAsyncJobsFastPool->isEmpty()) {
            bBuzy = true;
        }
        for (int i = 0; i < g_vWSJCppAsyncJobsFastWorkers->size(); i++) {
            if (g_vWSJCppAsyncJobsFastWorkers->at(i)->isBuzy()) {
                bBuzy = true;
            }
        }
    }
}

// ----------------------------------------------------------------------

void WSJCppAsyncJobsPool::cleanup() {
    g_pWSJCppAsyncJobsFastPool->cleanup();
    // TODO slow
    // TODO thread for cron and delay
}

// ----------------------------------------------------------------------
