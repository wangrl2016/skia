//
// Created by wangrl on 2020/12/9.
//

#pragma once

#include <algorithm>
#include <stdint.h>
#include <utility>

namespace WTF {

/**
 * The keyword __alignof__ allows you to inquire about how an object is aligned, or the minimum alignment
 * usually required by a type. Its syntax is just like sizeof.
 *
 * For example, if the target machine requires a double value to be aligned on an 8-byte boundary, then
 * __alignof__(double) is 8. This is true on many RISC machines.
 *
 * https://gcc.gnu.org/onlinedocs/gcc-6.3.0/gcc/Alignment.html
 *
 * https://gcc.gnu.org/onlinedocs/gcc/Common-Variable-Attributes.html#Common-Variable-Attributes
 *
 * The aligned attribute specifies a minimum alignment for the variable or structure field, measured in bytes.
 * When specified, alignment must be an integer constant power of.
 * Specifying no alignment argument implies the maximum alignment for the target, which is often, but by no means
 * always, 8 or 16 bytes.
 *
 * For example, the declaration:
 *
 * int x __attribute__((aligned(16)) = 0;
 *
 * causes the compiler to allocate the global variable x on a 16-byte boundary.
 */

#if COMPILER(GCC)
#define WTF_ALIGN_OF(type) __alignof__(type)
#define WTF_ALIGNED(variable_type, variable, n) variable_type variable __attribute__((__aligned__(n)))
#elif COMPILER(MSVC)
#define WTF_ALIGN_OF(type) __alignof(type)
#define WTF_ALIGNED(variable_type, variable, n) __declspec(align(n)) variable_type variable
#else
#error WTF_ALIGN macros need alignment control.
#endif


/**
 * Accesses to objects with types with this attribute are not subjected to type-based alias analysis, but are
 * instead assumed to be able to alias any other type of objects, just like the char type.
 *
 * Example of use:
 *
 * typedef short __attribute__((__may_alias__)) short_a;
 *
 * int main(void) {
 *     int a = 0x12345678;
 *     short_a * b = (short_a *) &a;
 *     b[1] = 0;
 *
 *     if (a == 0x12345678)
 *         abort();
 *
 *     exit(0);
 * }
 *
 * If you replaced short_a with short in the variable declaration, the above program would abort compiled
 * with -fstrict-aliasing, which is on by default at -O2 or above in recent GCC versions.
 */

#if COMPILER(GCC)
    typedef char __attribute__((__may_alias__)) AlignedBufferChar;
#else
    typedef char AlignedBufferChar;
#endif


    template<size_t size, size_t alignment>
    struct AlignedBuffer;

    template<size_t size>
    struct AlignedBuffer<size, 1> {
        AlignedBufferChar buffer[size];
    };

    template<size_t size>
    struct AlignedBuffer<size, 2> {
        WTF_ALIGNED(AlignedBufferChar, buffer[size], 2);
    };

    template<size_t size>
    struct AlignedBuffer<size, 4> {
        WTF_ALIGNED(AlignedBufferChar, buffer[size], 4);
    };

    template<size_t size>
    struct AlignedBuffer<size, 8> {
        WTF_ALIGNED(AlignedBufferChar, buffer[size], 8);
    };

    template<size_t size>
    struct AlignedBuffer<size, 16> {
        WTF_ALIGNED(AlignedBufferChar, buffer[size], 16);
    };

    template<size_t size>
    struct AlignedBuffer<size, 32> {
        WTF_ALIGNED(AlignedBufferChar, buffer[size], 32);
    };

    template<size_t size>
    struct AlignedBuffer<size, 64> {
        WTF_ALIGNED(AlignedBufferChar, buffer[size], 64);
    };

    template<size_t size, size_t alignment>
    void swap(AlignedBuffer<size, alignment>& a, AlignedBuffer<size, alignment>& b) {
        for (size_t i = 0; i < size; ++i) {
            std::swap(a.buffer[i], b.buffer[i]);
        }
    }


    /**
     * #define is_aligned(POINTER, BYTE_COUNT) \
     * (((uintptr_t)(const void *)(POINTER)) % (BYTE_COUNT) == 0)
     */

    template<uintptr_t mask>
    inline bool isAlignedTo(const void* pointer) {
        return !(reinterpret_cast<uintptr_t>(pointer) & mask);
    }
}
