#ifndef FASTA_READER_H
#define FASTA_READER_H

// includes
#include <cctype>
#include <fstream>
#include <iostream>
#include <map>
#include <stdexcept>
#include <string>
#include <string_view>

class FastaReader {
public:
  FastaReader(std::string fastaFile, bool forceUpperCase = true);
  ~FastaReader();
  [[nodiscard]] bool hasNext();
  void readNext();
  void readAll();

  [[nodiscard]] inline const std::string &currentID() const noexcept {
    return mCurrentID;
  }

  [[nodiscard]] inline const std::string &currentDescription() const noexcept {
    return mCurrentDescription;
  }

  [[nodiscard]] inline const std::string &currentSequence() const noexcept {
    return mCurrentSequence;
  }

  [[nodiscard]] inline std::map<std::string, std::string> &contigs() noexcept {
    return mAllContigs;
  }

  static bool test();

public:
  std::string mCurrentSequence{};
  std::string mCurrentID{};
  std::string mCurrentDescription{};
  std::map<std::string, std::string> mAllContigs{};

private:
  bool readLine();
  bool endOfLine(char c);
  void setFastaSequenceIdDescription();

private:
  std::string mFastaFile{};
  std::ifstream mFastaFileStream{};
  bool mForceUpperCase{};

  int mFd{-1};
  char *mMappedData{nullptr};
  size_t mFileSize{0};
  char *mCurrentPtr{nullptr};

  const char *mActiveTable;
};

#endif
