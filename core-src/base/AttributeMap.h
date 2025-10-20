#pragma once

#include "Common.h"

#include <any>
#include <string>
#include <optional>
#include <unordered_map>


namespace uranus {
    class CORE_API AttributeMap {

    public:
        AttributeMap() = default;
        virtual ~AttributeMap() = default;

        template<class T>
        void SetAttr(const std::string& key, const T& value) {
            attr_[key] = std::make_any<T>(value);
        }

        template<class T>
        void SetAttr(const std::string& key, T&& value) {
            attr_[key] = std::make_any<T>(std::forward<T>(value));
        }

        template<class T>
        std::optional<T> GetAttr(const std::string& key) const {
            const auto it = attr_.find(key);
            if (it == attr_.end()) {
                return std::nullopt;
            }
            return std::any_cast<T>(it->second);
        }

        [[nodiscard]] bool HasAttr(const std::string& key) const {
            return attr_.contains(key);
        }

        void ClearAttr() {
            attr_.clear();
        }

        void ResetAttr(const std::string& key) {
            attr_.erase(key);
        }

    private:
        std::unordered_map<std::string, std::any> attr_;
    };
}