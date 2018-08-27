/*
 * mix.cpp
 *
 *  Created on: 27 авг. 2018 г.
 *      Author: sadko
 */


#include <dsp/dsp.h>
#include <test/utest.h>
#include <test/FloatBuffer.h>

namespace native
{
    void mix2(float *dst, const float *src, float k1, float k2, size_t count);
    void mix3(float *dst, const float *src1, const float *src2, float k1, float k2, float k3, size_t count);
    void mix4(float *dst, const float *src1, const float *src2, const float *src3, float k1, float k2, float k3, float k4, size_t count);

    void mix_copy2(float *dst, const float *src1, const float *src2, float k1, float k2, size_t count);
    void mix_copy3(float *dst, const float *src1, const float *src2, const float *src3, float k1, float k2, float k3, size_t count);
    void mix_copy4(float *dst, const float *src1, const float *src2, const float *src3, const float *src4, float k1, float k2, float k3, float k4, size_t count);

    void mix_add2(float *dst, const float *src1, const float *src2, float k1, float k2, size_t count);
    void mix_add3(float *dst, const float *src1, const float *src2, const float *src3, float k1, float k2, float k3, size_t count);
    void mix_add4(float *dst, const float *src1, const float *src2, const float *src3, const float *src4, float k1, float k2, float k3, float k4, size_t count);
}

IF_ARCH_X86(
    namespace sse
    {
        void mix2(float *dst, const float *src, float k1, float k2, size_t count);
        void mix3(float *dst, const float *src1, const float *src2, float k1, float k2, float k3, size_t count);
        void mix4(float *dst, const float *src1, const float *src2, const float *src3, float k1, float k2, float k3, float k4, size_t count);

        void mix_copy2(float *dst, const float *src1, const float *src2, float k1, float k2, size_t count);
        void mix_copy3(float *dst, const float *src1, const float *src2, const float *src3, float k1, float k2, float k3, size_t count);
        void mix_copy4(float *dst, const float *src1, const float *src2, const float *src3, const float *src4, float k1, float k2, float k3, float k4, size_t count);

        void mix_add2(float *dst, const float *src1, const float *src2, float k1, float k2, size_t count);
        void mix_add3(float *dst, const float *src1, const float *src2, const float *src3, float k1, float k2, float k3, size_t count);
        void mix_add4(float *dst, const float *src1, const float *src2, const float *src3, const float *src4, float k1, float k2, float k3, float k4, size_t count);
    }
)

typedef void (* mix2_t)(float *dst, const float *src, float k1, float k2, size_t count);
typedef void (* mix3_t)(float *dst, const float *src1, const float *src2, float k1, float k2, float k3, size_t count);
typedef void (* mix4_t)(float *dst, const float *src1, const float *src2, const float *src3, float k1, float k2, float k3, float k4, size_t count);

typedef void (* mix_dst2_t)(float *dst, const float *src1, const float *src2, float k1, float k2, size_t count);
typedef void (* mix_dst3_t)(float *dst, const float *src1, const float *src2, const float *src3, float k1, float k2, float k3, size_t count);
typedef void (* mix_dst4_t)(float *dst, const float *src1, const float *src2, const float *src3, const float *src4, float k1, float k2, float k3, float k4, size_t count);


