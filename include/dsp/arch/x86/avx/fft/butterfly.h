/*
 * butterfly.h
 *
 *  Created on: 10 дек. 2019 г.
 *      Author: sadko
 */

#ifndef DSP_ARCH_X86_AVX_FFT_BUTTERFLY_H_
#define DSP_ARCH_X86_AVX_FFT_BUTTERFLY_H_

#ifndef DSP_ARCH_X86_AVX_IMPL
    #error "This header should not be included directly"
#endif /* DSP_ARCH_X86_AVX_IMPL */

#define FFT_BUTTERFLY_BODY4(add_b, add_a) \
    __IF_32(float *ptr1, *ptr1);\
    size_t off1, off2; \
    \
    ARCH_X86_ASM \
    ( \
        /* Prepare angle */ \
        __ASM_EMIT("xor             %[off1], %[off1]") \
        __ASM_EMIT("mov             $0x10, %[off2]") \
        __ASM_EMIT("vmovaps         0x00 + %[FFT_A], %%ymm6")               /* ymm6 = x_re */ \
        __ASM_EMIT("vmovaps         0x20 + %[FFT_A], %%ymm7")               /* ymm7 = x_im */ \
        /* Start loop */ \
        __ASM_EMIT("sub             $2, %[nb]") \
        __ASM_EMIT("jb              2f") \
        __ASM_EMIT("1:") \
            __ASM_EMIT("vmovups         0x00(%[dst_re], %[off1]), %%xmm0")              /* xmm0 = a_re */ \
            __ASM_EMIT("vmovups         0x00(%[dst_re], %[off2]), %%xmm2")              /* xmm2 = b_re */ \
            __ASM_EMIT("vinsertf128     $1, 0x20(%[dst_re], %[off1]), %%ymm0, %%ymm0")  /* ymm0 = a_re1 a_re2 */ \
            __ASM_EMIT("vinsertf128     $1, 0x20(%[dst_re], %[off2]), %%ymm2, %%ymm2")  /* ymm2 = b_re1 b_re2 */ \
            __ASM_EMIT("vmovups         0x00(%[dst_im], %[off1]), %%xmm1")              /* xmm1 = a_im */ \
            __ASM_EMIT("vmovups         0x00(%[dst_im], %[off2]), %%xmm3")              /* xmm3 = b_im */ \
            __ASM_EMIT("vinsertf128     $1, 0x20(%[dst_im], %[off1]), %%ymm1, %%ymm1")  /* ymm0 = a_re1 a_re2 */ \
            __ASM_EMIT("vinsertf128     $1, 0x20(%[dst_im], %[off2]), %%ymm3, %%ymm3")  /* ymm2 = b_re1 b_re2 */ \
            /* Calculate complex multiplication */ \
            __ASM_EMIT("vmulps          %%ymm7, %%ymm2, %%ymm4")            /* ymm4 = x_im * b_re */ \
            __ASM_EMIT("vmulps          %%ymm7, %%ymm3, %%ymm5")            /* ymm5 = x_im * b_im */ \
            __ASM_EMIT("vmulps          %%ymm6, %%ymm2, %%ymm2")            /* ymm2 = x_re * b_re */ \
            __ASM_EMIT("vmulps          %%ymm6, %%ymm3, %%ymm3")            /* ymm3 = x_re * b_im */ \
            __ASM_EMIT(add_b "          %%ymm5, %%ymm2, %%ymm5")            /* ymm5 = c_re = x_re * b_re +- x_im * b_im */ \
            __ASM_EMIT(add_a "          %%ymm4, %%ymm3, %%ymm4")            /* ymm4 = c_im = x_re * b_im -+ x_im * b_re */ \
            /* Perform butterfly */ \
            __ASM_EMIT("vsubps          %%ymm5, %%ymm0, %%ymm2")            /* ymm2 = a_re - c_re */ \
            __ASM_EMIT("vsubps          %%ymm4, %%ymm1, %%ymm3")            /* ymm3 = a_im - c_im */ \
            __ASM_EMIT("vaddps          %%ymm5, %%ymm0, %%ymm0")            /* ymm0 = a_re + c_re */ \
            __ASM_EMIT("vaddps          %%ymm4, %%ymm1, %%ymm1")            /* ymm1 = a_im + c_im */ \
            /* Store values */ \
            __ASM_EMIT("vmovups         %%xmm0, 0x00(%[dst_re], %[off1])") \
            __ASM_EMIT("vmovups         %%xmm2, 0x00(%[dst_re], %[off2])") \
            __ASM_EMIT("vextractf128    $1, %%ymm0, 0x20(%[dst_re], %[off1])") \
            __ASM_EMIT("vextractf128    $1, %%ymm2, 0x20(%[dst_re], %[off2])") \
            __ASM_EMIT("vmovups         %%xmm1, 0x00(%[dst_im], %[off1])") \
            __ASM_EMIT("vmovups         %%xmm3, 0x00(%[dst_im], %[off2])") \
            __ASM_EMIT("vextractf128    $1, %%ymm1, 0x20(%[dst_im], %[off1])") \
            __ASM_EMIT("vextractf128    $1, %%ymm3, 0x20(%[dst_im], %[off2])") \
            __ASM_EMIT("add             $0x40, %[off1]") \
            __ASM_EMIT("add             $0x40, %[off2]") \
            __ASM_EMIT("sub             $2, %[nb]") \
            __ASM_EMIT("jae             1b") \
        __ASM_EMIT("2:") \
        __ASM_EMIT32("add           $1, %[nb]") \
        __ASM_EMIT32("jl            4f") \
            __ASM_EMIT("vmovups         0x00(%[dst_re], %[off1]), %%xmm0")  /* xmm0 = a_re */ \
            __ASM_EMIT("vmovups         0x00(%[dst_re], %[off2]), %%xmm2")  /* xmm2 = b_re */ \
            __ASM_EMIT("vmovups         0x00(%[dst_im], %[off1]), %%xmm1")  /* xmm1 = a_im */ \
            __ASM_EMIT("vmovups         0x00(%[dst_im], %[off2]), %%xmm3")  /* xmm3 = b_im */ \
            /* Calculate complex multiplication */ \
            __ASM_EMIT("vmulps          %%xmm7, %%xmm2, %%xmm4")            /* xmm4 = x_im * b_re */ \
            __ASM_EMIT("vmulps          %%xmm7, %%xmm3, %%xmm5")            /* xmm5 = x_im * b_im */ \
            __ASM_EMIT("vmulps          %%xmm6, %%xmm2, %%xmm2")            /* xmm2 = x_re * b_re */ \
            __ASM_EMIT("vmulps          %%xmm6, %%xmm3, %%xmm3")            /* xmm3 = x_re * b_im */ \
            __ASM_EMIT(add_b "          %%xmm5, %%xmm2, %%xmm5")            /* xmm5 = c_re = x_re * b_re +- x_im * b_im */ \
            __ASM_EMIT(add_a "          %%xmm4, %%xmm3, %%xmm4")            /* xmm4 = c_im = x_re * b_im -+ x_im * b_re */ \
            /* Perform butterfly */ \
            __ASM_EMIT("vsubps          %%xmm5, %%xmm0, %%xmm2")            /* xmm2 = a_re - c_re */ \
            __ASM_EMIT("vsubps          %%xmm4, %%xmm1, %%xmm3")            /* xmm3 = a_im - c_im */ \
            __ASM_EMIT("vaddps          %%xmm5, %%xmm0, %%xmm0")            /* xmm0 = a_re + c_re */ \
            __ASM_EMIT("vaddps          %%xmm4, %%xmm1, %%xmm1")            /* xmm1 = a_im + c_im */ \
            /* Store values */ \
            __ASM_EMIT("vmovups         %%xmm0, 0x00(%[dst_re], %[off1])") \
            __ASM_EMIT("vmovups         %%xmm2, 0x00(%[dst_re], %[off2])") \
            __ASM_EMIT("vmovups         %%xmm1, 0x00(%[dst_im], %[off1])") \
            __ASM_EMIT("vmovups         %%xmm3, 0x00(%[dst_im], %[off2])") \
        __ASM_EMIT("4:") \
        \
        : [off1] "=&r" (off1), [off2] "=&r" (off2), [nb] "+r" (blocks) \
        : [dst_re] "r" (dst_re), [dst_im] "r" (dst_im), \
          [FFT_A] "o" (FFT_A) \
        : "cc", "memory",  \
          "%xmm0", "%xmm1", "%xmm2", "%xmm3", \
          "%xmm4", "%xmm5", "%xmm6", "%xmm7" \
    );

