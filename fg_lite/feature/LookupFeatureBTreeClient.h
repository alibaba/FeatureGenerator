#pragma once

#include "autil/ConstString.h"
#include "autil/Log.h"
#include <immintrin.h>
#include "fg_lite/feature/LookupFeatureBTreeEncoder.h"
#include "fg_lite/feature/LookupFeatureEncoder.h"

namespace fg_lite {

enum BlockBitsOption {
    BLOCKBITS_UNKNOWN = 0,
    BLOCKBITS_256 = 256,
    BLOCKBITS_512 = 512,
};

inline __attribute__((__always_inline__)) __m256i _mm256_set1_epi64 (long long __A) {
    return _mm256_set1_epi64x (__A);
}

#define AVX_LOWER_BOUND_FN(vector_bits, key_bits)                       \
    inline __attribute__((__always_inline__)) int lower_bound_##vector_bits##_##key_bits(const void *block, uint##key_bits##_t key) { \
        auto block_ = _mm ##vector_bits ## _loadu_si ##vector_bits ((__m##vector_bits##i_u*)block); \
        auto key_ =   _mm ##vector_bits ## _set1_epi ##key_bits (key);  \
        auto result_ = _mm##vector_bits ## _cmpgt_epu ## key_bits ##_mask(key_, block_); \
        return __builtin_popcount(result_);                             \
    }

AVX_LOWER_BOUND_FN(256, 16)
AVX_LOWER_BOUND_FN(256, 32)
AVX_LOWER_BOUND_FN(256, 64)
AVX_LOWER_BOUND_FN(512, 16)
AVX_LOWER_BOUND_FN(512, 32)
AVX_LOWER_BOUND_FN(512, 64)

template <typename KeyType, int blockBitsOpt>
int lowerBoundSearchOptionally(const KeyType* block, KeyType key, const BlockSizeType& blockSize);

template<typename KeyType, int blockBitsOpt>
inline __attribute__((__always_inline__)) const char* doWhileCurID(
        const KeyType* keyBase, const char * valueBase,
        KeyType key, int &curID, const int& blockNum,
        const BlockSizeType& blockSize,
        const size_t& valueSize,
        const size_t& dimension) {
    while (curID < blockNum) {
        const KeyType* block = keyBase + (curID - 1) * blockSize;
        int idx = lowerBoundSearchOptionally<KeyType, blockBitsOpt>(block, key, blockSize);
        if ((idx < blockSize) && (key == *(block + idx)))
        {
            return valueBase + ((curID - 1) * blockSize + idx) * valueSize * dimension;
        }
        curID = curID * (blockSize + 1) + 1 - (blockSize - idx);
    }
    return nullptr;
}

bool read_cpuid(unsigned int flag);

extern const bool supportAVX512F;
extern const bool supportAVX512VL; // for special Intrinsics
extern const bool supportAVX512BW;


template<typename KeyType>
inline __attribute__((__always_inline__)) const char * doWhileCurIDWrap(
        const int& blockBits,
        const KeyType* keyBase, const char * valueBase,
        KeyType key, int &curID, const int& blockNum,
        const BlockSizeType& blockSize,
        const size_t& valueSize,
        const size_t& dimension) {
    // CPUID FLAGS:
    // 16    BITS:             AVX512BW
    // 32/64 BITS:             AVX512F
    // _mm256_cmpgt_epu*_mask: AVX512VL
    const bool invalidBytesAndFlags = (sizeof(KeyType) <= 2 && !supportAVX512BW)
                                    ||(sizeof(KeyType) >= 4 && !supportAVX512F);
    if (!invalidBytesAndFlags && blockBits == 512) {
        return doWhileCurID<KeyType, 512>(keyBase, valueBase, key, curID, blockNum, blockSize, valueSize, dimension);
    } else if (!invalidBytesAndFlags && supportAVX512VL && blockBits == 256 ) {
        return doWhileCurID<KeyType, 256>(keyBase, valueBase, key, curID, blockNum, blockSize, valueSize, dimension);
    }
    return doWhileCurID<KeyType, 0>(keyBase, valueBase, key, curID, blockNum, blockSize, valueSize, dimension);
}

template<typename KeyType>
const void* branchFind(const autil::ConstString buffer, KeyType key, size_t valueSize, const size_t dimension = 1) {
    // get basic buffer info
    auto  headInfoRet = *((headInfoStruct*)buffer.data());
    auto  headInfoLength = sizeof(headInfoRet);
    auto  keyNum = headInfoRet.keyNum;
    auto  blockSize = headInfoRet.blockSize;
    auto  blockNum = (keyNum + blockSize - 1) / blockSize;

    int blockBits = blockSize * sizeof(KeyType) * 8;
    KeyType *keyBase = (KeyType *)(buffer.data() + headInfoLength);
    const char *valueBase = buffer.data() + headInfoLength + keyNum * sizeof(KeyType);

    int curID = 1;
    const char * ret = doWhileCurIDWrap<KeyType>(
            blockBits, keyBase, valueBase, key,
            curID, blockNum, blockSize, valueSize, dimension);
    // process the last block
    if (curID == blockNum) {
        for (int i = 0; i < blockSize; ++i) {
            if ((curID - 1) * blockSize + i >= keyNum) {
                return nullptr;
            }
            if (key == *(keyBase + (curID - 1) * blockSize + i)) {
                return valueBase + ((curID - 1) * blockSize + i) * valueSize * dimension;
            }
        }
    }
    return ret;
}

} // namespace fg_lite
