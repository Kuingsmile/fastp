#ifndef MATCHER_H
#define MATCHER_H

class Matcher {
public:
  Matcher() = delete;

  static bool matchWithOneInsertion(const char *insData, const char *normalData,
                                    int cmplen, int diffLimit) noexcept;
  static int diffWithOneInsertion(const char *insData, const char *normalData,
                                  int cmplen, int diffLimit) noexcept;
};

#endif