#define FFT_BUTTERFLY_BODY8(add_b, add_a) \
    __IF_32(float *ptr1, *ptr1);\
    \
    ARCH_X86_ASM \
    ( \
        /* Prepare angle */ \
        __ASM_EMIT32("mov       %[fft_a], %[ptr2]") \
        __ASM_EMIT32("mov       %[dst_re], %[ptr1]") \
        __ASM_EMIT("vmovaps     0x00(%[" __IF_32_64("ptr2", "fft_a") "]), %%xmm6")        /* xmm6 = x_re */ \
        __ASM_EMIT("vmovaps     0x20(%[" __IF_32_64("ptr2", "fft_a") "]), %%xmm7")        /* xmm7 = x_im */ \
        __ASM_EMIT32("mov       %[dst_im], %[ptr2]") \
        /* Start loop */ \
        __ASM_EMIT("1:") \
            __ASM_EMIT("vmovups     0x00(%[" __IF_32_64("ptr1", "dst_re") "], %[off1]), %%ymm0")    /* ymm0 = a_re */ \
            __ASM_EMIT("vmovups     0x00(%[" __IF_32_64("ptr1", "dst_re") "], %[off2]), %%ymm2")    /* ymm2 = b_re */ \
            __ASM_EMIT("vmovups     0x00(%[" __IF_32_64("ptr2", "dst_im") "], %[off1]), %%ymm1")    /* ymm1 = a_im */ \
            __ASM_EMIT("vmovups     0x00(%[" __IF_32_64("ptr2", "dst_im") "], %[off2]), %%ymm3")    /* ymm3 = b_im */ \
            /* Calculate complex multiplication */ \
            __ASM_EMIT("vmulps          %%ymm7, %%ymm2, %%ymm4")            /* ymm4 = x_im * b_re */ \
            __ASM_EMIT("vmulps          %%ymm7, %%ymm3, %%ymm5")            /* ymm5 = x_im * b_im */ \
            __ASM_EMIT("vmulps          %%ymm6, %%ymm2, %%ymm2")            /* ymm2 = x_re * b_re */ \
            __ASM_EMIT("vmulps          %%ymm6, %%ymm3, %%ymm3")            /* ymm3 = x_re * b_im */ \
            __ASM_EMIT(add_b "          %%ymm5, %%ymm2, %%ymm5")            /* ymm5 = c_re = x_re * b_re +- x_im * b_im */ \
            __ASM_EMIT(add_a "          %%ymm4, %%ymm3, %%ymm4")            /* ymm4 = c_im = x_re * b_im -+ x_im * b_re */ \
            /* Perform butterfly */ \
            __ASM_EMIT("vsubps          %%ymm5, %%ymm0, %%ymm2")            /* ymm2 = a_re - c_re */ \
            __ASM_EMIT("vsubps          %%ymm4, %%ymm1, %%ymm3")            /* ymm3 = a_im - c_im */ \
            __ASM_EMIT("vaddps          %%ymm5, %%ymm0, %%ymm0")            /* ymm0 = a_re + c_re */ \
            __ASM_EMIT("vaddps          %%ymm4, %%ymm1, %%ymm1")            /* ymm1 = a_im + c_im */ \
            /* Store values */ \
            __ASM_EMIT("vmovups     %%ymm0, 0x00(%[" __IF_32_64("ptr1", "dst_re") "], %[off1])") \
            __ASM_EMIT("vmovups     %%ymm2, 0x00(%[" __IF_32_64("ptr1", "dst_re") "], %[off2])") \
            __ASM_EMIT("vmovups     %%ymm1, 0x00(%[" __IF_32_64("ptr2", "dst_im") "], %[off1])") \
            __ASM_EMIT("vmovups     %%ymm3, 0x00(%[" __IF_32_64("ptr2", "dst_im") "], %[off2])") \
            __ASM_EMIT("add         $0x20, %[off1]") \
            __ASM_EMIT("add         $0x20, %[off2]") \
            __ASM_EMIT32("subl      $8, %[np]") \
            __ASM_EMIT64("subq      $8, %[np]") \
            __ASM_EMIT("jz          2f") \
            /* Rotate angle */ \
            __ASM_EMIT32("mov       %[fft_w], %[ptr2]") \
            __ASM_EMIT("vmovaps     0x00(%[" __IF_32_64("ptr2", "fft_w") "]), %%ymm4")        /* xmm4 = w_re */ \
            __ASM_EMIT("vmovaps     0x20(%[" __IF_32_64("ptr2", "fft_w") "]), %%ymm5")        /* xmm5 = w_im */ \
            __ASM_EMIT32("mov       %[dst_im], %[ptr2]") \
            __ASM_EMIT("vmulps      %%ymm4, %%ymm6, %%ymm0")            /* ymm0 = w_re * x_re */ \
            __ASM_EMIT("vmulps      %%ymm4, %%ymm7, %%ymm1")            /* ymm1 = w_re * x_im */ \
            __ASM_EMIT("vmulps      %%ymm5, %%ymm6, %%ymm2")            /* ymm2 = w_im * x_re */ \
            __ASM_EMIT("vmulps      %%ymm5, %%ymm7, %%ymm3")            /* ymm3 = w_im * x_im */ \
            __ASM_EMIT("vaddps      %%ymm2, %%ymm1, %%ymm7")            /* ymm7 = x_im' = w_re * x_im + w_im * x_re */ \
            __ASM_EMIT("vsubps      %%ymm3, %%ymm0, %%ymm6")            /* ymm6 = x_re' = w_re * x_re - w_im * x_im */ \
            /* Repeat loop */ \
        __ASM_EMIT("jmp         1b") \
        __ASM_EMIT("2:") \
        \
        : __IF_32([ptr1] "=&r" (ptr1), [ptr2] "=&r" (ptr2), ) \
          [off1] "+r" (off1), [off2] "+r" (off2), [np] __ASM_ARG_RW(np) \
        : __IF_32([dst_re] "g" (dst_re), [dst_im] "g" (dst_im), [fft_a] "g" (fft_a), [fft_w] "g" (fft_w)) \
          __IF_64([dst_re] "r" (dst_re), [dst_im] "r" (dst_im), [fft_a] "r" (fft_a), [fft_w] "r" (fft_w)) \
        : "cc", "memory",  \
        "%xmm0", "%xmm1", "%xmm2", "%xmm3", \
        "%xmm4", "%xmm5" \
    );

