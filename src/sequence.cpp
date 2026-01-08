#include "sequence.h"
#include <array>
#include <iostream>
#include <string_view>

Sequence::Sequence(std::string seq) : mStr(std::move(seq)) {}

void Sequence::print() const { std::cerr << mStr; }

alignas(64) static constexpr auto complementTable = []() {
  std::array<char, 256> t = {};
  for (std::size_t i = 0; i < 256; ++i) {
    t[i] = 'N';
  }

  t['A'] = 'T';
  t['a'] = 'T';
  t['T'] = 'A';
  t['t'] = 'A';
  t['C'] = 'G';
  t['c'] = 'G';
  t['G'] = 'C';
  t['g'] = 'C';
  t['N'] = 'N';
  t['n'] = 'N';

  return t;
}();

std::string Sequence::reverseComplement(std::string_view origin) {
  std::size_t len = origin.size();
  if (len == 0)
    return "";

  std::string result;
  result.resize(len);
  const char *src = origin.data();
  char *dst = &result[len - 1];
  for (std::size_t i = 0; i < len; ++i) {
    *dst-- = complementTable[static_cast<unsigned char>(src[i])];
  }
  return result;
}

Sequence Sequence::reverseComplement() const {
  return Sequence(reverseComplement(mStr));
}

bool Sequence::test() {
  std::string test_seq = "AAAATTTTCCCCGGGG";
  std::string expected = "CCCCGGGGAAAATTTT";
  Sequence s(test_seq);
  Sequence rc = ~s; // 调用 operator~，内部指向 reverseComplement()

  if (s.mStr != test_seq) {
    std::cerr << "Error: Original sequence modified!" << std::endl;
    return false;
  }

  if (rc.mStr != expected) {
    std::cerr << "Failed in reverseComplement(). "
              << "Expected: " << expected << " Got: " << rc.mStr << std::endl;
    return false;
  }

  std::cout << "All tests passed!" << std::endl;
  return true;
}