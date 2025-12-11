#pragma once

#include "http.export.h"

#include <map>
#include <vector>
#include <string>
#include <unordered_map>
#include <unordered_set>

namespace uranus::http {
    namespace detail {
        namespace case_ignore {
            inline unsigned char to_lower(const int c) {
                constexpr static unsigned char table[256] = {
                    0,   1,   2,  3,    4,   5,   6,   7,   8,   9,   10,  11,  12,  13,  14,
                    15,  16,  17,  18,  19,  20,  21,  22,  23,  24,  25,  26,  27,  28,  29,
                    30,  31,  32,  33,  34,  35,  36,  37,  38,  39,  40,  41,  42,  43,  44,
                    45,  46,  47,  48,  49,  50,  51,  52,  53,  54,  55,  56,  57,  58,  59,
                    60,  61,  62,  63,  64,  97,  98,  99,  100, 101, 102, 103, 104, 105, 106,
                    107, 108, 109, 110, 111, 112, 113, 114, 115, 116, 117, 118, 119, 120, 121,
                    122, 91,  92,  93,  94,  95,  96,  97,  98,  99,  100, 101, 102, 103, 104,
                    105, 106, 107, 108, 109, 110, 111, 112, 113, 114, 115, 116, 117, 118, 119,
                    120, 121, 122, 123, 124, 125, 126, 127, 128, 129, 130, 131, 132, 133, 134,
                    135, 136, 137, 138, 139, 140, 141, 142, 143, 144, 145, 146, 147, 148, 149,
                    150, 151, 152, 153, 154, 155, 156, 157, 158, 159, 160, 161, 162, 163, 164,
                    165, 166, 167, 168, 169, 170, 171, 172, 173, 174, 175, 176, 177, 178, 179,
                    180, 181, 182, 183, 184, 185, 186, 187, 188, 189, 190, 191, 224, 225, 226,
                    227, 228, 229, 230, 231, 232, 233, 234, 235, 236, 237, 238, 239, 240, 241,
                    242, 243, 244, 245, 246, 215, 248, 249, 250, 251, 252, 253, 254, 223, 224,
                    225, 226, 227, 228, 229, 230, 231, 232, 233, 234, 235, 236, 237, 238, 239,
                    240, 241, 242, 243, 244, 245, 246, 247, 248, 249, 250, 251, 252, 253, 254,
                    255,
                };

                return table[static_cast<unsigned char>(static_cast<char>(c))];
            }

            inline bool equal(const std::string &a, const std::string &b) {
                return a.size() == b.size() &&
                    std::equal(a.begin(), a.end(), b.begin(), [](const char ca, const char cb) {
                        return to_lower(ca) == to_lower(cb);
                    });
            }

            inline bool equal(const std::string_view a, const std::string_view b) {
                return a.size() == b.size() &&
                    std::equal(a.begin(), a.end(), b.begin(), [](const char ca, const char cb) {
                        return to_lower(ca) == to_lower(cb);
                    });
            }

            inline bool contains(const std::string_view src, const std::string_view sub) {
                if (src.empty())
                    return false;

                if (sub.empty())
                    return true;

                if (sub.size() > src.size())
                    return false;

                for (auto idx = 0; idx <= (src.size() - sub.size()); ++idx) {
                    if (equal(src.substr(idx, sub.size()), sub))
                        return true;
                }

                return false;
            }

            struct equal_to {
                using is_transparent = void;

                bool operator()(const std::string &a, const std::string &b) const {
                    return equal(a, b);
                }

                bool operator()(const std::string_view a, const std::string &b) const {
                    return equal(a, b);
                }

                bool operator()(const std::string &a, const std::string_view b) const {
                    return equal(a, b);
                }

                bool operator()(const std::string_view a, const std::string_view b) const {
                    return equal(a, b);
                }
            };

            struct hash {
                using is_transparent = void;

                size_t operator()(const std::string &key) const {
                    return hash_core(key.data(), key.size(), 0);
                }

