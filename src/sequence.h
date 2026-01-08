#ifndef SEQUENCE_H
#define SEQUENCE_H

#include <string>

class Sequence {
public:
  Sequence() = default;
  explicit Sequence(std::string seq);
  ~Sequence() = default;

  void print() const;
  std::size_t length() const noexcept { return mStr.length(); }

  Sequence reverseComplement() const;
  Sequence operator~() const { return reverseComplement(); }

  static bool test();
  static std::string reverseComplement(std::string_view origin);

public:
  std::string mStr;
};

#endif