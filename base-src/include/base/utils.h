#pragma once

#include <array>
#include <string>
#include <string_view>
#include <bit>

namespace uranus::utils {
    namespace crypto {
        /**
         * 参考自Boost.Beast
         * https://github.com/boostorg/beast/blob/develop/include/boost/beast/core/detail/base64.hpp
         */
        inline std::string Base64Encode(const std::string_view input) {
            static constexpr auto lookup = "ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz0123456789+/";

            std::string result;

            const auto len = input.size();
            result.resize(4 * ((len + 2) / 3));

            auto out = result.data();
            auto in = input.data();

            for (auto n = len / 3; n--;) {
                *out++ = lookup[(in[0] & 0xfc) >> 2];
                *out++ = lookup[((in[0] & 0x03) << 4) + ((in[1] & 0xf0) >> 4)];
                *out++ = lookup[((in[2] & 0xc0) >> 6) + ((in[1] & 0x0f) << 2)];
                *out++ = lookup[in[2] & 0x3f];
                in += 3;
            }

            switch (len % 3) {
                case 2:
                    *out++ = lookup[(in[0] & 0xfc) >> 2];
                    *out++ = lookup[((in[0] & 0x03) << 4) + ((in[1] & 0xf0) >> 4)];
                    *out++ = lookup[(in[1] & 0x0f) << 2];
                    *out++ = '=';
                    break;

                case 1:
                    *out++ = lookup[(in[0] & 0xfc) >> 2];
                    *out++ = lookup[((in[0] & 0x03) << 4)];
                    *out++ = '=';
                    *out++ = '=';
                    break;

                case 0:
                default: break;
            }

            result.resize(out - result.data());
            return result;
        }

        /**
         * 参考自Boost.Beast
         * https://github.com/boostorg/beast/blob/develop/include/boost/beast/core/detail/base64.hpp
         */
        inline std::string Base64Decode(const std::string_view input) {
            static constexpr signed char lookup[] = {
                -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, //   0-15
                -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, //  16-31
                -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, 62, -1, -1, -1, 63, //  32-47
                52, 53, 54, 55, 56, 57, 58, 59, 60, 61, -1, -1, -1, -1, -1, -1, //  48-63
                -1,  0,  1,  2,  3,  4,  5,  6,  7,  8,  9, 10, 11, 12, 13, 14, //  64-79
                15, 16, 17, 18, 19, 20, 21, 22, 23, 24, 25, -1, -1, -1, -1, -1, //  80-95
                -1, 26, 27, 28, 29, 30, 31, 32, 33, 34, 35, 36, 37, 38, 39, 40, //  96-111
                41, 42, 43, 44, 45, 46, 47, 48, 49, 50, 51, -1, -1, -1, -1, -1, // 112-127
                -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, // 128-143
                -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, // 144-159
                -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, // 160-175
                -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, // 176-191
                -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, // 192-207
                -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, // 208-223
                -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, // 224-239
                -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1  // 240-255
            };

            auto len = input.length();

            std::string result;
            result.resize((len + 3) / 4 * 3);

            auto out = result.data();
            auto in = input.data();

            unsigned char c3[3], c4[4] = {0, 0, 0, 0};
            int i = 0;
            int j = 0;

            while (len-- && *in != '=') {
                auto const v = lookup[*in];
                if (v == -1)
                    break;
                ++in;
                c4[i] = static_cast<unsigned char>(v);
                if (++i == 4) {
                    c3[0] = (c4[0] << 2) + ((c4[1] & 0x30) >> 4);
                    c3[1] = ((c4[1] & 0xf) << 4) + ((c4[2] & 0x3c) >> 2);
                    c3[2] = ((c4[2] & 0x3) << 6) + c4[3];

                    for (i = 0; i < 3; i++)
                        *out++ = static_cast<char>(c3[i]);
                    i = 0;
                }
            }

            if (i) {
                c3[0] = (c4[0] << 2) + ((c4[1] & 0x30) >> 4);
                c3[1] = ((c4[1] & 0xf) << 4) + ((c4[2] & 0x3c) >> 2);
                c3[2] = ((c4[2] & 0x3) << 6) + c4[3];

                for (j = 0; j < i - 1; j++)
                    *out++ = static_cast<char>(c3[j]);
            }

            result.resize(out - result.data());
            return result;
        }

