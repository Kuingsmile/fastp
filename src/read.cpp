#include "read.h"
#include "sequence.h"
#include <algorithm>
#include <charconv>
#include <cstring>
#include <sstream>

namespace {
inline void append_int(std::string &s, int v) {
  char buf[32];
  auto [ptr, ec] = std::to_chars(buf, buf + sizeof(buf), v);
  (void)ec;
  s.append(buf, static_cast<size_t>(ptr - buf));
}
} // namespace

Read::Read(std::string name, std::string seq, std::string strand,
           std::string quality, bool phred64)
    : mName(std::move(name)), mSeq(std::move(seq)), mStrand(std::move(strand)),
      mQuality(std::move(quality)) {
  if (phred64)
    convertPhred64To33();
}

Read::Read(const char *name, const char *seq, const char *strand,
           const char *quality, bool phred64)
    : mName(name), mSeq(seq), mStrand(strand), mQuality(quality) {
  if (phred64)
    convertPhred64To33();
}

void Read::convertPhred64To33() {
  for (char &q : mQuality) {
    int v = static_cast<unsigned char>(q);
    v = v - (64 - 33);
    if (v < 33)
      v = 33;
    q = static_cast<char>(v);
  }
}

void Read::print() const {
  std::cerr << mName << '\n';
  std::cerr << mSeq << '\n';
  std::cerr << mStrand << '\n';
  std::cerr << mQuality << '\n';
}

void Read::printFile(std::ofstream &file) const {
  file.write(mName.data(), static_cast<std::streamsize>(mName.size()));
  file.put('\n');
  file.write(mSeq.data(), static_cast<std::streamsize>(mSeq.size()));
  file.put('\n');
  file.write(mStrand.data(), static_cast<std::streamsize>(mStrand.size()));
  file.put('\n');
  file.write(mQuality.data(), static_cast<std::streamsize>(mQuality.size()));
  file.put('\n');
}

std::unique_ptr<Read> Read::reverseComplement() const {
  std::string seq = Sequence::reverseComplement(mSeq);
  std::string qual(mQuality.rbegin(), mQuality.rend());
  return std::make_unique<Read>(mName, std::move(seq), "+", std::move(qual));
}

void Read::resize(int len) {
  if (len >= 0 && len <= static_cast<int>(mSeq.length())) {
    mSeq.resize(len);
    mQuality.resize(len);
  }
}

void Read::trimFront(int len) {
  len = std::min(static_cast<int>(mSeq.length()), len);
  mSeq.erase(0, len);
  mQuality.erase(0, len);
}

std::string_view Read::lastIndex() const {
  size_t len = mName.length();
  if (len < 5)
    return "";
  for (size_t i = len - 3; i > 0; i--) {
    if (mName[i] == ':' || mName[i] == '+') {
      return std::string_view(mName).substr(i + 1);
    }
  }
  return "";
}

std::string_view Read::firstIndex() const {
  size_t len = mName.length();
  if (len < 5)
    return "";
  size_t end = len;
  for (size_t i = len - 3; i > 0; i--) {
    if (mName[i] == '+')
      end = i;
    if (mName[i] == ':') {
      return std::string_view(mName).substr(i + 1, end - (i + 1));
    }
  }
  return "";
}

int Read::lowQualCount(int qual) const {
  int count = 0;
  const int threshold = qual + 33;
  const char *q = mQuality.data();
  const size_t n = mQuality.size();
  for (size_t i = 0; i < n; ++i) {
    count += (static_cast<unsigned char>(q[i]) <
              static_cast<unsigned char>(threshold));
  }
  return count;
}

std::string Read::toString() const {
  std::string out;
  out.reserve(mName.size() + mSeq.size() + mStrand.size() + mQuality.size() +
              4);
  out.append(mName).push_back('\n');
  out.append(mSeq).push_back('\n');
  out.append(mStrand).push_back('\n');
  out.append(mQuality).push_back('\n');
  return out;
}

void Read::appendToString(std::string &target) const {
  const size_t total =
      mName.size() + mSeq.size() + mStrand.size() + mQuality.size() + 4;
  target.reserve(target.size() + total);

  target.append(mName).push_back('\n');
  target.append(mSeq).push_back('\n');
  target.append(mStrand).push_back('\n');
  target.append(mQuality).push_back('\n');
}

