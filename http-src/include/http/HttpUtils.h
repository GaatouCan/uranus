#pragma once

#include "FormData.h"

#include <base/utils.h>
#include <cassert>
#include <sstream>
#include <iomanip>
#include <set>
#include <regex>
#include <chrono>
#include <format>

namespace uranus::http {

    namespace detail {
        inline bool IsHex(const char c, int &v) {
            if (0x20 <= c && std::isdigit(c)) {
                v = c - '0';
                return true;
            }
            if ('A' <= c && c <= 'F') {
                v = c - 'A' + 10;
                return true;
            }
            if ('a' <= c && c <= 'f') {
                v = c - 'a' + 10;
                return true;
            }
            return false;
        }

        inline bool FromHexToInt(const std::string_view sv, size_t idx, size_t count, int &val) {
            if (idx >= sv.size()) {
                return false;
            }

            val = 0;
            for (; count; idx++, count--) {
                if (!sv[idx]) {
                    return false;
                }
                if (auto v = 0; IsHex(sv[idx], v)) {
                    val = val * 16 + v;
                } else {
                    return false;
                }
            }
            return true;
        }

        inline std::string FromIntToHex(size_t n) {
            static constexpr auto charset = "0123456789abcdef";
            std::string ret;
            do {
                ret = charset[n & 15] + ret;
                n >>= 4;
            } while (n > 0);
            return ret;
        }

        inline size_t ToUTF8(const int code, char *buff) {
            if (code < 0x0080) {
                buff[0] = static_cast<char>(code & 0x7F);
                return 1;
            }
            if (code < 0x0800) {
                buff[0] = static_cast<char>(0xC0 | ((code >> 6) & 0x1F));
                buff[1] = static_cast<char>(0x80 | (code & 0x3F));
                return 2;
            }
            if (code < 0xD800) {
                buff[0] = static_cast<char>(0xE0 | ((code >> 12) & 0xF));
                buff[1] = static_cast<char>(0x80 | ((code >> 6) & 0x3F));
                buff[2] = static_cast<char>(0x80 | (code & 0x3F));
                return 3;
            }
            if (code < 0xE000) {
                // D800 - DFFF is invalid...
                return 0;
            }
            if (code < 0x10000) {
                buff[0] = static_cast<char>(0xE0 | ((code >> 12) & 0xF));
                buff[1] = static_cast<char>(0x80 | ((code >> 6) & 0x3F));
                buff[2] = static_cast<char>(0x80 | (code & 0x3F));
                return 3;
            }
            if (code < 0x110000) {
                buff[0] = static_cast<char>(0xF0 | ((code >> 18) & 0x7));
                buff[1] = static_cast<char>(0x80 | ((code >> 12) & 0x3F));
                buff[2] = static_cast<char>(0x80 | ((code >> 6) & 0x3F));
                buff[3] = static_cast<char>(0x80 | (code & 0x3F));
                return 4;
            }

            return 0;
        }

        template<size_t len>
        constexpr size_t StringLength(const char(&)[len]) {
            return len - 1;
        }

        inline bool IsNumeric(const std::string_view sv) {
            if (sv.empty()) {
                return false;
            }
            return std::ranges::all_of(sv, [](const char c) { return std::isdigit(c); });
        }