        // Pure inline SHA-1 calculation function (no header-only class, no ODR risk)
        inline std::array<uint8_t, 20> Sha1Calculate(std::string_view sv) {
            // Initial hash values
            uint32_t h0 = 0x67452301u;
            uint32_t h1 = 0xEFCDAB89u;
            uint32_t h2 = 0x98BADCFEu;
            uint32_t h3 = 0x10325476u;
            uint32_t h4 = 0xC3D2E1F0u;

            auto transform = [&](const uint8_t block[64]) {
                uint32_t w[80];
                for (int i = 0; i < 16; ++i) {
                    w[i] = (static_cast<uint32_t>(block[i * 4]) << 24) |
                           (static_cast<uint32_t>(block[i * 4 + 1]) << 16) |
                           (static_cast<uint32_t>(block[i * 4 + 2]) << 8) |
                           (static_cast<uint32_t>(block[i * 4 + 3]));
                }
                for (int i = 16; i < 80; ++i) {
                    w[i] = std::rotl(w[i - 3] ^ w[i - 8] ^ w[i - 14] ^ w[i - 16], 1);
                }

                uint32_t a = h0, b = h1, c = h2, d = h3, e = h4;
                for (int i = 0; i < 80; ++i) {
                    uint32_t f, k;
                    if (i < 20) {
                        f = (b & c) | ((~b) & d);
                        k = 0x5A827999u;
                    } else if (i < 40) {
                        f = b ^ c ^ d;
                        k = 0x6ED9EBA1u;
                    } else if (i < 60) {
                        f = (b & c) | (b & d) | (c & d);
                        k = 0x8F1BBCDCu;
                    } else {
                        f = b ^ c ^ d;
                        k = 0xCA62C1D6u;
                    }
                    const uint32_t temp = std::rotl(a, 5) + f + e + k + w[i];
                    e = d;
                    d = c;
                    c = std::rotl(b, 30);
                    b = a;
                    a = temp;
                }
                h0 += a;
                h1 += b;
                h2 += c;
                h3 += d;
                h4 += e;
            };

            // Process full 64-byte blocks directly from input
            const auto *data = reinterpret_cast<const uint8_t *>(sv.data());
            size_t len = sv.size();
            const uint64_t bit_len = len * 8ull;

            while (len >= 64) {
                transform(data);
                data += 64;
                len -= 64;
            }

            // Pad the final block(s)
            uint8_t buffer[64] = {0};
            size_t idx = 0;
            // Copy remaining bytes
            for (idx = 0; idx < len; ++idx) buffer[idx] = data[idx];
            // Append 0x80
            buffer[idx++] = 0x80;
            if (idx > 56) {
                // Not enough room for length, process this block and clear next
                for (size_t i = idx; i < 64; ++i) buffer[i] = 0;
                transform(buffer);
                // Prepare a new zeroed block
                for (size_t i = 0; i < 56; ++i) buffer[i] = 0;
                idx = 56;
            } else {
                // Pad zeros until 56
                for (size_t i = idx; i < 56; ++i) buffer[i] = 0;
                idx = 56;
            }
            // Append big-endian 64-bit message length
            for (int i = 7; i >= 0; --i) {
                buffer[idx++] = static_cast<uint8_t>((bit_len >> (i * 8)) & 0xFF);
            }

            transform(buffer);

            // Produce digest in big-endian
            auto store_be32 = [](uint32_t v, uint8_t *p) {
                p[0] = static_cast<uint8_t>((v >> 24) & 0xFF);
                p[1] = static_cast<uint8_t>((v >> 16) & 0xFF);
                p[2] = static_cast<uint8_t>((v >> 8) & 0xFF);
                p[3] = static_cast<uint8_t>(v & 0xFF);
            };

            std::array<uint8_t, 20> out{};

            store_be32(h0, out.data() + 0);
            store_be32(h1, out.data() + 4);
            store_be32(h2, out.data() + 8);
            store_be32(h3, out.data() + 12);
            store_be32(h4, out.data() + 16);

            return out;
        }
    }

    inline std::string_view FileExtension(const std::string_view path) {
        const auto pos = path.rfind('.');
        if (pos == std::string_view::npos || pos + 1 == path.size())
            return {};
        return path.substr(pos + 1);
    }

    inline std::string_view TrimDoubleQuotes(const std::string_view sv) {
        if (sv.length() >= 2 && sv.front() == '"' && sv.back() == '"') {
            return sv.substr(1, sv.size() - 2);
        }
        return {sv.data(), sv.size()};
    }

    inline std::string_view TrimString(const std::string_view sv) {
        auto is_space = [](const unsigned char c) { return std::isspace(c); };

        size_t start = 0;
        size_t end = sv.size();

        while (start < end && is_space(static_cast<unsigned char>(sv[start])))
            ++start;

        while (end > start && is_space(static_cast<unsigned char>(sv[end - 1])))
            --end;

        return sv.substr(start, end - start);
    }

    template<typename Func>
    requires std::invocable<Func, std::string_view>
    void SplitString(std::string_view sv, const char delim, const size_t max_parts, Func &&fn) {
        size_t count = 1;

        while (!sv.empty() && count < max_parts) {
            const size_t pos = sv.find(delim);
            if (pos == std::string_view::npos)
                break;

            if (auto token = TrimString(sv.substr(0, pos)); !token.empty())
                fn(token);

            sv.remove_prefix(pos + 1);
            ++count;
        }

        // The last part
        if (auto token = TrimString(sv); !token.empty())
            fn(token);
    }

    template<typename Func>
    requires std::invocable<Func, std::string_view>
    void SplitString(std::string_view sv, const char delim, Func &&fn) {
        SplitString(sv, delim, (std::numeric_limits<size_t>::max)(), std::forward<Func>(fn));
    }

    template<typename Func>
    requires std::invocable<Func, std::string_view, std::string_view>
    void DivideString(std::string_view sv, const char delim, Func &&fn) {
        if (const auto pos = sv.find(delim); pos != std::string_view::npos) {
            auto lhs = sv.substr(0, pos);
            auto rhs = sv.substr(pos + 1);
            fn(lhs, rhs);
        } else {
            fn(sv, std::string_view{});
        }
    }

    constexpr unsigned int StringToTag_Internal(const char *s, const size_t l, const unsigned int h) {
        // Unsets the 6 high bits of h, therefore no overflow happens
        return (l == 0) ? h : StringToTag_Internal(s + 1, l - 1,(((std::numeric_limits<unsigned int>::max)() >> 6) & h * 33) ^ static_cast<unsigned char>(*s));
    }

    inline unsigned int StringToTag(const std::string &str) {
        return StringToTag_Internal(str.data(), str.size(), 0);
    }

    inline unsigned int StringToTag(const std::string_view sv) {
        return StringToTag_Internal(sv.data(), sv.size(), 0);
    }

    namespace udl {
        constexpr unsigned int operator""_t(const char *s, const size_t l) {
            return StringToTag_Internal(s, l, 0);
        }
    }
}