void Read::appendToStringWithTag(std::string &target,
                                 std::string_view tag) const {
  const size_t total = mName.size() + 1 + tag.size() + 1 + mSeq.size() + 1 +
                       mStrand.size() + 1 + mQuality.size() + 1;
  target.reserve(target.size() + total);

  target.append(mName);
  target.push_back(' ');
  target.append(tag.data(), tag.size());
  target.push_back('\n');
  target.append(mSeq).push_back('\n');
  target.append(mStrand).push_back('\n');
  target.append(mQuality).push_back('\n');
}

std::string Read::toStringWithTag(std::string_view tag) const {
  std::string out;
  out.reserve(mName.size() + 1 + tag.size() + 1 + mSeq.size() + 1 +
              mStrand.size() + 1 + mQuality.size() + 1);

  out.append(mName);
  out.push_back(' ');
  out.append(tag.data(), tag.size());
  out.push_back('\n');
  out.append(mSeq).push_back('\n');
  out.append(mStrand).push_back('\n');
  out.append(mQuality).push_back('\n');
  return out;
}

bool Read::fixMGI() {
  if (mName.size() < 3)
    return false;
  const size_t len = mName.size();
  if ((mName[len - 1] == '1' || mName[len - 1] == '2') &&
      mName[len - 2] == '/') {
    std::string suffix = mName.substr(len - 2);
    mName.resize(len - 2);
    mName.reserve(mName.size() + 1 + suffix.size());
    mName.push_back(' ');
    mName.append(suffix);
    return true;
  }
  return false;
}

bool Read::test() {
  Read r("@NS500713:64:HFKJJBGXY:1:11101:20469:1097 1:N:0:TATAGCCT+GGTCCCGA",
         "CTCTTGGACTCTAACACTGTTTTTTCTTATGAAAACACAGGAGTGATGACTAGTTGAGTGC"
         "ATTCTTATGAGACTCATAGTCATTCTATGATGTAGTTTTCCTTAGGAGGACATTTTTTACA"
         "TGAAATTATTAACCTAAATAGAGTTGATC",
         "+",
         "AAAAA6EEEEEEEEEEEEEEEEE#EEEEEEEEEEEEEEEEE/"
         "EEEEEEEEEEEEEEEEAEEEAEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEE<"
         "EEEEAEEEEEEEEEEEEEEEAEEE/EEEEEEEEEEAAEAEAAEEEAEEAA");
  std::string_view idx = r.lastIndex();
  return idx == "GGTCCCGA";
}

ReadPair::ReadPair(std::unique_ptr<Read> left,
                   std::unique_ptr<Read> right) noexcept
    : mLeft(std::move(left)), mRight(std::move(right)) {}