        inline bool IsValidPath(const std::string_view path) {
            size_t level = 0;
            size_t idx = 0;

            // Skip slash
            while (idx < path.size() && path[idx] == '/') {
                idx++;
            }

            while (idx < path.size()) {
                // Read component
                const auto beg = idx;
                while (idx < path.size() && path[idx] != '/') {
                    if (path[idx] == '\0' || path[idx] == '\\') {
                        return false;
                    }
                    idx++;
                }

                const auto len = idx - beg;
                assert(len > 0);

                if (!path.compare(beg, len, ".")) {
                    ;
                } else if (!path.compare(beg, len, "..")) {
                    if (level == 0) { return false; }
                    level--;
                } else {
                    level++;
                }

                // Skip slash
                while (idx < path.size() && path[idx] == '/') {
                    idx++;
                }
            }

            return true;
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

    namespace field {
        inline bool IsTokenChar(const char c) {
            return std::isalnum(c) ||
                   c == '!' || c == '#' || c == '$' || c == '%' ||
                   c == '&' || c == '\'' || c == '*' || c == '+' || c == '-' ||
                   c == '.' || c == '^' || c == '_' || c == '`' || c == '|' || c == '~';
        }

        inline bool IsToken(const std::string_view sv) {
            if (sv.empty()) {
                return false;
            }
            return std::ranges::all_of(sv, IsTokenChar);
        }

        inline bool IsFieldName(const std::string_view sv) {
            return IsToken(sv);
        }

        inline bool IsVChar(const char c) {
            return c >= 33 && c <= 126;
        }

        inline bool IsObsText(const char c) {
            return 128 <= static_cast<unsigned char>(c);
        }

        inline bool IsFieldVChar(const char c) {
            return IsVChar(c) || IsObsText(c);
        }

        inline bool IsFieldContent(const std::string_view sv) {
            if (sv.empty()) { return true; }

            if (sv.size() == 1) {
                return IsFieldVChar(sv[0]);
            }

            if (sv.size() == 2) {
                return IsFieldVChar(sv[0]) && IsFieldVChar(sv[1]);
            }

            size_t idx = 0;

            if (!IsFieldVChar(sv[idx])) {
                return false;
            }

            idx++;
            while (idx < sv.size() - 1) {
                if (const auto c = sv[idx++]; c == ' ' || c == '\t' || IsFieldVChar(c)) {
                } else {
                    return false;
                }
            }
            return IsFieldVChar(sv[idx]);
        }

        inline bool IsFieldValue(const std::string_view sv) {
            return IsFieldContent(sv);
        }
    }

#pragma region Parser functions
    inline std::string EncodeUriComponent(const std::string_view value) {
        std::ostringstream escaped;

        escaped.fill('0');
        escaped << std::hex;

        for (const auto c: value) {
            if (std::isalnum(static_cast<uint8_t>(c)) || c == '-' || c == '_' ||
                c == '.' || c == '!' || c == '~' || c == '*' || c == '\'' || c == '(' ||
                c == ')') {
                escaped << c;
            } else {
                escaped << std::uppercase;
                escaped << '%' << std::setw(2) << static_cast<int>(static_cast<unsigned char>(c));
                escaped << std::nouppercase;
            }
        }

        return escaped.str();
    }

    inline std::string EncodeUri(const std::string_view value) {
        std::ostringstream escaped;

        escaped.fill('0');
        escaped << std::hex;

        for (const auto c: value) {
            if (std::isalnum(static_cast<uint8_t>(c)) || c == '-' || c == '_' ||
                c == '.' || c == '!' || c == '~' || c == '*' || c == '\'' || c == '(' ||
                c == ')' || c == ';' || c == '/' || c == '?' || c == ':' || c == '@' ||
                c == '&' || c == '=' || c == '+' || c == '$' || c == ',' || c == '#') {
                escaped << c;
            } else {
                escaped << std::uppercase;
                escaped << '%' << std::setw(2) << static_cast<int>(static_cast<unsigned char>(c));
                escaped << std::nouppercase;
            }
        }

        return escaped.str();
    }

    inline std::string DecodeUriComponent(const std::string_view value) {
        std::string result;

        for (size_t idx = 0; idx < value.size(); idx++) {
            if (value[idx] == '%' && idx + 2 < value.size()) {
                if (auto val = 0; detail::FromHexToInt(value, idx + 1, 2, val)) {
                    result += static_cast<char>(val);
                    idx += 2;
                } else {
                    result += value[idx];
                }
            } else {
                result += value[idx];
            }
        }

        return result;
    }

    inline std::string DecodeUri(const std::string_view value) {
        std::string result;

        for (size_t idx = 0; idx < value.size(); idx++) {
            if (value[idx] == '%' && idx + 2 < value.size()) {
                if (auto val = 0; detail::FromHexToInt(value, idx + 1, 2, val)) {
                    result += static_cast<char>(val);
                    idx += 2;
                } else {
                    result += value[idx];
                }
            } else {
                result += value[idx];
            }
        }

        return result;
    }

    inline std::string EncodePathComponent(const std::string_view comp) {
        std::string result;
        result.reserve(comp.size() * 3);

        for (const char idx: comp) {
            // Unreserved characters per RFC 3986: ALPHA / DIGIT / "-" / "." / "_" / "~"
            if (const auto c = static_cast<unsigned char>(idx);
                std::isalnum(c) || c == '-' || c == '.' || c == '_' || c == '~') {
                result += static_cast<char>(c);
            }
            // Path-safe sub-delimiters: "!" / "$" / "&" / "'" / "(" / ")" / "*" / "+" /
            // "," / ";" / "="
            else if (c == '!' || c == '$' || c == '&' || c == '\'' || c == '(' ||
                     c == ')' || c == '*' || c == '+' || c == ',' || c == ';' ||
                     c == '=') {
                result += static_cast<char>(c);
            }
            // Colon is allowed in path segments except first segment
            else if (c == ':') {
                result += static_cast<char>(c);
            }
            // @ is allowed in path
            else if (c == '@') {
                result += static_cast<char>(c);
            } else {
                result += '%';
                char hex[3];
                snprintf(hex, sizeof(hex), "%02X", c);
                result.append(hex, 2);
            }
        }
        return result;
    }

    inline std::string DecodePathComponent(const std::string_view comp) {
        std::string result;
        result.reserve(comp.size());

        for (size_t idx = 0; idx < comp.size(); idx++) {
            if (comp[idx] == '%' && idx + 1 < comp.size()) {
                if (comp[idx + 1] == 'u') {
                    // Unicode %uXXXX encoding
                    if (auto val = 0; detail::FromHexToInt(comp, idx + 2, 4, val)) {
                        // 4 digits Unicode codes
                        char buff[4];
                        if (const size_t len = detail::ToUTF8(val, buff); len > 0) {
                            result.append(buff, len);
                        }
                        idx += 5; // 'u0000'
                    } else {
                        result += comp[idx];
                    }
                } else {
                    // Standard %XX encoding
                    if (auto val = 0; detail::FromHexToInt(comp, idx + 1, 2, val)) {
                        // 2 digits hex codes
                        result += static_cast<char>(val);
                        idx += 2; // 'XX'
                    } else {
                        result += comp[idx];
                    }
                }
            } else {
                result += comp[idx];
            }
        }
        return result;
    }

    inline std::string EncodeQueryComponent(const std::string_view comp, const bool spaceAsPlus = true) {
        std::string result;
        result.reserve(comp.size() * 3);

        for (const char idx: comp) {
            // Unreserved characters per RFC 3986
            if (const auto c = static_cast<unsigned char>(idx);
                std::isalnum(c) || c == '-' || c == '.' || c == '_' || c == '~') {
                result += static_cast<char>(c);
            }
            // Space handling
            else if (c == ' ') {
                if (spaceAsPlus) {
                    result += '+';
                } else {
                    result += "%20";
                }
            }
            // Plus sign handling
            else if (c == '+') {
                if (spaceAsPlus) {
                    result += "%2B";
                } else {
                    result += static_cast<char>(c);
                }
            }
            // Query-safe sub-delimiters (excluding & and = which are query delimiters)
            else if (c == '!' || c == '$' || c == '\'' || c == '(' || c == ')' ||
                     c == '*' || c == ',' || c == ';') {
                result += static_cast<char>(c);
            }
            // Colon and @ are allowed in query
            else if (c == ':' || c == '@') {
                result += static_cast<char>(c);
            }
            // Forward slash is allowed in query values
            else if (c == '/') {
                result += static_cast<char>(c);
            }
            // Question mark is allowed in query values (after first ?)
            else if (c == '?') {
                result += static_cast<char>(c);
            } else {
                result += '%';
                char hex[3];
                snprintf(hex, sizeof(hex), "%02X", c);
                result.append(hex, 2);
            }
        }
        return result;
    }

    inline std::string DecodeQueryComponent(const std::string_view comp, const bool plusAsSpace = true) {
        std::string result;
        result.reserve(comp.size());

        for (size_t idx = 0; idx < comp.size(); idx++) {
            if (comp[idx] == '%' && idx + 2 < comp.size()) {
                auto hex = std::string(comp.substr(idx + 1, 2));
                char *end;
                const unsigned long value = std::strtoul(hex.c_str(), &end, 16);
                if (end == hex.c_str() + 2) {
                    result += static_cast<char>(value);
                    idx += 2;
                } else {
                    result += comp[idx];
                }
            } else if (comp[idx] == '+' && plusAsSpace) {
                result += ' '; // + becomes space in form-urlencoded
            } else {
                result += comp[idx];
            }
        }
        return result;
    }

    inline std::string ParamsToQueryStr(const Params &params) {
        std::string query;

        for (auto it = params.begin(); it != params.end(); ++it) {
            if (it != params.begin()) {
                query += "&";
            }
            query += EncodeQueryComponent(it->first);
            query += "=";
            query += EncodeQueryComponent(it->second);
        }
        return query;
    }

    template<typename Char, typename Traits, typename Alloc>
    std::basic_string<Char, Traits, Alloc> AppendQueryParams(
        const std::basic_string<Char, Traits, Alloc> &path,
        const Params &params)
    {
        std::basic_string<Char, Traits, Alloc> path_with_query = path;
        thread_local const std::regex re("[^?]+\\?.*");
        const auto delm = std::regex_match(path, re) ? '&' : '?';
        path_with_query += delm + ParamsToQueryStr(params);
        return path_with_query;
    }

    inline void ParseQueryText(const std::string_view data, Params &params) {
        std::set<std::string_view> cache;
        utils::SplitString(data, '&', [&](const auto sv) {
            if (cache.contains(sv)) return;
            cache.insert(sv);
            utils::DivideString(sv, '=', [&](const auto lhs, const auto rhs) {
                if (!lhs.empty()) {
                    params.emplace(DecodePathComponent(lhs), DecodePathComponent(rhs));
                }
            });
        });
    }

    template<typename Func>
    requires std::invocable<Func, std::string_view, std::string_view>
    bool ParseHeader(const std::string_view sv, Func &&fn) {
        const auto pos = sv.find(':');
        if (pos == std::string_view::npos)
            return false;

        const auto key = sv.substr(0, pos);
        if (!field::IsFieldName(key)) {
            return false;
        }

        const auto val = utils::TrimString(sv.substr(pos + 1));
        if (!field::IsFieldValue(val)) {
            return false;
        }

        if (detail::case_ignore::equal(key, "Location") ||
            detail::case_ignore::equal(key, "Referer")) {
            fn(key, val);
        } else {
            fn(key, DecodePathComponent(val));
        }

        return true;
    }

    inline bool IsProhibitedHeaderName(const std::string &name) {
        using detail::udl::operator""_t;
        switch (detail::StringToTag(name)) {
            case "REMOTE_ADDR"_t:
            case "REMOTE_PORT"_t:
            case "LOCAL_ADDR"_t:
            case "LOCAL_PORT"_t: return true;
            default: return false;
        }
    }

    inline bool IsProhibitedHeaderName(const std::string_view name) {
        using detail::udl::operator""_t;
        switch (detail::StringToTag(name)) {
            case "REMOTE_ADDR"_t:
            case "REMOTE_PORT"_t:
            case "LOCAL_ADDR"_t:
            case "LOCAL_PORT"_t: return true;
            default: return false;
        }
    }

    template<typename Func>
    requires std::invocable<Func, std::string_view>
    bool ParseMultipartBoundary(const std::string_view sv, Func &&fn) {
        static constexpr auto keyword = "boundary=";
        const auto pos = sv.find(keyword);
        if (pos == std::string::npos) { return false; }
        const auto end = sv.find(';', pos);
        const auto beg = pos + strlen(keyword);
        auto boundary = utils::TrimDoubleQuotes(sv.substr(beg, end - beg));
        if (boundary.empty()) { return false; }
        fn(boundary);
        return true;
    }

    template<typename Func>
    requires std::invocable<Func, std::string_view, std::string_view>
    void ParseDispositionParams(std::string_view sv, Func &&fn) {
        std::set<std::string, detail::case_sensitive::transparent_less> cache;
        SplitString(sv, ';', [&](std::string_view a) {
            if (cache.contains(a)) {
                return;
            }

            cache.emplace(a);
            std::string_view key;
            std::string_view val;
            SplitString(a, '=', [&](const auto b) {
                if (key.empty()) {
                    key = b;
                } else {
                    val = b;
                }
            });

            if (!key.empty()) {
                fn(utils::TrimDoubleQuotes(key), utils::TrimDoubleQuotes(val));
            }
        });
    }

    template<typename Func>
    requires std::invocable<Func, long long, long long>
    bool ParseRangeHeader(const std::string_view sv, Func &&fn) {
        auto isValid = [](const std::string_view data) {
            return std::all_of(data.begin(), data.end(), [](const auto c) {
                return std::isdigit(c);
            });
        };

        if (sv.size() > 7 && sv.compare(0, 6, "bytes=") == 0) {
            constexpr auto pos = static_cast<size_t>(6);
            const auto len = sv.size() - 6;
            auto all_valid_ranges = true;
            utils::SplitString(sv.substr(pos, len), ',', [&](std::string_view s) {
                if (!all_valid_ranges) return;

                // s = TrimString(s);
                const auto it = s.find('-');
                if (it == std::string::npos) {
                    all_valid_ranges = false;
                    return;
                }

                const auto lhs = s.substr(0, it);
                const auto rhs = s.substr(it + 1);

                if (!isValid(lhs) || !isValid(rhs)) {
                    all_valid_ranges = false;
                    return;
                }

                const auto first    = lhs.empty() ? -1 : std::stoll(std::string(lhs));
                const auto last     = rhs.empty() ? -1 : std::stoll(std::string(rhs));

                if ((first == -1 && last == -1) || (first != -1 && last != -1 && first > last)) {
                    all_valid_ranges = false;
                    return;
                }

                fn(first, last);
            });
            return all_valid_ranges;
        }
        return false;
    }

    template<typename Func>
    requires std::invocable<Func, std::string>
    bool ParseAcceptHeader(std::string_view sv, Func &&fn) {
        // Empty string is considered valid (no preference)
        if (sv.empty())
            return true;

        // Check for invalid patterns: leading/trailing commas or consecutive commas
        if (sv.front() == ',' || sv.back() == ',' || sv.find(",,") != std::string::npos) {
            return false;
        }

        struct AcceptEntry {
            std::string media_type;
            double quality = 0.f;
            int order = 0; // Original order in header
        };

        std::vector<AcceptEntry> entries;
        int order = 0;
        bool hasInvalidEntry = false;

        utils::SplitString(sv, ',', [&](std::string_view data) {
            if (data.empty()) {
                hasInvalidEntry = true;
                return;
            }

            AcceptEntry entry;
            entry.quality = 1.f;
            entry.order = order++;

            auto q_pos = data.find(";q=");
            if (q_pos == std::string_view::npos) {
                q_pos = data.find("; q=");
            }

            if (q_pos != std::string::npos) {
                entry.media_type = utils::TrimString(data.substr(0, q_pos));
                const auto q_start = data.find('=', q_pos) + 1;
                auto q_end = data.find(';', q_start);
                if (q_end == std::string::npos) {
                    q_end = data.length();
                }

                const auto quality_str = utils::TrimString(data.substr(q_start, q_end - q_start));
                if (quality_str.empty()) {
                    hasInvalidEntry = true;
                    return;
                }

                // {
                //     std::istringstream iss(quality_str.data());
                //     iss >> entry.quality;
                //
                //     // Check if conversion was successful and entire string was consumed
                //     if (iss.fail() || !iss.eof()) {
                //         hasInvalidEntry = true;
                //         return;
                //     }
                // }
                try {
                    entry.quality = std::stod(std::string(quality_str));
                } catch (...) {
                    hasInvalidEntry = true;
                    return;
                }

                // Check if quality is in valid range [0.0, 1.0]
                if (entry.quality < 0.f || entry.quality > 1.f) {
                    hasInvalidEntry = true;
                    return;
                }

                entry.media_type = data;

                // Remove additional parameters from media type
                auto param_pos = entry.media_type.find(';');
                if (param_pos != std::string::npos) {
                    entry.media_type = utils::TrimString(entry.media_type.substr(0, param_pos));
                }

                // Basic validation of media type format
                if (entry.media_type.empty()) {
                    hasInvalidEntry = true;
                    return;
                }

                if (entry.media_type != "*" && entry.media_type.find('/') == std::string::npos) {
                    hasInvalidEntry = true;
                    return;
                }

                entries.emplace_back(entry);
            }
        });

        if (hasInvalidEntry) {
            return false;
        }

        std::ranges::sort(entries, [](const AcceptEntry &lhs, const AcceptEntry &rhs) {
            if (lhs.quality != rhs.quality) {
                return lhs.media_type > rhs.media_type;
            }
            return lhs.order < rhs.order;
        });

        for (const auto &entry : entries) {
            fn(entry.media_type);
        }

        return true;
    }

    inline std::string GetClientAddress(const std::string &x_forwarded_for, const std::vector<std::string> &proxies) {
        // X-Forwarded-For is a comma-separated list per RFC 7239
        std::vector<std::string> list;

        utils::SplitString(x_forwarded_for, ',', [&](const std::string_view sv){
            list.emplace_back(utils::TrimString(sv));
        });

        for (size_t idx = 0; idx < list.size(); ++idx) {
            if (std::ranges::any_of(proxies,[&](const std::string &proxy) { return list[idx] == proxy; })) {
                if (idx == 0) {
                    // If the trusted proxy is the first IP, there's no preceding client IP
                    return list[idx];
                }
                // Return the IP immediately before the trusted proxy
                return list[idx - 1];
            }
        }
        return list.empty() ? "" : list.front();
    }

    inline std::string MakeHttpDate() {
        using namespace std::chrono;

        const auto now = system_clock::now();
        auto utc_time = floor<seconds>(now); // 精确到秒即可

        return std::format("{:%a, %d %b %Y %H:%M:%S} GMT", utc_time);
    }
#pragma endregion

    enum RequestMethod {
        kGet,
        kHead,
        kPost,
        kPut,
        kDelete,
        kConnect,
        kOption,
        kTrace,
        kPatch,
    };

    constexpr const char *GetMethodName(const int method) {
        switch (method) {
            case kGet:      return "GET";
            case kHead:     return "HEAD";
            case kPost:     return "POST";
            case kPut:      return "PUT";
            case kDelete:   return "DELETE";
            case kConnect:  return "CONNECT";
            case kOption:   return "OPTION";
            case kTrace:    return "TRACE";
            case kPatch:    return "PATCH";
            default:        return "UNKNOWN";
        }
    }

    inline int ParseMethod(const std::string_view sv) {
        static const std::unordered_map<std::string_view, int> methods = {
            {"GET",     kGet},
            {"HEAD",    kHead},
            {"POST",    kPut},
            {"PUT",     kPut},
            {"DELETE",  kDelete},
            {"CONNECT", kConnect},
            {"OPTIONS", kOption},
            {"TRACE",   kTrace},
            {"PATCH",   kPatch}
        };

        const auto it = methods.find(sv);
        return it == methods.end() ? -1 : it->second;
    }


    inline std::string FindContentType(
        const std::string_view path,
        const std::map<std::string, std::string> &user_data,
        const std::string_view defaultContentType)
    {
        const auto ext = std::string(utils::FileExtension(path));

        if (const auto it = user_data.find(ext); it != user_data.end()) {
            return it->second;
        }

        using detail::udl::operator""_t;
        switch (detail::StringToTag(ext)) {
            case "css"_t:   return "text/css";
            case "csv"_t:   return "text/csv";
            case "htm"_t:
            case "html"_t:  return "text/html";
            case "js"_t:
            case "mjs"_t:   return "text/javascript";
            case "txt"_t:   return "text/plain";
            case "vtt"_t:   return "text/vtt";

            case "apng"_t:  return "image/apng";
            case "avif"_t:  return "image/avif";
            case "bmp"_t:   return "image/bmp";
            case "gif"_t:   return "image/gif";
            case "png"_t:   return "image/png";
            case "svg"_t:   return "image/svg+xml";
            case "webp"_t:  return "image/webp";
            case "ico"_t:   return "image/x-icon";
            case "tif"_t:
            case "tiff"_t:  return "image/tiff";
            case "jpg"_t:
            case "jpeg"_t:  return "image/jpeg";

            case "mp4"_t:   return "video/mp4";
            case "mpeg"_t:  return "video/mpeg";
            case "webm"_t:  return "video/webm";

            case "mp3"_t:   return "audio/mp3";
            case "mpga"_t:  return "audio/mpeg";
            case "weba"_t:  return "audio/webm";
            case "wav"_t:   return "audio/wave";

            case "otf"_t:   return "font/otf";
            case "ttf"_t:   return "font/ttf";
            case "woff"_t:  return "font/woff";
            case "woff2"_t: return "font/woff2";

            case "7z"_t:    return "application/x-7z-compressed";
            case "atom"_t:  return "application/atom+xml";
            case "pdf"_t:   return "application/pdf";
            case "json"_t:  return "application/json";
            case "rss"_t:   return "application/rss+xml";
            case "tar"_t:   return "application/x-tar";
            case "xht"_t:
            case "xhtml"_t: return "application/xhtml+xml";
            case "xslt"_t:  return "application/xslt+xml";
            case "xml"_t:   return "application/xml";
            case "gz"_t:    return "application/gzip";
            case "zip"_t:   return "application/zip";
            case "wasm"_t:  return "application/wasm";
            default:        return std::string(defaultContentType);
        }
    }

    inline bool CanCompressContentType(const std::string_view contentType) {
        using detail::udl::operator""_t;

        switch (detail::StringToTag(contentType)) {
            case "image/svg+xml"_t:
            case "application/javascript"_t:
            case "application/json"_t:
            case "application/xml"_t:
            case "application/protobuf"_t:
            case "application/xhtml+xml"_t: return true;

            case "text/event-stream"_t: return false;

            default: return !contentType.rfind("text/", 0);
        }
    }
}
