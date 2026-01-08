#include "matcher.h"

#include <array>
#include <limits>
#include <memory>

bool Matcher::matchWithOneInsertion(const char *insData, const char *normalData,
                                    int cmplen, int diffLimit) noexcept {
  if (!insData || !normalData)
    return false;
  if (cmplen < 2 || diffLimit < 0)
    return false;
  const int cap = (diffLimit == std::numeric_limits<int>::max())
                      ? diffLimit
                      : (diffLimit + 1);

  int prefix[cmplen];

  prefix[0] = (insData[0] != normalData[0]);
  for (int i = 1; i < cmplen; ++i) {
    prefix[i] = prefix[i - 1] + (insData[i] != normalData[i]);
  }
  int suffix = (insData[cmplen] != normalData[cmplen - 1]) ? 1 : 0;
  if (suffix > cap)
    suffix = cap;
  for (int i = cmplen - 1; i >= 1; --i) {
    const int diff = prefix[i - 1] + suffix;
    if (diff <= diffLimit)
      return true;

    suffix += (insData[i] != normalData[i - 1]) ? 1 : 0;
    if (suffix > cap)
      suffix = cap;

    if (prefix[0] + suffix > diffLimit)
      break;
  }
  return false;
}

int Matcher::diffWithOneInsertion(const char *insData, const char *normalData,
                                  int cmplen, int diffLimit) noexcept {
  if (!insData || !normalData)
    return -1;
  if (cmplen < 2 || diffLimit < 0)
    return -1;
  const int cap = (diffLimit == std::numeric_limits<int>::max())
                      ? diffLimit
                      : (diffLimit + 1);

  int prefix[cmplen];
  prefix[0] = (insData[0] != normalData[0]);
  for (int i = 1; i < cmplen; ++i) {
    prefix[i] = prefix[i - 1] + (insData[i] != normalData[i]);
  }

  int minDiff = cap;

  int suffix = (insData[cmplen] != normalData[cmplen - 1]) ? 1 : 0;
  if (suffix > cap)
    suffix = cap;

  for (int i = cmplen - 1; i >= 1; --i) {
    const int diff = prefix[i - 1] + suffix;
    if (diff < minDiff) {
      minDiff = diff;
      if (minDiff == 0)
        return 0; // can't do better
    }

    suffix += (insData[i] != normalData[i - 1]) ? 1 : 0;
    if (suffix > cap)
      suffix = cap;

    if (prefix[0] + suffix > diffLimit)
      break;
  }

  return (minDiff <= diffLimit) ? minDiff : -1;
}