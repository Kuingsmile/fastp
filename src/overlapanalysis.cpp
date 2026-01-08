#include "overlapanalysis.h"
#include "matcher.h"
#include "sequence.h"

OverlapResult OverlapAnalysis::analyze(const Read *r1, const Read *r2,
                                       int overlapDiffLimit, int overlapRequire,
                                       double diffPercentLimit, bool allowGap) {
  return analyze(r1->mSeq, r2->mSeq, overlapDiffLimit, overlapRequire,
                 diffPercentLimit, allowGap);
}

// ported from the python code of AfterQC
OverlapResult OverlapAnalysis::analyze(std::string_view r1, std::string_view r2,
                                       int diffLimit, int overlapRequire,
                                       double diffPercentLimit, bool allowGap) {
  std::string rcr2 = Sequence::reverseComplement(r2);
  int len1 = r1.length();
  int len2 = rcr2.length();
  // use the pointer directly for speed
  const char *str1 = r1.data();
  const char *str2 = rcr2.data();

  int complete_compare_require = 50;

  int overlap_len = 0;
  int offset = 0;
  int diff = 0;

  // forward with no gap
  // a match of less than overlapRequire is considered as unconfident
  while (offset < len1 - overlapRequire) {
    // the overlap length of r1 & r2 when r2 is move right for offset
    overlap_len = std::min(len1 - offset, len2);
    int overlapDiffLimit =
        std::min(diffLimit, (int)(overlap_len * diffPercentLimit));

    diff = 0;
    int i = 0;
    for (i = 0; i < overlap_len; i++) {
      if (str1[offset + i] != str2[i]) {
        diff += 1;
        if (diff > overlapDiffLimit && i < complete_compare_require)
          break;
      }
    }

    if (diff <= overlapDiffLimit ||
        (diff > overlapDiffLimit && i > complete_compare_require)) {
      OverlapResult ov;
      ov.overlapped = true;
      ov.offset = offset;
      ov.overlap_len = overlap_len;
      ov.diff = diff;
      ov.hasGap = false;
      return ov;
    }

    offset += 1;
  }

  // reverse with no gap
  // in this case, the adapter is sequenced since TEMPLATE_LEN < SEQ_LEN
  // check if distance can get smaller if offset goes negative
  // this only happens when insert DNA is shorter than sequencing read length,
  // and some adapter/primer is sequenced but not trimmed cleanly we go
  // reversely
  offset = 0;
  while (offset > -(len2 - overlapRequire)) {
    // the overlap length of r1 & r2 when r2 is move right for offset
    overlap_len = std::min(len1, len2 - abs(offset));
    int overlapDiffLimit =
        std::min(diffLimit, (int)(overlap_len * diffPercentLimit));

    diff = 0;
    int i = 0;
    for (i = 0; i < overlap_len; i++) {
      if (str1[i] != str2[-offset + i]) {
        diff += 1;
        if (diff > overlapDiffLimit && i < complete_compare_require)
          break;
      }
    }

    if (diff <= overlapDiffLimit ||
        (diff > overlapDiffLimit && i > complete_compare_require)) {
      OverlapResult ov;
      ov.overlapped = true;
      ov.offset = offset;
      ov.overlap_len = overlap_len;
      ov.diff = diff;
      ov.hasGap = false;
      return ov;
    }

    offset -= 1;
  }

  if (allowGap) {
    // forward with one gap
    offset = 0;
    while (offset < len1 - overlapRequire) {
      // the overlap length of r1 & r2 when r2 is move right for offset
      overlap_len = std::min(len1 - offset, len2);
      int overlapDiffLimit =
          std::min(diffLimit, (int)(overlap_len * diffPercentLimit));

      int diff = Matcher::diffWithOneInsertion(
          str1 + offset, str2, overlap_len - 1, overlapDiffLimit);
      if (diff < 0 || diff > overlapDiffLimit)
        diff = Matcher::diffWithOneInsertion(str2, str1 + offset,
                                             overlap_len - 1, overlapDiffLimit);

      if (diff <= overlapDiffLimit && diff >= 0) {
        OverlapResult ov;
        ov.overlapped = true;
        ov.offset = offset;
        ov.overlap_len = overlap_len;
        ov.diff = diff;
        ov.hasGap = true;
        return ov;
      }

      offset += 1;
    }

    // reverse with one gap
    offset = 0;
    while (offset > -(len2 - overlapRequire)) {
      // the overlap length of r1 & r2 when r2 is move right for offset
      overlap_len = std::min(len1, len2 - abs(offset));
      int overlapDiffLimit =
          std::min(diffLimit, (int)(overlap_len * diffPercentLimit));

      int diff = Matcher::diffWithOneInsertion(
          str1, str2 - offset, overlap_len - 1, overlapDiffLimit);
      if (diff < 0 || diff > overlapDiffLimit)
        diff = Matcher::diffWithOneInsertion(str2 - offset, str1,
                                             overlap_len - 1, overlapDiffLimit);

      if (diff <= overlapDiffLimit && diff >= 0) {
        OverlapResult ov;
        ov.overlapped = true;
        ov.offset = offset;
        ov.overlap_len = overlap_len;
        ov.diff = diff;
        ov.hasGap = true;
        return ov;
      }

      offset -= 1;
    }
  }

  OverlapResult ov;
  ov.overlapped = false;
  ov.offset = ov.overlap_len = ov.diff = 0;
  ov.hasGap = false;
  return ov;
}

