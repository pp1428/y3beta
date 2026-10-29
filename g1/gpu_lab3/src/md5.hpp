#pragma once

#include <array>
#include <cstdint>
#include <string>
#include <vector>

namespace lab {
using Md5Words = std::array<std::uint32_t, 4>;

Md5Words md5_words(const std::string& text);
std::string md5_hex(const std::string& text);
Md5Words md5_words_from_hex(const std::string& hex);
bool is_hex_md5(const std::string& hex);

struct BruteForceResult {
    bool found = false;
    std::string password;
    std::uint64_t checked = 0;
    bool limit_reached = false;
};

std::uint64_t search_space_size(std::size_t alphabet_size, int length);
std::string candidate_from_index(std::uint64_t index, const std::string& alphabet, int length);
BruteForceResult md5_bruteforce_cpu(const Md5Words& target,
                                    const std::string& alphabet,
                                    int min_length,
                                    int max_length,
                                    std::uint64_t max_candidates);
} // namespace lab
