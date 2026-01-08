#ifndef READ_H
#define READ_H

#include <fstream>
#include <iostream>
#include <memory>
#include <string>
#include <vector>

class Read {
public:
  Read(std::string name, std::string seq, std::string strand,
       std::string quality, bool phred64 = false);
  Read(const char *name, const char *seq, const char *strand,
       const char *quality, bool phred64 = false);

  Read(const Read &) = delete;
  Read &operator=(const Read &) = delete;
  Read(Read &&) noexcept = default;
  Read &operator=(Read &&) noexcept = default;

  ~Read() = default;

  // I/O functions
  void print() const;
  void printFile(std::ofstream &file) const;

  // manipulations
  [[nodiscard]] std::unique_ptr<Read> reverseComplement() const;

  [[nodiscard]] std::string_view firstIndex() const;
  [[nodiscard]] std::string_view lastIndex() const;

  // default is Q20
  [[nodiscard]] int lowQualCount(int qual = 20) const;

  [[nodiscard]] int length() const noexcept {
    return static_cast<int>(mSeq.size());
  }

  [[nodiscard]] std::string toString() const;
  [[nodiscard]] std::string toStringWithTag(std::string_view tag) const;

  void resize(int len);
  void appendToString(std::string &target) const;
  void appendToStringWithTag(std::string &target, std::string_view tag) const;

  void reserve(int len) {
    if (len > 0) {
      mSeq.reserve(static_cast<size_t>(len));
      mQuality.reserve(static_cast<size_t>(len));
    }
  }

  void convertPhred64To33();
  void trimFront(int len);
  bool fixMGI();

public:
  static bool test();

private:
public:
  std::string mName;
  std::string mSeq;
  std::string mStrand;
  std::string mQuality;
};

class ReadPair {
public:
  ReadPair(std::unique_ptr<Read> left, std::unique_ptr<Read> right) noexcept;
  ~ReadPair() = default;

  // merge a pair, without consideration of seq error caused false INDEL
  std::unique_ptr<Read> fastMerge();

public:
  std::unique_ptr<Read> mLeft;
  std::unique_ptr<Read> mRight;

public:
  static bool test();
};

struct ReadPack {
  std::vector<std::unique_ptr<Read>> data;
};
#endif