std::unique_ptr<Read> OverlapAnalysis::merge(const Read *r1, const Read *r2,
                                             OverlapResult ov) {
  int ol = ov.overlap_len;
  if (!ov.overlapped)
    return nullptr;

  int len1 = ol + std::max(0, ov.offset);
  int len2 = 0;
  if (ov.offset > 0)
    len2 = r2->length() - ol;

  std::unique_ptr<Read> rr2 = r2->reverseComplement();
  std::string mergedSeq = r1->mSeq.substr(0, len1);
  if (ov.offset > 0) {
    mergedSeq += rr2->mSeq.substr(ol, len2);
  }

  std::string mergedQual = r1->mQuality.substr(0, len1);
  if (ov.offset > 0) {
    mergedQual += rr2->mQuality.substr(ol, len2);
  }

  std::string name = r1->mName + " merged_" + std::to_string(len1) + "_" +
                     std::to_string(len2);
  std::string strand = r1->mStrand;
  if (strand != "+") {
    strand =
        strand + " merged_" + std::to_string(len1) + "_" + std::to_string(len2);
  }
  std::unique_ptr<Read> mergedRead =
      std::make_unique<Read>(name, mergedSeq, strand, mergedQual);

  return mergedRead;
}

bool OverlapAnalysis::test() {
  // Sequence
  // r1("CAGCGCCTACGGGCCCCTTTTTCTGCGCGACCGCGTGGCTGTGGGCGCGGATGCCTTTGAGCGCGGTGACTTCTCACTGCGTATCGAGCCGCTGGAGGTCTCCC");
  // Sequence
  // r2("ACCTCCAGCGGCTCGATACGCAGTGAGAAGTCACCGCGCTCAAAGGCATCCGCGCCCACAGCCACGCGGTCGCGCAGAAAAAGGGGCCCGTAGGCGCGGCTCCC");

  std::string r1 = "CAGCGCCTACGGGCCCCTTTTTCTGCGCGACCGCGTGGCTGTGGGCGCGGAT"
                   "GCCTTTGAGCGCGGTGACTTCTCACTGCGTATCGAGC";
  std::string r2 = "ACCTCCAGCGGCTCGATACGCAGTGAGAAGTCACCGCGCTCAAAGGCATCCG"
                   "CGCCCACAGCCACGCGGTCGCGCAGAAAAAGGGGTCC";
  std::string qual1 = "FFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFF"
                      "FFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFF";
  std::string qual2 = "#################################################"
                      "########################################";

  OverlapResult ov = OverlapAnalysis::analyze(r1, r2, 2, 30, 0.2);

  Read read1("name1", r1, "+", qual1);
  Read read2("name2", r2, "+", qual2);

  std::unique_ptr<Read> mergedRead = OverlapAnalysis::merge(&read1, &read2, ov);
  mergedRead->print();

  return ov.overlapped && ov.offset == 10 && ov.overlap_len == 79 &&
         ov.diff == 1;
}
