#ifndef UTIL_H
#define UTIL_H

#include <algorithm>
#include <chrono>
#include <filesystem>
#include <iostream>
#include <mutex>
#include <regex>
#include <string>
#include <string_view>
#include <vector>

struct TableGenerator {
  using Table = std::array<char, 256>;

  static constexpr Table makeFilterTable(bool forceUpper) {
    Table table{};

    const char *valid =
        "ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz-*";

    for (int i = 0; i < 54; ++i) { // 字符集长度为 54
      unsigned char c = static_cast<unsigned char>(valid[i]);
      if (forceUpper) {
        if (c >= 'a' && c <= 'z')
          table[c] = static_cast<char>(c - 32);
        else
          table[c] = static_cast<char>(c);
      } else {
        table[c] = static_cast<char>(c);
      }
    }
    return table;
  }
};

inline constexpr std::array<char, 256> FilterTable =
    TableGenerator::makeFilterTable(false);
inline constexpr std::array<char, 256> UpperTable =
    TableGenerator::makeFilterTable(true);

[[nodiscard]] constexpr char complement(char base) {
  switch (base) {
  case 'A':
  case 'a':
    return 'T';
  case 'T':
  case 't':
    return 'A';
  case 'C':
  case 'c':
    return 'G';
  case 'G':
  case 'g':
    return 'C';
  default:
    return 'N';
  }
}

[[nodiscard]] inline bool starts_with(std::string_view value,
                                      std::string_view starting) {
  return value.size() >= starting.size() &&
         value.compare(0, starting.size(), starting) == 0;
}

[[nodiscard]] inline bool starts_with(const std::string *value,
                                      std::string_view starting) {
  if (value == nullptr)
    return false;
  return starts_with(std::string_view(*value), starting);
}

[[nodiscard]] inline bool ends_with(std::string_view value,
                                    std::string_view ending) {
  return value.size() >= ending.size() &&
         value.compare(value.size() - ending.size(), ending.size(), ending) ==
             0;
}

[[nodiscard]] inline std::string_view trim(std::string_view str) {
  const auto pos = str.find_first_not_of(' ');
  if (pos == std::string_view::npos)
    return "";
  const auto pos2 = str.find_last_not_of(' ');
  return str.substr(pos, pos2 - pos + 1);
}

inline void split(const std::string &str, std::vector<std::string> &ret,
                  std::string_view sep = ",") {
  ret.clear();
  size_t start = str.find_first_not_of(sep);
  while (start != std::string::npos) {
    size_t end = str.find(sep, start);
    if (end == std::string::npos) {
      ret.emplace_back(str.substr(start));
      break;
    }
    ret.emplace_back(str.substr(start, end - start));
    start = str.find_first_not_of(sep, end);
  }
}
[[nodiscard]] inline std::string
replace(std::string_view str, std::string_view src, std::string_view dest) {
  if (src.empty())
    return std::string(str);

  std::string result;
  size_t count = 0;
  size_t pos = str.find(src);
  while (pos != std::string_view::npos) {
    count++;
    pos = str.find(src, pos + src.size());
  }

  if (count == 0)
    return std::string(str);

  size_t new_size = str.size() + count * (dest.size() - src.size());
  result.reserve(new_size);

  size_t last_pos = 0;
  pos = str.find(src);
  while (pos != std::string_view::npos) {
    result.append(str.data() + last_pos, pos - last_pos);
    result.append(dest);

    last_pos = pos + src.size();
    pos = str.find(src, last_pos);
  }

  result.append(str.data() + last_pos, str.size() - last_pos);
  return result;
}

[[nodiscard]] inline std::string reverse(std::string_view str) {
  return std::string(str.rbegin(), str.rend());
}

inline void str2upper(std::string &s) {
  std::transform(s.begin(), s.end(), s.begin(),
                 [](unsigned char c) { return std::toupper(c); });
}

inline void str2lower(std::string &s) {
  std::transform(s.begin(), s.end(), s.begin(),
                 [](unsigned char c) { return std::tolower(c); });
}