                size_t operator()(const std::string_view key) const {
                    return hash_core(key.data(), key.size(), 0);
                }

                size_t hash_core(const char *s, const size_t l, const size_t h) const {
                    return (l == 0) ? h : hash_core(s + 1, l - 1, (((std::numeric_limits<size_t>::max)() >> 6) & h * 33) ^ to_lower(*s));
                }
            };

            using unordered_set = std::unordered_set<std::string, hash, equal_to>;
        }

        namespace case_sensitive {
            inline bool equal(const std::string_view a, const std::string_view b) {
                return a.size() == b.size() &&
                    std::equal(a.begin(), a.end(), b.begin(),
                    [](const char ca, const char cb) {
                        return ca == cb;
                    });
            }

            struct equal_to {
                using is_transparent = void;

                bool operator()(const std::string &a, const std::string &b) const {
                    return equal(a, b);
                }

                bool operator()(const std::string_view a, const std::string &b) const {
                    return equal(a, b);
                }

                bool operator()(const std::string &a, const std::string_view b) const {
                    return equal(a, b);
                }

                bool operator()(const std::string_view a, const std::string_view b) const {
                    return equal(a, b);
                }
            };

            struct hash {
                using is_transparent = void;

                size_t operator()(const std::string &key) const {
                    return hash_core(key.data(), key.size(), 0);
                }

                size_t operator()(const std::string_view key) const {
                    return hash_core(key.data(), key.size(), 0);
                }

                size_t hash_core(const char *s, const size_t l, const size_t h) const {
                    return (l == 0) ? h : hash_core(s + 1, l - 1, (((std::numeric_limits<size_t>::max)() >> 6) & h * 33) ^ (*s));
                }
            };

            struct transparent_less {
                using is_transparent = void;

                bool operator()(const std::string &lhs, const std::string &rhs) const noexcept {
                    return lhs < rhs;
                }

                bool operator()(const std::string_view lhs, const std::string &rhs) const noexcept {
                    return lhs < rhs;
                }

                bool operator()(const std::string &lhs, const std::string_view rhs) const noexcept {
                    return lhs < rhs;
                }

                bool operator()(const std::string_view lhs, const std::string_view rhs) const noexcept {
                    return lhs < rhs;
                }
            };
        }
    }

    using Params        = std::multimap<std::string, std::string, detail::case_sensitive::transparent_less>;
    using PathParams    = std::unordered_map<std::string, std::string, detail::case_sensitive::hash, detail::case_sensitive::equal_to>;
    using Headers       = std::unordered_multimap<std::string, std::string, detail::case_ignore::hash, detail::case_ignore::equal_to>;

    struct FormData {
        std::string name;
        std::string content;
        std::string filename;
        std::string type;
        Headers     headers;
    };

    struct FormField {
        std::string name;
        std::string content;
        Headers     headers;
    };

    using FormFields    = std::multimap<std::string, FormField, detail::case_sensitive::transparent_less>;
    using FormFiles     = std::multimap<std::string, FormData, detail::case_sensitive::transparent_less>;
    using Ranges        = std::vector<std::pair<size_t, size_t>>;

    struct HTTP_API MultipartFormData {
        /// Text fields from multipart
        FormFields fields;

        /// Files from multipart
        FormFiles files;

        /// Text field access
        [[nodiscard]] std::string               getField        (std::string_view key, size_t id = 0) const;
        [[nodiscard]] std::vector<std::string>  getFields       (std::string_view key) const;
        [[nodiscard]] bool                      hasField        (std::string_view key) const;
        [[nodiscard]] size_t                    getFieldCount   (std::string_view key) const;

        /// File access
        [[nodiscard]] FormData              getFile     (std::string_view key, size_t id = 0) const;
        [[nodiscard]] std::vector<FormData> getFiles    (std::string_view key) const;
        [[nodiscard]] bool                  hasFile     (std::string_view key) const;
        [[nodiscard]] size_t                getFileCount(std::string_view key) const;
    };
}