namespace avx
{
    static inline void butterfly_direct4(float *dst_re, float *dst_im, size_t blocks)
    {
        FFT_BUTTERFLY_BODY4("vaddps", "vsubps");
    }

    static inline void butterfly_reverse4(float *dst_re, float *dst_im, size_t rank, size_t blocks)
    {
        FFT_BUTTERFLY_BODY4("vsubps", "vaddps");
    }

    static inline void butterfly_direct8p(float *dst_re, float *dst_im, size_t rank, size_t blocks)
    {
        size_t pairs = 1 << rank;
        size_t off1 = 0, shift = 1 << (rank + 2);
        const float *fft_a = &FFT_A[(rank - 2) << 5];
        const float *fft_w = &FFT_DW[(rank - 2) << 5];

        for (size_t b=0; b<blocks; ++b)
        {
            size_t off2  = off1 + shift;
            size_t np    = pairs;

            FFT_BUTTERFLY_BODY8("vaddps", "vsubps");

            off1        = off2;
        }
    }

    static inline void butterfly_reverse8p(float *dst_re, float *dst_im, size_t rank, size_t blocks)
    {
        size_t pairs = 1 << rank;
        size_t off1 = 0, shift = 1 << (rank + 2);
        const float *fft_a = &FFT_A[(rank - 2) << 5];
        const float *fft_w = &FFT_DW[(rank - 2) << 5];

        for (size_t b=0; b<blocks; ++b)
        {
            size_t off2  = off1 + shift;
            size_t np    = pairs;

            FFT_BUTTERFLY_BODY8("vsubps", "vaddps");

            off1        = off2;
        }
    }
}

#undef FFT_BUTTERFLY_BODY4
#undef FFT_BUTTERFLY_BODY8

#endif /* INCLUDE_DSP_ARCH_X86_AVX_FFT_BUTTERFLY_H_ */
