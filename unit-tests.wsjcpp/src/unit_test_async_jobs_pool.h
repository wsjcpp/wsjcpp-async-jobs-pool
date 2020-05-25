#ifndef UNIT_TEST_ASYNC_JOBS_POOL_H
#define UNIT_TEST_ASYNC_JOBS_POOL_H

#include <wsjcpp_unit_tests.h>

class UnitTestAsyncJobsPool : public WsjcppUnitTestBase {
    public:
        UnitTestAsyncJobsPool();
        virtual void init();
        virtual bool run();
};

#endif // UNIT_TEST_ASYNC_JOBS_POOL_H
