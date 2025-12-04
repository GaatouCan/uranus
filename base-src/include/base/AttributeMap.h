#pragma once

#include "base.export.h"

#include <any>
#include <optional>
#include <unordered_map>
#include <string>

namespace uranus {
    class BASE_API AttributeMap final {

    public:
        AttributeMap();
        ~AttributeMap();

        [[nodiscard]] bool has(const std::string& key) const;

        template<class T>
        void set(const std::string& key, const T& value) {
            attr_[key] = std::make_any<T>(value);
        }

        template<class T>
        void set(const std::string& key, T&& value) {
            attr_[key] = std::make_any<T>(std::forward<T>(value));
        }

        template<class T>
        std::optional<T> get(const std::string& key) const {
            const auto it = attr_.find(key);
            if (it == attr_.end()) {
                return std::nullopt;
            }
            return std::any_cast<T>(it->second);
        }

        void clear();
        void erase(const std::string& key);

    private:
        std::unordered_map<std::string, std::any> attr_;
    };
}