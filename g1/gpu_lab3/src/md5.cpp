#include "md5.hpp"

#include <algorithm>
#include <array>
#include <cctype>
#include <cstring>
#include <limits>
#include <stdexcept>

namespace lab {
namespace {
constexpr std::array<std::uint32_t, 64> K = {
    0xd76aa478, 0xe8c7b756, 0x242070db, 0xc1bdceee,
    0xf57c0faf, 0x4787c62a, 0xa8304613, 0xfd469501,
    0x698098d8, 0x8b44f7af, 0xffff5bb1, 0x895cd7be,
    0x6b901122, 0xfd987193, 0xa679438e, 0x49b40821,
    0xf61e2562, 0xc040b340, 0x265e5a51, 0xe9b6c7aa,
    0xd62f105d, 0x02441453, 0xd8a1e681, 0xe7d3fbc8,
    0x21e1cde6, 0xc33707d6, 0xf4d50d87, 0x455a14ed,
    0xa9e3e905, 0xfcefa3f8, 0x676f02d9, 0x8d2a4c8a,
    0xfffa3942, 0x8771f681, 0x6d9d6122, 0xfde5380c,
    0xa4beea44, 0x4bdecfa9, 0xf6bb4b60, 0xbebfbc70,
    0x289b7ec6, 0xeaa127fa, 0xd4ef3085, 0x04881d05,
    0xd9d4d039, 0xe6db99e5, 0x1fa27cf8, 0xc4ac5665,
    0xf4292244, 0x432aff97, 0xab9423a7, 0xfc93a039,
    0x655b59c3, 0x8f0ccc92, 0xffeff47d, 0x85845dd1,
    0x6fa87e4f, 0xfe2ce6e0, 0xa3014314, 0x4e0811a1,
    0xf7537e82, 0xbd3af235, 0x2ad7d2bb, 0xeb86d391
};
constexpr std::array<std::uint32_t, 64> S = {
    7,12,17,22,7,12,17,22,7,12,17,22,7,12,17,22,
    5,9,14,20,5,9,14,20,5,9,14,20,5,9,14,20,
    4,11,16,23,4,11,16,23,4,11,16,23,4,11,16,23,
    6,10,15,21,6,10,15,21,6,10,15,21,6,10,15,21
};

inline std::uint32_t rotate_left(std::uint32_t x, std::uint32_t amount) {
    return (x << amount) | (x >> (32U - amount));
}

int hex_value(char c) {
    if (c >= '0' && c <= '9') return c - '0';
    c = static_cast<char>(std::tolower(static_cast<unsigned char>(c)));
    if (c >= 'a' && c <= 'f') return c - 'a' + 10;
    return -1;
}
} // namespace

Md5Words md5_words(const std::string& text) {
    if (text.size() > 55) {
        throw std::invalid_argument("MD5 helper supports strings up to 55 bytes in this lab implementation");
    }

    std::array<std::uint8_t, 64> block{};
    if (!text.empty()) {
        std::memcpy(block.data(), text.data(), text.size());
    }
    block[text.size()] = 0x80;
    const std::uint64_t bit_length = static_cast<std::uint64_t>(text.size()) * 8U;
    for (int i = 0; i < 8; ++i) {
        block[56 + i] = static_cast<std::uint8_t>((bit_length >> (8 * i)) & 0xffU);
    }

    std::array<std::uint32_t, 16> M{};
    for (int i = 0; i < 16; ++i) {
        const int b = i * 4;
        M[i] = static_cast<std::uint32_t>(block[b])
             | (static_cast<std::uint32_t>(block[b + 1]) << 8U)
             | (static_cast<std::uint32_t>(block[b + 2]) << 16U)
             | (static_cast<std::uint32_t>(block[b + 3]) << 24U);
    }

    std::uint32_t a0 = 0x67452301;
    std::uint32_t b0 = 0xefcdab89;
    std::uint32_t c0 = 0x98badcfe;
    std::uint32_t d0 = 0x10325476;
    std::uint32_t a = a0, b = b0, c = c0, d = d0;

    for (std::uint32_t i = 0; i < 64; ++i) {
        std::uint32_t f = 0;
        std::uint32_t g = 0;
        if (i < 16) {
            f = (b & c) | ((~b) & d);
            g = i;
        } else if (i < 32) {
            f = (d & b) | ((~d) & c);
            g = (5U * i + 1U) % 16U;
        } else if (i < 48) {
            f = b ^ c ^ d;
            g = (3U * i + 5U) % 16U;
        } else {
            f = c ^ (b | (~d));
            g = (7U * i) % 16U;
        }
        const std::uint32_t old_d = d;
        d = c;
        c = b;
        b = b + rotate_left(a + f + K[i] + M[g], S[i]);
        a = old_d;
    }

    return {a0 + a, b0 + b, c0 + c, d0 + d};
}

std::string md5_hex(const std::string& text) {
    const Md5Words words = md5_words(text);
    static constexpr char hex[] = "0123456789abcdef";
    std::string result;
    result.reserve(32);
    for (const std::uint32_t word : words) {
        for (int byte = 0; byte < 4; ++byte) {
            const auto value = static_cast<unsigned char>((word >> (8U * byte)) & 0xffU);
            result.push_back(hex[(value >> 4U) & 0x0fU]);
            result.push_back(hex[value & 0x0fU]);
        }
    }
    return result;
}

bool is_hex_md5(const std::string& hex) {
    if (hex.size() != 32) return false;
    return std::all_of(hex.begin(), hex.end(), [](char c) { return hex_value(c) >= 0; });
}

Md5Words md5_words_from_hex(const std::string& hex) {
    if (!is_hex_md5(hex)) {
        throw std::invalid_argument("MD5-хэш должен состоять ровно из 32 шестнадцатеричных символов");
    }
    std::array<std::uint8_t, 16> bytes{};
    for (std::size_t i = 0; i < bytes.size(); ++i) {
        const int hi = hex_value(hex[i * 2]);
        const int lo = hex_value(hex[i * 2 + 1]);
        bytes[i] = static_cast<std::uint8_t>((hi << 4) | lo);
    }
    Md5Words result{};
    for (int i = 0; i < 4; ++i) {
        result[i] = static_cast<std::uint32_t>(bytes[i * 4])
                  | (static_cast<std::uint32_t>(bytes[i * 4 + 1]) << 8U)
                  | (static_cast<std::uint32_t>(bytes[i * 4 + 2]) << 16U)
                  | (static_cast<std::uint32_t>(bytes[i * 4 + 3]) << 24U);
    }
    return result;
}

std::uint64_t search_space_size(std::size_t alphabet_size, int length) {
    if (length < 0) throw std::invalid_argument("Длина не может быть отрицательной");
    std::uint64_t result = 1;
    for (int i = 0; i < length; ++i) {
        if (alphabet_size != 0 && result > std::numeric_limits<std::uint64_t>::max() / alphabet_size) {
            return std::numeric_limits<std::uint64_t>::max();
        }
        result *= static_cast<std::uint64_t>(alphabet_size);
    }
    return result;
}

std::string candidate_from_index(std::uint64_t index, const std::string& alphabet, int length) {
    std::string candidate(static_cast<std::size_t>(length), alphabet.empty() ? '\0' : alphabet.front());
    for (int pos = length - 1; pos >= 0; --pos) {
        candidate[static_cast<std::size_t>(pos)] = alphabet[static_cast<std::size_t>(index % alphabet.size())];
        index /= static_cast<std::uint64_t>(alphabet.size());
    }
    return candidate;
}

BruteForceResult md5_bruteforce_cpu(const Md5Words& target,
                                    const std::string& alphabet,
                                    int min_length,
                                    int max_length,
                                    std::uint64_t max_candidates) {
    if (alphabet.empty()) throw std::invalid_argument("Алфавит не должен быть пустым");
    if (min_length < 1 || max_length < min_length || max_length > 12) {
        throw std::invalid_argument("Ожидаются длины 1 <= min-length <= max-length <= 12");
    }
    BruteForceResult result;
    for (int length = min_length; length <= max_length; ++length) {
        const std::uint64_t total = search_space_size(alphabet.size(), length);
        for (std::uint64_t index = 0; index < total; ++index) {
            if (result.checked >= max_candidates) {
                result.limit_reached = true;
                return result;
            }
            const std::string candidate = candidate_from_index(index, alphabet, length);
            ++result.checked;
            if (md5_words(candidate) == target) {
                result.found = true;
                result.password = candidate;
                return result;
            }
        }
    }
    return result;
}
} // namespace lab