// Remove non alphabetic characters from a string
inline std::string str_keep_alpha(const std::string &s) {
  std::string new_str;
  for (size_t it = 0; it < s.size(); it++) {
    if (isalpha(s[it])) {
      new_str += s[it];
    }
  }
  return new_str;
}

// Remove invalid sequence characters from a string
inline void str_keep_valid_sequence(std::string &s,
                                    bool forceUpperCase = false) {
  size_t total = 0;
  const char case_gap = 'a' - 'A';
  for (size_t it = 0; it < s.size(); it++) {
    char c = s[it];
    if (forceUpperCase && c >= 'a' && c <= 'z') {
      c -= case_gap;
    }
    if (isalpha(c) || c == '-' || c == '*') {
      s[total] = c;
      total++;
    }
  }

  s.resize(total);
}

inline int find_with_right_pos(std::string_view str, std::string_view pattern,
                               size_t start = 0) {
  if (auto pos = str.find(pattern, start); pos != std::string_view::npos)
    return static_cast<int>(pos + pattern.length());
  return -1;
}

inline char num2qual(int num) {
  if (num > 127 - 33)
    num = 127 - 33;
  if (num < 0)
    num = 0;

  char c = num + 33;
  return c;
}

// --- 文件系统操作 (std::filesystem) ---
[[nodiscard]] inline std::string basename(const std::string &filename) {
  return std::filesystem::path(filename).filename().string();
}

[[nodiscard]] inline std::string dirname(const std::string &filename) {
  auto p = std::filesystem::path(filename).parent_path();
  return p.empty() ? "./" : p.string() + "/";
}

[[nodiscard]] inline std::string joinpath(const std::string &dir,
                                          const std::string &base) {
  return (std::filesystem::path(dir) / base).string();
}

// Check if a string is a file or directory
[[nodiscard]] inline bool file_exists(const std::string &s) {
  return std::filesystem::exists(s);
}

// check if a string is a directory
[[nodiscard]] inline bool is_directory(const std::string &path) {
  return std::filesystem::is_directory(path);
}

inline void check_file_valid(const std::string &s) {
  if (!file_exists(s)) {
    std::cerr << "ERROR: file '" << s << "' doesn't exist, quit now"
              << std::endl;
    exit(-1);
  }
  if (is_directory(s)) {
    std::cerr << "ERROR: '" << s << "' is a folder, not a file, quit now"
              << std::endl;
    exit(-1);
  }
}

inline bool check_filename_valid(const std::string &s) {
  std::string_view t = trim(s);
  return !t.empty() && t.length() <= 255 &&
         std::regex_match(s, std::regex("^[A-Za-z0-9_\\.\\-]+$"));
}

inline void check_file_writable(const std::string &s) {
  std::string dir = dirname(s);
  if (!file_exists(dir)) {
    std::cerr
        << "ERROR: '" << dir
        << " doesn't exist. Create this folder and run this command again."
        << std::endl;
    exit(-1);
  }
  if (is_directory(s)) {
    std::cerr << "ERROR: '" << s << "' is not a writable file, quit now"
              << std::endl;
    exit(-1);
  }
}

// --- 系统与日志 ---
inline void error_exit(const std::string &msg) {
  std::cerr << "ERROR: " << msg << std::endl;
  exit(-1);
}

extern std::mutex logmtx;
inline void loginfo(std::string_view s) {
  std::lock_guard<std::mutex> lock(logmtx);
  auto now =
      std::chrono::system_clock::to_time_t(std::chrono::system_clock::now());
  std::tm t;
#ifdef _MSC_VER
  localtime_s(&t, &now);
#else
  localtime_r(&now, &t);
#endif
  std::fprintf(stderr, "[%02d:%02d:%02d] %.*s \n", t.tm_hour, t.tm_min,
               t.tm_sec, static_cast<int>(s.size()), s.data());
}

#endif /* UTIL_H */
