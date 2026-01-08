#include "adaptertrimmer.h"
#include "matcher.h"

bool AdapterTrimmer::trimByOverlapAnalysis(Read *r1, Read *r2, FilterResult *fr,
                                           int diffLimit, int overlapRequire,
                                           double diffPercentLimit) {
  OverlapResult ov = OverlapAnalysis::analyze(r1, r2, diffLimit, overlapRequire,
                                              diffPercentLimit);
  return trimByOverlapAnalysis(r1, r2, fr, ov);
}

bool AdapterTrimmer::trimByOverlapAnalysis(Read *r1, Read *r2, FilterResult *fr,
                                           OverlapResult ov, int frontTrimmed1,
                                           int frontTrimmed2) {
  int ol = ov.overlap_len;
  if (ov.overlapped && ov.offset < 0) {

    // 5'
    // ......frontTrimmed1......|------------------------------------------|-----
    // 3' 3'
    // -----|-------------------------------------------|......frontTrimmed2.....
    // 5'

    int len1 = std::min(r1->length(), ol + frontTrimmed2);
    int len2 = std::min(r2->length(), ol + frontTrimmed1);
    std::string adapter1 = r1->mSeq.substr(len1, r1->length() - len1);
    std::string adapter2 = r2->mSeq.substr(len2, r2->length() - len2);

    if (DEBUG_MODE) {
      std::cerr << adapter1 << std::endl;
      std::cerr << adapter2 << std::endl;
      std::cerr << "frontTrimmed2: " << frontTrimmed1 << std::endl;
      std::cerr << "frontTrimmed2: " << frontTrimmed2 << std::endl;
      std::cerr << "overlap:" << ov.offset << "," << ov.overlap_len << ", "
                << ov.diff << std::endl;
      r1->print();
      r2->reverseComplement()->print();
      std::cerr << std::endl;
    }
    r1->resize(len1);
    r2->resize(len2);

    fr->addAdapterTrimmed(adapter1, adapter2);
    return true;
  }
  return false;
}

bool AdapterTrimmer::trimByMultiSequences(Read *r, FilterResult *fr,
                                          std::vector<std::string> &adapterList,
                                          bool isR2, bool incTrimmedCounter) {
  int matchReq = 4;
  if (adapterList.size() > 16)
    matchReq = 5;
  if (adapterList.size() > 256)
    matchReq = 6;
  bool trimmed = false;

  std::string originalSeq = r->mSeq;
  for (int i = 0; i < adapterList.size(); i++) {
    trimmed |= trimBySequence(r, NULL, adapterList[i], isR2, matchReq);
  }

  if (trimmed) {
    std::string adapter =
        originalSeq.substr(r->length(), originalSeq.length() - r->length());
    if (fr)
      fr->addAdapterTrimmed(adapter, isR2, incTrimmedCounter);
    else
      std::cerr << adapter << std::endl;
  }

  return trimmed;
}

bool AdapterTrimmer::trimBySequence(Read *r, FilterResult *fr,
                                    std::string &adapterseq, bool isR2,
                                    int matchReq) {
  const int allowOneMismatchForEach = 8;

  int rlen = r->length();
  int alen = adapterseq.length();

  const char *adata = adapterseq.c_str();
  const char *rdata = r->mSeq.c_str();

  if (alen < matchReq)
    return false;

  int pos = 0;
  bool found = false;
  int start = 0;
  if (alen >= 16)
    start = -4;
  else if (alen >= 12)
    start = -3;
  else if (alen >= 8)
    start = -2;
  // we start from negative numbers since the Illumina adapter dimer usually
  // have the first A skipped as A-tailing try exact match with hamming distance
  // (no insertion of deletion)
  for (pos = start; pos < rlen - matchReq; pos++) {
    int cmplen = std::min(rlen - pos, alen);
    int allowedMismatch = cmplen / allowOneMismatchForEach;
    int mismatch = 0;
    bool matched = true;
    for (int i = std::max(0, -pos); i < cmplen; i++) {
      if (adata[i] != rdata[i + pos]) {
        mismatch++;
        if (mismatch > allowedMismatch) {
          matched = false;
          break;
        }
      }
    }
    if (matched) {
      found = true;
      break;
    }
  }

  // if failed to exact match, we try one gap
  // to lower computational cost, we only allow one gap, and it's much enough
  // for short reads we try insertion in the sequence
  bool hasInsertion = false;
  if (!found) {
    for (pos = 0; pos < rlen - matchReq - 1; pos++) {
      int cmplen = std::min(rlen - pos - 1, alen);
      int allowedMismatch = cmplen / allowOneMismatchForEach - 1;
      bool matched =
          Matcher::matchWithOneInsertion(rdata, adata, cmplen, allowedMismatch);
      if (matched) {
        found = true;
        hasInsertion = true;
        // cerr << ".";
        break;
      }
    }
  }

  // if failed to exact match, and failed to match with one insertion in
  // sequence we then try deletion in the sequence
  bool hasDeletion = false;
  if (!found) {
    for (pos = 0; pos < rlen - matchReq; pos++) {
      int cmplen = std::min(rlen - pos, alen - 1);
      int allowedMismatch = cmplen / allowOneMismatchForEach - 1;
      bool matched =
          Matcher::matchWithOneInsertion(adata, rdata, cmplen, allowedMismatch);
      if (matched) {
        found = true;
        hasDeletion = true;
        // cerr << "|";
        break;
      }
    }
  }

  if (found) {
    if (pos < 0) {
      std::string adapter = adapterseq.substr(0, alen + pos);
      r->mSeq.resize(0);
      r->mQuality.resize(0);
      if (fr) {
        fr->addAdapterTrimmed(adapter, isR2);
      }

    } else {
      std::string adapter = r->mSeq.substr(pos, rlen - pos);
      r->resize(pos);
      if (fr) {
        fr->addAdapterTrimmed(adapter, isR2);
      }
    }
    return true;
  }

  return false;
}

bool AdapterTrimmer::test() {
  Read r("@name", "TTTTAACCCCCCCCCCCCCCCCCCCCCCCCCCCCAATTTTAAAATTTTCCCCGGGG",
         "+", "///EEEEEEEEEEEEEEEEEEEEEEEEEE////EEEEEEEEEEEEE////E////E");
  std::string adapter = "TTTTCCACGGGGATACTACTG";
  bool trimmed = AdapterTrimmer::trimBySequence(&r, NULL, adapter);
  if (r.mSeq != "TTTTAACCCCCCCCCCCCCCCCCCCCCCCCCCCCAATTTTAAAA")
    return false;

  Read read("@name",
            "TTTTAACCCCCCCCCCCCCCCCCCCCCCCCCCCCAATTTTAAAATTTTCCCCGGGGAAATTTCCCG"
            "GGAAATTTCCCGGGATCGATCGATCGATCGAATTCC",
            "+",
            "///EEEEEEEEEEEEEEEEEEEEEEEEEE////EEEEEEEEEEEEE////E////"
            "EEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEE");
  std::vector<std::string> adapterList;
  adapterList.push_back("GCTAGCTAGCTAGCTA");
  adapterList.push_back("AAATTTCCCGGGAAATTTCCCGGG");
  adapterList.push_back("ATCGATCGATCGATCG");
  adapterList.push_back("AATTCCGGAATTCCGG");
  trimmed = AdapterTrimmer::trimByMultiSequences(&read, NULL, adapterList);
  if (read.mSeq != "TTTTAACCCCCCCCCCCCCCCCCCCCCCCCCCCCAATTTTAAAATTTTCCCCGGGG") {
    std::cerr << read.mSeq << std::endl;
    return false;
  }

  return true;
}