std::unique_ptr<Read> ReadPair::fastMerge() {
  const int lLen = mLeft->length();
  const int rLen = mRight->length();
  const char *lSeq = mLeft->mSeq.data();
  const char *lQual = mLeft->mQuality.data();

  std::string rcSeq = Sequence::reverseComplement(mRight->mSeq);
  std::string rcQual;
  rcQual.resize(mRight->mQuality.size());
  std::reverse_copy(mRight->mQuality.begin(), mRight->mQuality.end(),
                    rcQual.begin());

  // we require at least 30 bp overlapping to merge a pair
  const int MIN_OVERLAP = 30;
  bool overlapped = false;
  // overlap length
  int olen = MIN_OVERLAP;
  // difference count
  int diff = 0;
  // the diff count for 1 high qual + 1 low qual
  int lowQualDiff = 0;
  const int max_olen = std::min(lLen, static_cast<int>(rcSeq.size()));

  while (olen <= max_olen) {
    diff = 0;
    lowQualDiff = 0;
    bool ok = true;
    int offset = lLen - olen;

    for (int i = 0; i < olen; i++) {
      if (lSeq[offset + i] != rcSeq[i]) {
        ++diff;
        const char q1 = lQual[offset + i];
        const char q2 = rcQual[i];
        // one is >= Q30 and the other is <= Q15
        if ((q1 >= '?' && q2 <= '0') || (q1 <= '0' && q2 >= '?')) {
          lowQualDiff++;
        }
        // we disallow high quality diff, and only allow up to 3 low qual diff
        if (diff > lowQualDiff || lowQualDiff >= 3) {
          ok = false;
          break;
        }
      }
    }
    if (ok) {
      overlapped = true;
      break;
    }
    olen++;
  }

  if (!overlapped)
    return nullptr;

  const int offset = lLen - olen;
  std::string mergedName;
  mergedName.reserve(mLeft->mName.size() + 64);
  mergedName.append(mLeft->mName);
  mergedName.append(" merged offset:");
  append_int(mergedName, offset);
  mergedName.append(" overlap:");
  append_int(mergedName, olen);
  mergedName.append(" diff:");
  append_int(mergedName, diff);
  std::string mergedSeq;
  std::string mergedQual;
  mergedSeq.resize(static_cast<size_t>(offset + rLen));
  mergedQual.resize(static_cast<size_t>(offset + rLen));
  if (offset > 0) {
    std::memcpy(mergedSeq.data(), lSeq, static_cast<size_t>(offset));
    std::memcpy(mergedQual.data(), lQual, static_cast<size_t>(offset));
  }
  std::memcpy(mergedSeq.data() + offset, rcSeq.data(),
              static_cast<size_t>(rLen));
  std::memcpy(mergedQual.data() + offset, rcQual.data(),
              static_cast<size_t>(rLen));
  // quality adjuction and correction for low qual diff
  for (int i = 0; i < olen; i++) {
    int idx = offset + i;
    if (lSeq[idx] != rcSeq[i]) {
      if (lQual[idx] >= '?' && rcQual[i] <= '0') {
        mergedSeq[idx] = lSeq[idx];
        mergedQual[idx] = lQual[idx];
      } else {
        mergedSeq[idx] = rcSeq[i];
        mergedQual[idx] = rcQual[i];
      }
    } else {
      // add the quality of the pair to make a high qual
      mergedQual[idx] = lQual[idx] + rcQual[i] - 33;
    }
  }
  return std::make_unique<Read>(std::move(mergedName), std::move(mergedSeq),
                                "+", std::move(mergedQual));
}

bool ReadPair::test() {
  auto left = std::make_unique<Read>(
      "@NS500713:64:HFKJJBGXY:1:11101:20469:1097 1:N:0:TATAGCCT+GGTCCCGA",
      "TTTTTTCTCTTGGACTCTAACACTGTTTTTTCTTATGAAAACACAGGAGTGATGACTAGTT"
      "GAGTGCATTCTTATGAGACTCATAGTCATTCTATGATGTAG",
      "+",
      "AAAAA6EEEEEEEEEEEEEEEEE#"
      "EEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEAEEEAEEEEEEEEEEEEEEEEEEEEEE"
      "EEEEEEEEEEEEEEEEE");
  auto right = std::make_unique<Read>(
      "@NS500713:64:HFKJJBGXY:1:11101:20469:1097 1:N:0:TATAGCCT+GGTCCCGA",
      "AAAAAACTACACCATAGAATGACTATGAGTCTCATAAGAATGCACTCAACTAGTCATCACT"
      "CCTGTGTTTTCATAAGAAAAAACAGTGTTAGAGTCCAAGAG",
      "+",
      "AAAAA6EEEEE/"
      "EEEEEEEEEEE#"
      "EEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEAEEEAEEEEEEEEEEEEEEEEEEEEEE"
      "EEEEEEEEEEEEEEEEE");

  ReadPair pair(std::move(left), std::move(right));
  std::unique_ptr<Read> merged = pair.fastMerge();

  if (merged == nullptr)
    return false;

  if (merged->mSeq !=
      "TTTTTTCTCTTGGACTCTAACACTGTTTTTTCTTATGAAAACACAGGAGTGATGACTAGTTGAGTGCATTCT"
      "TATGAGACTCATAGTCATTCTATGATGTAGTTTTTT")
    return false;

  return true;
}
