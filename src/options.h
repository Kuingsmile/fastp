#pragma once

#include <cstdio>
#include <cstdlib>
#include <map>
#include <string>
#include <vector>

enum class UMILocation : std::uint8_t {
  None = 0,
  Index1 = 1,
  Index2 = 2,
  Read1 = 3,
  Read2 = 4,
  PerIndex = 5,
  PerRead = 6
};

class MergeOptions {
public:
  bool enabled{false};
  bool includeUnmerged{false};
  std::string out{};
};

class DuplicationOptions {
public:
  bool enabled{true};
  int histSize{32};
  bool dedup{false};
  int accuracyLevel{1};
};

class IndexFilterOptions {
public:
  std::vector<std::string> blacklist1{};
  std::vector<std::string> blacklist2{};
  bool enabled{false};
  int threshold{0};
};

class LowComplexityFilterOptions {
public:
  bool enabled{false};
  double threshold{0.3};
};

class OverrepresentedSequenceAnasysOptions {
public:
  bool enabled{false};
  int sampling{20};
};

class PolyGTrimmerOptions {
public:
  bool enabled{false};
  int minLen{10};
};

class PolyXTrimmerOptions {
public:
  bool enabled{false};
  int minLen{10};
};

class UMIOptions {
public:
  bool enabled{false};
  int location{static_cast<int>(UMILocation::None)};
  int length{0};
  int skip{0};
  std::string prefix{};
  std::string separator{};
  std::string delimiter{":"};
};

class CorrectionOptions {
public:
  bool enabled{false};
};

class QualityCutOptions {
public:
  // enable 5' cutting by quality
  bool enabledFront{false};
  // enable 3' cutting by quality
  bool enabledTail{false};
  // enable agressive cutting mode
  bool enabledRight{false};
  // the sliding window size
  int windowSizeShared{4};
  // the mean quality requirement
  int qualityShared{20};
  // the sliding window size for cutting by quality in 5'
  int windowSizeFront{4};
  // the mean quality requirement for cutting by quality in 5'
  int qualityFront{20};
  // the sliding window size for cutting by quality in 3'
  int windowSizeTail{4};
  // the mean quality requirement for cutting by quality in 3'
  int qualityTail{20};
  // the sliding window size for cutting by quality in aggressive mode
  int windowSizeRight{4};
  // the mean quality requirement for cutting by quality in aggressive mode
  int qualityRight{20};
};

class SplitOptions {
public:
  bool enabled{false};
  // number of files
  int number{0};
  // lines of each file
  long size{0};
  // digits number of file name prefix, for example 0001 means 4 digits
  int digits{4};
  // need evaluation?
  bool needEvaluation{false};
  bool byFileNumber{false};
  bool byFileLines{false};
};

class AdapterOptions {
public:
  bool enabled{true};
  std::string sequence{};
  std::string sequenceR2{};
  std::string detectedAdapter1{};
  std::string detectedAdapter2{};
  std::vector<std::string> seqsInFasta{};
  std::string fastaFile{};
  bool hasSeqR1{false};
  bool hasSeqR2{false};
  bool hasFasta{false};
  bool detectAdapterForPE{false};
  bool allowGapOverlapTrimming{false};
};

class TrimmingOptions {
public:
  // trimming first cycles for read1
  int front1{0};
  // trimming last cycles for read1
  int tail1{0};
  // trimming first cycles for read2
  int front2{0};
  // trimming last cycles for read2
  int tail2{0};
  // max length of read1
  int maxLen1{0};
  // max length of read2
  int maxLen2{0};
};

class QualityFilteringOptions {
public:
  // quality filter enabled
  bool enabled{true};
  // if a base's quality phred score < qualifiedPhred, then it's considered as a
  // low_qual_base ('0' = Q15)
  char qualifiedQual{'0'};
  // if low_qual_base_num > lowQualLimit, then discard this read
  int unqualifiedPercentLimit{40};
  // if n_base_number > nBaseLimit, then discard this read
  int nBaseLimit{5};
  // if average qual score < avgQualReq, then discard this read
  int avgQualReq{0};
};

class ReadLengthFilteringOptions {
public:
  // length filter enabled
  bool enabled{false};
  // if read_length < requiredLength, then this read is discard
  int requiredLength{15};
  // length limit, 0 for no limitation
  int maxLength{0};
};

class Options {
public:
  Options();
  void init();
  [[nodiscard]] bool isPaired() const;
  [[nodiscard]] bool validate();
  [[nodiscard]] bool adapterCuttingEnabled() const;
  [[nodiscard]] bool polyXTrimmingEnabled() const;
  [[nodiscard]] std::string getAdapter1() const;
  [[nodiscard]] std::string getAdapter2() const;
  void initIndexFiltering(const std::string &blacklistFile1,
                          const std::string &blacklistFile2, int threshold = 0);
  [[nodiscard]] std::vector<std::string>
  makeListFromFileByLine(const std::string &filename);
  [[nodiscard]] bool shallDetectAdapter(bool isR2 = false) const;
  void loadFastaAdapters();

public:
  // file name of read1 input
  std::string in1{};
  // file name of read2 input
  std::string in2{};
  // file name of read1 output
  std::string out1{};
  // file name of read2 output
  std::string out2{};
  // file name of unpaired read1 output
  std::string unpaired1{};
  // file name of unpaired read2 output
  std::string unpaired2{};
  // file name of failed reads output
  std::string failedOut{};
  // json file
  std::string overlappedOut{};
  // json file
  std::string jsonFile{};
  // html file
  std::string htmlFile{};
  // html report title
  std::string reportTitle{};
  // compression level
  int compression{};
  // the input file is using phred64 quality scoring
  bool phred64{};
  // do not rewrite existing files
  bool dontOverwrite{};
  // read STDIN
  bool inputFromSTDIN{};
  // write STDOUT
  bool outputToSTDOUT{};
  // the input R1 file is interleaved
  bool interleavedInput{};
  // only process first N reads
  int readsToProcess{};
  // fix the MGI ID tailing issue
  bool fixMGI{};
  // worker thread number
  int thread{};
  // trimming options
  TrimmingOptions trim{};
  // quality filtering options
  QualityFilteringOptions qualfilter{};
  // length filtering options
  ReadLengthFilteringOptions lengthFilter{};
  // adapter options
  AdapterOptions adapter{};
  // multiple file splitting options
  SplitOptions split{};
  // options for quality cutting
  QualityCutOptions qualityCut{};
  // options for base correction
  CorrectionOptions correction{};
  // options for UMI
  UMIOptions umi{};
  // 3' end polyG trimming, default for Illumina NextSeq/NovaSeq
  PolyGTrimmerOptions polyGTrim{};
  // 3' end polyX trimming
  PolyXTrimmerOptions polyXTrim{};
  // for overrepresentation analysis
  OverrepresentedSequenceAnasysOptions overRepAnalysis{};
  std::map<std::string, long> overRepSeqs1{};
  std::map<std::string, long> overRepSeqs2{};
  int seqLen1{};
  int seqLen2{};
  // low complexity filtering
  LowComplexityFilterOptions complexityFilter{};
  // black lists for filtering by index
  IndexFilterOptions indexFilter{};
  // options for duplication profiling
  DuplicationOptions duplicate{};
  // max value of insert size
  int insertSizeMax{};
  // overlap analysis threshold
  int overlapRequire{};
  int overlapDiffLimit{};
  int overlapDiffPercentLimit{};
  // output debug information
  bool verbose{};
  // merge options
  MergeOptions merge{};
  // the buffer size for writer
  size_t writerBufferSize{}; // Default 1MB
};
