#include <iostream>
#include <string>
#include <algorithm>
#include <chrono>
#include <vector>
#include <cctype>

// 版本 1：手动循环 + 结果拼接 (const 引用)
std::string str_keep_alpha_v1(const std::string &s) {
    std::string new_str;
    for (size_t it = 0; it < s.size(); it++) {
        if (isalpha(static_cast<unsigned char>(s[it]))) {
            new_str += s[it];
        }
    }
    return new_str;
}

// 版本 2：Erase-Remove 习语 (值传递)
inline std::string str_keep_alpha_v2(const std::string &s) {
    std::string new_str;
    new_str.reserve(s.size()); // Pre-allocate memory once

    for (unsigned char c : s) {
        if (std::isalpha(c)) {
            new_str += c;
        }
    }
    return new_str;
}

void run_test(const std::string& name, size_t iterations, const std::string& data, std::string (*func)(std::string)) {
    // 这里的 func 签名微调以匹配传值调用
}

// 简单的辅助测试宏/函数
void benchmark(int case_num, const std::string& label, const std::string& test_data, int iters) {
    std::cout << "--- 测试场景: " << label << " (长度: " << test_data.size() << ") ---" << std::endl;

    // 测试版本 1
    {
        auto start = std::chrono::high_resolution_clock::now();
        size_t total_len = 0;
        for (int i = 0; i < iters; ++i) {
            total_len += str_keep_alpha_v1(test_data).length();
        }
        auto end = std::chrono::high_resolution_clock::now();
        std::chrono::duration<double, std::milli> ms = end - start;
        std::cout << "版本 1 (手动循环): " << ms.count() << " ms | 校验位: " << total_len << std::endl;
    }

    // 测试版本 2
    {
        auto start = std::chrono::high_resolution_clock::now();
        size_t total_len = 0;
        for (int i = 0; i < iters; ++i) {
            total_len += str_keep_alpha_v2(test_data).length();
        }
        auto end = std::chrono::high_resolution_clock::now();
        std::chrono::duration<double, std::milli> ms = end - start;
        std::cout << "版本 2 (Erase-Remove): " << ms.count() << " ms | 校验位: " << total_len << std::endl;
    }
    std::cout << std::endl;
}

int main() {
    // 构造不同长度的测试数据
    auto gen_data = [](size_t len) {
        std::string s;
        for (size_t i = 0; i < len; ++i) s += (i % 3 == 0) ? 'A' : '1';
        return s;
    };

    std::string short_str = gen_data(16);       // 触发 SSO (短字符串优化)
    std::string long_str = gen_data(100000);    // 10万字符，触发大量动态分配

    benchmark(1, "短字符串", short_str, 1000000);
    benchmark(2, "长字符串", long_str, 1000);

    return 0;
}