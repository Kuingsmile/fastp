#include "duplicate.h"
#include "util.h"

#include <cmath>
#include <cstring>

namespace {
constexpr uint32 PRIME_ARRAY_LEN = 1u << 9; // 512

inline uint64 base_code(unsigned char c) noexcept {
  switch (c) {
  case 'A':
  case 'a':
    return 7;
  case 'T':
  case 't':
    return 222;
  case 'C':
  case 'c':
    return 74;
  case 'G':
  case 'g':
    return 31;
  default:
    return 13;
  }
}
} // namespace

uint32 Duplicate::nextPow2(uint32 x) noexcept {
  if (x <= 1)
    return 1;
  --x;
  x |= x >> 1;
  x |= x >> 2;
  x |= x >> 4;
  x |= x >> 8;
  x |= x >> 16;
  return x + 1;
}

Duplicate::Duplicate(Options *opt) : mOptions(opt) {
  // Base settings (matching original intent)
  mBufLenInBytes = 1ull << 29; // 512MiB per slice
  mBufNum = 2;

  switch (mOptions->duplicate.accuracyLevel) {
  case 1:
    break;
  case 2:
    mBufLenInBytes *= 2;
    break;
  case 3:
    mBufLenInBytes *= 2;
    mBufNum *= 2;
    break;
  case 4:
    mBufLenInBytes *= 4;
    mBufNum *= 2;
    break;
  case 5:
    mBufLenInBytes *= 8;
    mBufNum *= 2;
    break;
  case 6:
    mBufLenInBytes *= 8;
    mBufNum *= 3;
    break;
  default:
    break;
  }

  mBufLenInBits = mBufLenInBytes << 3; // bytes * 8
  mBitMask = mBufLenInBits - 1;

  // 64-bit word addressing
  mWordsPerBuf = (mBufLenInBits + 63) >> 6;

  const size_t totalWords =
      static_cast<size_t>(mWordsPerBuf) * static_cast<size_t>(mBufNum);
  try {
    mDupBufWords = std::make_unique<std::atomic<uint64_t>[]>(totalWords);
  } catch (...) {
    error_exit("Out of memory, failed to allocate bloom buffer for duplication "
               "analysis, "
               "please reduce dup_accuracy_level and try again.");
  }
  for (size_t i = 0; i < totalWords; ++i)
    mDupBufWords[i].store(0, std::memory_order_relaxed);

  const uint32 needed = static_cast<uint32>(mBufNum) * PRIME_ARRAY_LEN;
  mPrimeTableSize = nextPow2(needed);
  mPrimeTableMask = mPrimeTableSize - 1;

  mPrimeArrays.assign(mPrimeTableSize, 0);
  initPrimeArrays();

  mTotalReads.store(0, std::memory_order_relaxed);
  mDupReads.store(0, std::memory_order_relaxed);
}

void Duplicate::initPrimeArrays() {
  uint64 number = 10000;
  uint32 count = 0;

  while (count < mPrimeTableSize) {
    number++;
    bool isPrime = true;
    for (uint64 i = 2; i * i <= number; ++i) {
      if (number % i == 0) {
        isPrime = false;
        break;
      }
    }

    if (isPrime) {
      mPrimeArrays[count] = number;
      ++count;

      number += 10000;
    }
  }
}

void Duplicate::seq2intvector(const char *data, int len, uint64 *output,
                              int posOffset) {
  const uint32 bufNum = mBufNum;
  const uint32 mask = mPrimeTableMask;

  for (int p = 0; p < len; ++p) {
    const uint64 pp = static_cast<uint64>(p + posOffset);
    const uint64 base = base_code(static_cast<unsigned char>(data[p]));
    const uint64 add = base + pp;

    uint32 off = static_cast<uint32>(pp * bufNum);

    for (uint32 i = 0; i < bufNum; ++i) {
      const uint32 idx = (off + i) & mask;
      output[i] += mPrimeArrays[idx] * add;
    }
  }
}

bool Duplicate::applyBloomFilter(uint64 *positions) {
  bool isDup = true;

  for (uint32 i = 0; i < mBufNum; ++i) {
    const uint64 bitPos = positions[i] & mBitMask;

    const uint64 wordPos = bitPos >> 6; // 64
    const uint32 bitOff = static_cast<uint32>(bitPos & 63);
    const uint64 bitMask = 1ull << bitOff;

    const size_t idx =
        static_cast<size_t>(i) * static_cast<size_t>(mWordsPerBuf) +
        static_cast<size_t>(wordPos);

    // fetch_or returns previous value; if bit was already set, it's "present"
    // for this hash
    const uint64 prev =
        mDupBufWords[idx].fetch_or(bitMask, std::memory_order_relaxed);

    isDup &= ((prev & bitMask) != 0);
  }

  return isDup;
}

bool Duplicate::checkRead(Read *r) {
  thread_local std::vector<uint64> positions;
  positions.assign(mBufNum, 0);

  const int len = r->length();
  seq2intvector(r->mSeq.data(), len, positions.data());

  const bool isDup = applyBloomFilter(positions.data());

  mTotalReads.fetch_add(1, std::memory_order_relaxed);
  if (isDup)
    mDupReads.fetch_add(1, std::memory_order_relaxed);

  return isDup;
}

bool Duplicate::checkPair(Read *r1, Read *r2) {
  thread_local std::vector<uint64> positions;
  positions.assign(mBufNum, 0);

  const int len1 = r1->length();
  seq2intvector(r1->mSeq.data(), len1, positions.data());
  seq2intvector(r2->mSeq.data(), r2->length(), positions.data(), len1);

  const bool isDup = applyBloomFilter(positions.data());

  mTotalReads.fetch_add(1, std::memory_order_relaxed);
  if (isDup)
    mDupReads.fetch_add(1, std::memory_order_relaxed);

  return isDup;
}

double Duplicate::getDupRate() {
  const unsigned long total = mTotalReads.load(std::memory_order_relaxed);
  if (total == 0)
    return 0.0;

  const unsigned long dup = mDupReads.load(std::memory_order_relaxed);
  return static_cast<double>(dup) / static_cast<double>(total);
}
