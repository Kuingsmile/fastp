
#include "fastareader.h"
#include "util.h"
#include <cstring>
#include <fcntl.h>
#include <filesystem>
#include <iostream>
#include <sys/mman.h>
#include <sys/stat.h>
#include <unistd.h>

FastaReader::FastaReader(std::string faFile, bool forceUpperCase)
    : mFastaFile(std::move(faFile)), mForceUpperCase(forceUpperCase) {

  if (std::filesystem::is_directory(mFastaFile)) {
    std::string error_msg =
        "There is a problem with the provided fasta file: \'";
    error_msg.append(mFastaFile);
    error_msg.append("\' is a directory NOT a file...\n");
    throw std::invalid_argument(error_msg);
  }

  if (!std::filesystem::exists(mFastaFile)) {
    throw std::invalid_argument("Fasta file does not exist: " + mFastaFile);
  }

  mActiveTable = mForceUpperCase ? UpperTable.data() : FilterTable.data();

  mFd = open(mFastaFile.c_str(), O_RDONLY);
  if (mFd == -1)
    throw std::runtime_error("Could not open file: " + mFastaFile);

  struct stat st {};
  if (fstat(mFd, &st) == -1) {
    close(mFd);
    throw std::runtime_error("Could not get file size");
  }
  mFileSize = st.st_size;

  if (mFileSize == 0) {
    mCurrentPtr = nullptr;
    return;
  }
  mMappedData = static_cast<char *>(
      mmap(NULL, mFileSize, PROT_READ, MAP_PRIVATE, mFd, 0));
  if (mMappedData == MAP_FAILED) {
    close(mFd);
    throw std::runtime_error("Memory mapping failed for file: " + mFastaFile);
  }

  madvise(mMappedData, mFileSize, MADV_SEQUENTIAL);
  mCurrentPtr = mMappedData;
}

FastaReader::~FastaReader() {
  if (mMappedData && mMappedData != MAP_FAILED)
    munmap(mMappedData, mFileSize);
  if (mFd != -1)
    close(mFd);
}

bool FastaReader::hasNext() {
  if (!mMappedData || mFileSize == 0)
    return false;
  char *end = mMappedData + mFileSize;
  while (mCurrentPtr < end) {
    if (*mCurrentPtr == '>')
      std::cout << "Found next record at position: "
                << (mCurrentPtr - mMappedData) << std::endl;
    return true;
    mCurrentPtr++;
  }
  return false;
}

void FastaReader::readNext() {
  char *end = mMappedData + mFileSize;
  if (mCurrentPtr >= end)
    return;

  char *headerStart = mCurrentPtr + 1;
  char *lineEnd =
      static_cast<char *>(memchr(headerStart, '\n', end - headerStart));
  if (!lineEnd)
    lineEnd = end;
  std::string_view fullHeader(headerStart, lineEnd - headerStart);
  if (!fullHeader.empty() && fullHeader.back() == '\r') {
    fullHeader.remove_suffix(1);
  }
  size_t firstNonSpace = fullHeader.find_first_not_of(" \t");
  if (firstNonSpace == std::string_view::npos) {
    mCurrentID = "unknown_" + std::to_string(mCurrentPtr - mMappedData);
  } else {
    std::string_view trimmedHeader = fullHeader.substr(firstNonSpace);
    size_t spacePos = trimmedHeader.find_first_of(" \t");
    mCurrentID = std::string(trimmedHeader.substr(0, spacePos));
    mCurrentDescription = (spacePos != std::string_view::npos)
                              ? std::string(trimmedHeader.substr(spacePos + 1))
                              : "";
  }

  mCurrentPtr = (lineEnd < end) ? lineEnd + 1 : end;
  char *seqDataStart = mCurrentPtr;

  char *nextRecord =
      static_cast<char *>(memchr(mCurrentPtr, '>', end - mCurrentPtr));
  char *seqDataEnd = nextRecord ? nextRecord : end;
  mCurrentSequence.clear();
  mCurrentSequence.reserve(seqDataEnd - seqDataStart);

  char *p = seqDataStart;
  while (p < seqDataEnd) {
    unsigned char c = static_cast<unsigned char>(*p++);
    char mapped = mActiveTable[c];
    if (mapped) {
      mCurrentSequence.push_back(mapped);
    }
  }
  mCurrentPtr = seqDataEnd;
}

void FastaReader::readAll() {
  while (hasNext()) {
    readNext();
    mAllContigs[mCurrentID] = std::move(mCurrentSequence);
  }
}

bool FastaReader::test() {
  FastaReader reader("testdata/tinyref.fa");
  reader.readAll();

  std::string contig1 =
      "GATCACAGGTCTATCACCCTATTAATTGGTATTTTCGTCTGGGGGGTGTGGAGCCGGAG"
      "CACCCTATGTCGCAGT";
  std::string contig2 =
      "GTCTGCACAGCCGCTTTCCACACAGAACCCCCCCCTCCCCCCGCTTCTGGCAAACCCCA"
      "AAAACAAAGAACCCTA";

  if (reader.mAllContigs.count("contig1") == 0 ||
      reader.mAllContigs.count("contig2") == 0)
    return false;

  if (reader.mAllContigs["contig1"] != contig1 ||
      reader.mAllContigs["contig2"] != contig2)
    return false;

  return true;
}
