#ifndef OVERLAP_ANALYSIS_H
#define OVERLAP_ANALYSIS_H

#include "read.h"
#include <iostream>
#include <stdio.h>
#include <stdlib.h>
#include <string>
#include <vector>

using namespace std;

class OverlapResult {
public:
  bool overlapped;
  int offset;
  int overlap_len;
  int diff;
  bool hasGap;
};

class OverlapAnalysis {
public:
  OverlapAnalysis() = default;
  ~OverlapAnalysis() = default;

  static OverlapResult analyze(std::string_view r1, std::string_view r2,
                               int diffLimit, int overlapRequire,
                               double diffPercentLimit, bool allowGap = false);
  static OverlapResult analyze(const Read *r1, const Read *r2, int diffLimit,
                               int overlapRequire, double diffPercentLimit,
                               bool allowGap = false);
  static std::unique_ptr<Read> merge(const Read *r1, const Read *r2,
                                     OverlapResult ov);

public:
  static bool test();
};

#endif