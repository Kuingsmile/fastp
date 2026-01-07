#include <chrono>
#include <filesystem>
#include <iomanip>
#include <iostream>
#include <vector>

// 假设你已经把旧代码存为 FastaReaderOld.h，新代码存为 FastaReader.h
#include "fastareader.h"
#include "fastareaderOld.h"

namespace fs = std::filesystem;

/**
 * 性能测试工具类
 */
class Benchmark {
public:
  static void run(const std::string &filename) {
    if (!fs::exists(filename)) {
      std::cerr << "File not found: " << filename << std::endl;
      return;
    }

    size_t fileSize = fs::file_size(filename);
    double fileSizeMB = fileSize / (1024.0 * 1024.0);

    std::cout << "--- Testing File: " << filename << " (" << std::fixed
              << std::setprecision(2) << fileSizeMB << " MB) ---" << std::endl;

    // 1. 测试旧版逻辑 (iostream)
    {
      auto start = std::chrono::high_resolution_clock::now();

      FastaReaderOld reader(filename);
      reader.readAll();

      auto end = std::chrono::high_resolution_clock::now();
      std::chrono::duration<double> diff = end - start;
      printResult("Old (iostream)", diff.count(), fileSizeMB,
                  reader.mAllContigs.size());
    }

    // 2. 测试新版逻辑 (mmap)
    {
      auto start = std::chrono::high_resolution_clock::now();

      FastaReader reader(filename);
      reader.readAll();

      auto end = std::chrono::high_resolution_clock::now();
      std::chrono::duration<double> diff = end - start;
      printResult("New (mmap)", diff.count(), fileSizeMB,
                  reader.mAllContigs.size());
    }
    std::cout << std::endl;
  }

private:
  static void printResult(const std::string &label, double seconds,
                          double sizeMB, size_t count) {
    double speed = sizeMB / seconds;
    std::cout << std::left << std::setw(20) << label << " Time: " << std::fixed
              << std::setprecision(4) << seconds << "s | "
              << " Speed: " << std::setw(8) << speed << " MB/s | "
              << " Contigs: " << count << std::endl;
  }
};

int main() {
  // 自动遍历 testdata 目录下的所有 .fa 或 .fasta 文件
  std::string path = "testdata";

  try {
    for (const auto &entry : fs::directory_iterator(path)) {
      std::string ext = entry.path().extension().string();
      if (ext == ".fa" || ext == ".fasta") {
        Benchmark::run(entry.path().string());
      }
    }
  } catch (const std::exception &e) {
    std::cerr << "Error: " << e.what() << std::endl;
  }

  return 0;
}