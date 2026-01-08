#ifndef DUPLICATE_H
#define DUPLICATE_H

#include "common.h"
#include "options.h"
#include "read.h"
#include <atomic>
#include <string>

class Duplicate {
public:
  Duplicate(Options *opt);
  ~Duplicate() = default;

  bool checkRead(Read *r1);
  bool checkPair(Read *r1, Read *r2);
  void seq2intvector(const char *data, int len, uint64 *output,
                     int posOffset = 0);

  double getDupRate();

private:
  static uint32 nextPow2(uint32 x) noexcept;
  void initPrimeArrays();
  bool applyBloomFilter(uint64 *positions);

private:
  Options *mOptions{nullptr};

  uint64 mBufLenInBytes{0};
  uint64 mBufLenInBits{0};
  uint64 mBitMask{0};     // mBufLenInBits - 1 (valid because power-of-two)
  uint64 mWordsPerBuf{0}; // number of uint64 words per slice
  uint32 mBufNum{0};      // number of slices/hashes

  // Bloom storage: mBufNum slices, each slice is mWordsPerBuf 64-bit words
  std::unique_ptr<std::atomic<uint64>[]> mDupBufWords;

  // Prime table for hashing in seq2intvector
  uint32 mPrimeTableSize = 0; // power-of-two
  uint32 mPrimeTableMask = 0; // mPrimeTableSize - 1
  std::vector<uint64> mPrimeArrays;

  std::atomic_ulong mTotalReads{0};
  std::atomic_ulong mDupReads{0};
};

#endif