UTEST_BEGIN("dsp", mix)

    void call(const char *label, size_t align, mix2_t func1, mix2_t func2)
    {
        if (!UTEST_SUPPORTED(func1))
            return;
        if (!UTEST_SUPPORTED(func2))
            return;

        UTEST_FOREACH(count, 0, 1, 3, 4, 5, 8, 16, 24, 32, 33, 64, 47, 0x80, 0xfff)
        {
            for (size_t mask=0; mask <= 0x03; ++mask)
            {
                FloatBuffer dst1(count, align, mask & 0x01);
                FloatBuffer dst2(dst1);
                FloatBuffer src1(count, align, mask & 0x02);

                func1(dst1, src1, 2.0f, 3.0f, count);
                func2(dst2, src1, 2.0f, 3.0f, count);

                UTEST_ASSERT_MSG(src1.valid(), "Source buffer 1 corrupted");
                UTEST_ASSERT_MSG(dst1.valid(), "Destination buffer 1 corrupted");
                UTEST_ASSERT_MSG(dst2.valid(), "Destination buffer 2 corrupted");

                // Compare buffers
                if (!dst1.equals_absolute(dst2))
                {
                    src1.dump("src1");
                    dst1.dump("dst1");
                    dst2.dump("dst2");
                    UTEST_FAIL_MSG("Output of functions for test '%s' differs", label);
                }
            }
        }
    }

    void call(const char *label, size_t align, mix_dst2_t func1, mix_dst2_t func2)
    {
        if (!UTEST_SUPPORTED(func1))
            return;
        if (!UTEST_SUPPORTED(func2))
            return;

        UTEST_FOREACH(count, 0, 1, 3, 4, 5, 8, 16, 24, 32, 33, 64, 47, 0x80, 0xfff)
        {
            for (size_t mask=0; mask <= 0x07; ++mask)
            {
                FloatBuffer dst1(count, align, mask & 0x01);
                FloatBuffer dst2(dst1);
                FloatBuffer src1(count, align, mask & 0x02);
                FloatBuffer src2(count, align, mask & 0x04);

                func1(dst1, src1, src2, 2.0f, 3.0f, count);
                func2(dst2, src1, src2, 2.0f, 3.0f, count);

                UTEST_ASSERT_MSG(src1.valid(), "Source buffer 1 corrupted");
                UTEST_ASSERT_MSG(dst1.valid(), "Destination buffer 1 corrupted");
                UTEST_ASSERT_MSG(dst2.valid(), "Destination buffer 2 corrupted");

                // Compare buffers
                if (!dst1.equals_absolute(dst2))
                {
                    src1.dump("src1");
                    dst1.dump("dst1");
                    dst2.dump("dst2");
                    UTEST_FAIL_MSG("Output of functions for test '%s' differs", label);
                }
            }
        }
    }

    void call(const char *label, size_t align, mix3_t func1, mix3_t func2)
    {
        if (!UTEST_SUPPORTED(func1))
            return;
        if (!UTEST_SUPPORTED(func2))
            return;

        UTEST_FOREACH(count, 0, 1, 3, 4, 5, 8, 16, 24, 32, 33, 64, 47, 0x80, 0xfff)
        {
            for (size_t mask=0; mask <= 0x07; ++mask)
            {
                FloatBuffer dst1(count, align, mask & 0x01);
                FloatBuffer dst2(dst1);
                FloatBuffer src1(count, align, mask & 0x02);
                FloatBuffer src2(count, align, mask & 0x04);

                func1(dst1, src1, src2, 2.0f, 3.0f, 5.0f, count);
                func2(dst2, src1, src2, 2.0f, 3.0f, 5.0f, count);

                UTEST_ASSERT_MSG(src1.valid(), "Source buffer 1 corrupted");
                UTEST_ASSERT_MSG(src2.valid(), "Source buffer 2 corrupted");
                UTEST_ASSERT_MSG(dst1.valid(), "Destination buffer 1 corrupted");
                UTEST_ASSERT_MSG(dst2.valid(), "Destination buffer 2 corrupted");

                // Compare buffers
                if (!dst1.equals_absolute(dst2))
                {
                    src1.dump("src1");
                    src2.dump("src2");
                    dst1.dump("dst1");
                    dst2.dump("dst2");
                    UTEST_FAIL_MSG("Output of functions for test '%s' differs", label);
                }
            }
        }
    }

    void call(const char *label, size_t align, mix_dst3_t func1, mix_dst3_t func2)
    {
        if (!UTEST_SUPPORTED(func1))
            return;
        if (!UTEST_SUPPORTED(func2))
            return;

        UTEST_FOREACH(count, 0, 1, 3, 4, 5, 8, 16, 24, 32, 33, 64, 47, 0x80, 0xfff)
        {
            for (size_t mask=0; mask <= 0x0f; ++mask)
            {
                FloatBuffer dst1(count, align, mask & 0x01);
                FloatBuffer dst2(dst1);
                FloatBuffer src1(count, align, mask & 0x02);
                FloatBuffer src2(count, align, mask & 0x04);
                FloatBuffer src3(count, align, mask & 0x08);

                func1(dst1, src1, src2, src3, 2.0f, 3.0f, 5.0f, count);
                func2(dst2, src1, src2, src3, 2.0f, 3.0f, 5.0f, count);

                UTEST_ASSERT_MSG(src1.valid(), "Source buffer 1 corrupted");
                UTEST_ASSERT_MSG(src2.valid(), "Source buffer 2 corrupted");
                UTEST_ASSERT_MSG(src3.valid(), "Source buffer 3 corrupted");
                UTEST_ASSERT_MSG(dst1.valid(), "Destination buffer 1 corrupted");
                UTEST_ASSERT_MSG(dst2.valid(), "Destination buffer 2 corrupted");

                // Compare buffers
                if (!dst1.equals_absolute(dst2))
                {
                    src1.dump("src1");
                    src2.dump("src2");
                    src2.dump("src3");
                    dst1.dump("dst1");
                    dst2.dump("dst2");
                    UTEST_FAIL_MSG("Output of functions for test '%s' differs", label);
                }
            }
        }
    }

    void call(const char *label, size_t align, mix4_t func1, mix4_t func2)
    {
        if (!UTEST_SUPPORTED(func1))
            return;
        if (!UTEST_SUPPORTED(func2))
            return;

        UTEST_FOREACH(count, 0, 1, 3, 4, 5, 8, 16, 24, 32, 33, 64, 47, 0x80, 0xfff)
        {
            for (size_t mask=0; mask <= 0x0f; ++mask)
            {
                FloatBuffer dst1(count, align, mask & 0x01);
                FloatBuffer dst2(dst1);
                FloatBuffer src1(count, align, mask & 0x02);
                FloatBuffer src2(count, align, mask & 0x04);
                FloatBuffer src3(count, align, mask & 0x08);

                func1(dst1, src1, src2, src3, 2.0f, 3.0f, 5.0f, 7.0f, count);
                func2(dst2, src1, src2, src3, 2.0f, 3.0f, 5.0f, 7.0f, count);

                UTEST_ASSERT_MSG(src1.valid(), "Source buffer 1 corrupted");
                UTEST_ASSERT_MSG(src2.valid(), "Source buffer 2 corrupted");
                UTEST_ASSERT_MSG(src3.valid(), "Source buffer 3 corrupted");
                UTEST_ASSERT_MSG(dst1.valid(), "Destination buffer 1 corrupted");
                UTEST_ASSERT_MSG(dst2.valid(), "Destination buffer 2 corrupted");

                // Compare buffers
                if (!dst1.equals_absolute(dst2))
                {
                    src1.dump("src1");
                    src2.dump("src2");
                    src3.dump("src3");
                    dst1.dump("dst1");
                    dst2.dump("dst2");
                    UTEST_FAIL_MSG("Output of functions for test '%s' differs", label);
                }
            }
        }
    }

    void call(const char *label, size_t align, mix_dst4_t func1, mix_dst4_t func2)
    {
        if (!UTEST_SUPPORTED(func1))
            return;
        if (!UTEST_SUPPORTED(func2))
            return;

        UTEST_FOREACH(count, 0, 1, 3, 4, 5, 8, 16, 24, 32, 33, 64, 47, 0x80, 0xfff)
        {
            for (size_t mask=0; mask <= 0x1f; ++mask)
            {
                FloatBuffer dst1(count, align, mask & 0x01);
                FloatBuffer dst2(dst1);
                FloatBuffer src1(count, align, mask & 0x02);
                FloatBuffer src2(count, align, mask & 0x04);
                FloatBuffer src3(count, align, mask & 0x08);
                FloatBuffer src4(count, align, mask & 0x10);

                func1(dst1, src1, src2, src3, src4, 2.0f, 3.0f, 5.0f, 7.0f, count);
                func2(dst2, src1, src2, src3, src4, 2.0f, 3.0f, 5.0f, 7.0f, count);

                UTEST_ASSERT_MSG(src1.valid(), "Source buffer 1 corrupted");
                UTEST_ASSERT_MSG(src2.valid(), "Source buffer 2 corrupted");
                UTEST_ASSERT_MSG(src3.valid(), "Source buffer 3 corrupted");
                UTEST_ASSERT_MSG(src4.valid(), "Source buffer 4 corrupted");
                UTEST_ASSERT_MSG(dst1.valid(), "Destination buffer 1 corrupted");
                UTEST_ASSERT_MSG(dst2.valid(), "Destination buffer 2 corrupted");

                // Compare buffers
                if (!dst1.equals_absolute(dst2))
                {
                    src1.dump("src1");
                    src2.dump("src2");
                    src3.dump("src3");
                    src4.dump("src4");
                    dst1.dump("dst1");
                    dst2.dump("dst2");
                    UTEST_FAIL_MSG("Output of functions for test '%s' differs", label);
                }
            }
        }
    }

    UTEST_MAIN
    {
        // Do tests
        IF_ARCH_X86(call("mix2", 16, native::mix2, sse::mix2));
        IF_ARCH_X86(call("mix3", 16, native::mix3, sse::mix3));
        IF_ARCH_X86(call("mix4", 16, native::mix4, sse::mix4));

        IF_ARCH_X86(call("mix_copy2", 16, native::mix_copy2, sse::mix_copy2));
        IF_ARCH_X86(call("mix_copy3", 16, native::mix_copy3, sse::mix_copy3));
        IF_ARCH_X86(call("mix_copy4", 16, native::mix_copy4, sse::mix_copy4));

        IF_ARCH_X86(call("mix_add2", 16, native::mix_add2, sse::mix_add2));
        IF_ARCH_X86(call("mix_add3", 16, native::mix_add3, sse::mix_add3));
        IF_ARCH_X86(call("mix_add4", 16, native::mix_add4, sse::mix_add4));
    }
UTEST_END;

