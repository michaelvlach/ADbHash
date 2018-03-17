#pragma once

#include <cstdint>
#include <emmintrin.h>

namespace adb
{
inline int match(char byte, const char *data)
{
    const __m128i m = _mm_set1_epi8(byte);
    const __m128i ctrl = _mm_load_si128(reinterpret_cast<const __m128i *>(data));
    return _mm_movemask_epi8(_mm_cmpeq_epi8(m, ctrl));
}

inline int64_t hashIndex(uint64_t hash, int64_t size)
{
    return static_cast<int64_t>(hash % static_cast<uint64_t>(size));
}

inline char hashMetaValue(uint64_t hash)
{
    return hash & static_cast<char>(0b01111111);
}
}
