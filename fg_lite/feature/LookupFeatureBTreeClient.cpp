#include "fg_lite/feature/LookupFeatureBTreeClient.h"

using namespace std;

namespace fg_lite {

bool read_cpuid(unsigned int flag) {
    unsigned int eax = 0;
    unsigned int ebx = 0;
    unsigned int ecx = 0;
    unsigned int edx = 0;
    if (__get_cpuid(1, &eax, &ebx, &ecx, &edx)) {
        return ((eax & flag) == flag)
            || ((ebx & flag) == flag)
            || ((ecx & flag) == flag)
            || ((edx & flag) == flag);
    }
    return false;
}

const bool supportAVX512F = read_cpuid(bit_AVX512F);
const bool supportAVX512VL = read_cpuid(bit_AVX512VL);
const bool supportAVX512BW = read_cpuid(bit_AVX512BW);

#define SEARCH_OPTIONALLY_FN(keyTypeParam, Bits)             \
    template<>                                                          \
    int lowerBoundSearchOptionally<uint##keyTypeParam##_t, BLOCKBITS_##Bits> \
    (const uint##keyTypeParam##_t* block, uint##keyTypeParam##_t key, const BlockSizeType& blockSize) { \
        return lower_bound_##Bits##_##keyTypeParam (block, key); \
    }

SEARCH_OPTIONALLY_FN(16, 256)
SEARCH_OPTIONALLY_FN(16, 512)
SEARCH_OPTIONALLY_FN(32, 256)
SEARCH_OPTIONALLY_FN(32, 512)
SEARCH_OPTIONALLY_FN(64, 256)
SEARCH_OPTIONALLY_FN(64, 512)

#define SEARCH_WITHOUT_AVX_FN(keyTypeParam)                                         \
    template<>                                                          \
    int lowerBoundSearchOptionally<uint##keyTypeParam##_t, BLOCKBITS_UNKNOWN> \
    (const uint##keyTypeParam##_t* block, uint##keyTypeParam##_t key, const BlockSizeType& blockSize) { \
        int idx;                                                        \
        for (idx = 0; idx < blockSize; ++idx) {                         \
            if (*(block + idx) >= key) break;                           \
        }                                                               \
        return idx;                                                     \
    }

SEARCH_WITHOUT_AVX_FN(16)
SEARCH_WITHOUT_AVX_FN(32)
SEARCH_WITHOUT_AVX_FN(64)

}
