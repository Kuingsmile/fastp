#ifndef FILTER_RESULT_H
#define FILTER_RESULT_H

#include "common.h"
#include "options.h"
#include <fstream>
#include <iostream>
#include <map>
#include <stdio.h>
#include <stdlib.h>
#include <string>
#include <vector>

struct classcomp {
  bool operator()(const std::string &lhs, const std::string &rhs) const {
    if (lhs.length() < rhs.length())
      return true;
    else if (lhs.length() == rhs.length()) {
      return lhs < rhs;
    } else
      return false;
  }
};
class FilterResult {
public:
  FilterResult(Options *opt, bool paired = false);
  ~FilterResult();
  inline long *getFilterReadStats() { return mFilterReadStats; }
  void addFilterResult(int result, int readNum = 1);
  static FilterResult *merge(std::vector<FilterResult *> &list);
  void print();
  // for single end
  void addAdapterTrimmed(std::string adapter, bool isR2 = false,
                         bool incTrimmedCounter = true);
  // for paired end
  void addAdapterTrimmed(std::string adapter1, std::string adapter2);
  void addPolyXTrimmed(int base, int length);
  long getTotalPolyXTrimmedReads();
  long getTotalPolyXTrimmedBases();
  // a part of JSON report
  void reportJson(std::ofstream &ofs, std::string padding);
  // a part of JSON report for adapters
  void reportAdapterJson(std::ofstream &ofs, std::string padding);
  // a part of JSON report for polyX trim
  void reportPolyXTrimJson(std::ofstream &ofs, std::string padding);
  // a part of HTML report
  void reportHtml(std::ofstream &ofs, long totalReads, long totalBases);
  // a part of HTML report for adapters
  void reportAdapterHtml(std::ofstream &ofs, long totalBases);
  void
  outputAdaptersJson(std::ofstream &ofs,
                     std::map<std::string, long, classcomp> &adapterCounts);
  int outputAdaptersHtml(std::ofstream &ofs,
                         std::map<std::string, long, classcomp> &adapterCounts,
                         long totalBases, int limitCount = 0);
  int getAdapterReportCount(
      std::map<std::string, long, classcomp> &adapterCounts);
  // deal with base correction results
  long *getCorrectionMatrix() { return mCorrectionMatrix; }
  long getTotalCorrectedBases();
  void addCorrection(char from, char to);
  long getCorrectionNum(char from, char to);
  void incCorrectedReads(int count);
  void addMergedPairs(int pairs);
  bool isLowComplexity(std::string &adapter);

public:
  Options *mOptions;
  bool mPaired;
  long mCorrectedReads;
  long mMergedPairs;

private:
  long mFilterReadStats[FILTER_RESULT_TYPES];
  long mTrimmedAdapterRead;
  long mTrimmedAdapterBases;
  long mTrimmedPolyXReads[4] = {0};
  long mTrimmedPolyXBases[4] = {0};
  std::map<std::string, long, classcomp> mAdapter1;
  std::map<std::string, long, classcomp> mAdapter2;
  long *mCorrectionMatrix;
};

#endif