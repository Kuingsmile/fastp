#ifndef COMMON_H
#define COMMON_H

#include <array>
#include <cstdint>
#include <string_view>

inline constexpr std::string_view FASTP_VER = "1.0.1";

inline constexpr bool DEBUG_MODE = false;

using int64 = std::int64_t;
using uint64 = std::uint64_t;
using int32 = std::int32_t;
using uint32 = std::uint32_t;
using int16 = std::int16_t;
using uint16 = std::uint16_t;
using int8 = std::int8_t;
using uint8 = std::uint8_t;

inline constexpr std::array<char, 4> ATCG_BASES = {'A', 'T', 'C', 'G'};

// how many reads one pack has
inline constexpr int PACK_SIZE = 256;
// if one pack is produced, but not consumed, it will be kept in the memory
// this number limit the number of in memory packs
// if the number of in memory packs is full, the producer thread should sleep
inline constexpr int PACK_IN_MEM_LIMIT = 128;

// different filtering results, bigger number means worse
// if r1 and r2 are both failed, then the bigger one of the two results will be
// recorded we reserve some gaps for future types to be added
inline constexpr int PASS_FILTER = 0;
inline constexpr int FAIL_POLY_X = 4;
inline constexpr int FAIL_OVERLAP = 8;
inline constexpr int FAIL_N_BASE = 12;
inline constexpr int FAIL_LENGTH = 16;
inline constexpr int FAIL_TOO_LONG = 17;
inline constexpr int FAIL_QUALITY = 20;
inline constexpr int FAIL_COMPLEXITY = 24;
// how many types in total we support
inline constexpr int FILTER_RESULT_TYPES = 32;

inline constexpr std::array<std::string_view, FILTER_RESULT_TYPES>
    FAILED_TYPES = [] {
      std::array<std::string_view, FILTER_RESULT_TYPES> arr{};
      arr[PASS_FILTER] = "passed";
      arr[FAIL_POLY_X] = "failed_polyx_filter";
      arr[FAIL_OVERLAP] = "failed_bad_overlap";
      arr[FAIL_N_BASE] = "failed_too_many_n_bases";
      arr[FAIL_LENGTH] = "failed_too_short";
      arr[FAIL_TOO_LONG] = "failed_too_long";
      arr[FAIL_QUALITY] = "failed_quality_filter";
      arr[FAIL_COMPLEXITY] = "failed_low_complexity";
      return arr;
    }();
#endif /* COMMON_H */
