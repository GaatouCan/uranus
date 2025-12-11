#include "FormData.h"

namespace uranus::http {
    std::string MultipartFormData::getField(const std::string_view key, const size_t id) const {
        const auto [beg, end] = fields.equal_range(key);
        auto it = beg;
        std::advance(it, id);
        if (it != end) {
            return it->second.content;
        }
        return {};
    }

    std::vector<std::string> MultipartFormData::getFields(const std::string_view key) const {
        std::vector<std::string> result;
        auto [beg, end] = fields.equal_range(key);
        for (auto it = beg; it != end; ++it) {
            result.push_back(it->second.content);
        }
        return result;
    }

    bool MultipartFormData::hasField(const std::string_view key) const {
        return fields.contains(key);
    }

    size_t MultipartFormData::getFieldCount(const std::string_view key) const {
        const auto [beg, end] = fields.equal_range(key);
        return std::distance(beg, end);
    }

    FormData MultipartFormData::getFile(const std::string_view key, const size_t id) const {
        const auto [beg, end] = files.equal_range(key);
        auto it = beg;
        std::advance(it, id);
        if (it != end) {
            return it->second;
        }
        return {};
    }

    std::vector<FormData> MultipartFormData::getFiles(const std::string_view key) const {
        std::vector<FormData> result;
        auto [beg, end] = files.equal_range(key);
        for (auto it = beg; it != end; ++it) {
            result.push_back(it->second);
        }
        return result;
    }

    bool MultipartFormData::hasFile(const std::string_view key) const {
        return files.contains(key);
    }

    size_t MultipartFormData::getFileCount(const std::string_view key) const {
        const auto [beg, end] = files.equal_range(key);
        return std::distance(beg, end);
    }
}
