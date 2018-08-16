/*
 * ptest.h
 *
 *  Created on: 16 авг. 2018 г.
 *      Author: sadko
 */

#ifndef INCLUDE_TEST_PTEST_H_
#define INCLUDE_TEST_PTEST_H_

#include <stdio.h>
#include <stddef.h>
#include <stdint.h>
#include <stdlib.h>
#include <time.h>

#define PTEST_BEGIN(group, name, time, iterations) \
        namespace test { \
        namespace ptest { \
        namespace name { \
            \
            using namespace ::test::ptest; \
            \
            class ptest_ ## name: public PerformanceTest { \
                public: \
                    explicit ptest_ ## name() : PerformanceTest(group, #name, time, iterations) {} \
                    \
                    virtual ~ptest_ ## name() {}

#define PTEST_LOOP(...) { \
        double __start = clock(); \
        double __time = 0.0f; \
        size_t __iterations = 0; \
        \
        do { \
            for (size_t i=0; i<__test_iterations; ++i) { \
                __VA_ARGS__; \
            } \
            /* Calculate statistics */ \
            __iterations   += __test_iterations; \
            __time          = (clock() - __start) / CLOCKS_PER_SEC; \
        } while (time < __test_time); \
        \
        printf("Time = %.1f/%.1f [s], iterations = %ld/%ld, performance = %.1f [i/s], average time = %.7f [ms/i]\n", \
            time, __test_time, \
            long(__iterations), long((__iterations * __test_time) / __time), \
            __iterations / time, (1000.0f * __time) / __iterations \
        ); \
    }


#define PTEST_FAIL(code)    \
        fprintf(stderr, "Performance test %s group %s has failed at file %s, line %d", \
                __test_name, __test_group, __FILE__, __LINE__); \
        exit(code));

#define PTEST_END \
        } performance_test;  /* ptest class */ \
        } /* namespace name */ \
        } /* namespace ptest */ \
        } /* namespace test */

namespace test
{
    namespace ptest
    {
        class PerformanceTest
        {
            private:
                static PerformanceTest    *__root;
                PerformanceTest           *__next;

            protected:
                const char *__test_group;
                const char *__test_name;
                size_t      __test_iterations;
                double      __test_time;

            public:
                explicit PerformanceTest(const char *group, const char *name, float time, size_t iterations);
                virtual ~PerformanceTest();

            public:
                inline const char *name() const     { return __test_name; }
                inline const char *group() const    { return __test_group; }
                virtual void execute() = 0;

            public:
                static inline PerformanceTest    *first() { return __root; };
        };

    }
}

#endif /* INCLUDE_TEST_PTEST_H_ */
