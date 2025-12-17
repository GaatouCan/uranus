#pragma once

#include <unordered_set>
#include <atomic>
#include <mutex>


template<class Type, bool bConcurrent>
requires std::is_integral_v<Type>
class IdentAllocator {

    struct EmptyMutex {};

    using AllocatorMutex   = std::conditional_t<bConcurrent, std::mutex, EmptyMutex>;
    using IntegralType     = std::conditional_t<bConcurrent, std::atomic<Type>, Type>;

public:
    //Type allocateTS();
    Type allocate();

    //void recycleTS(Type id);
    void recycle(Type id);

    Type usage() const;

private:
    std::unordered_set<Type>    set_;
    AllocatorMutex              mtx_;
    IntegralType                next_;
    IntegralType                usage_;
};

template<class Type, bool bConcurrent>
requires std::is_integral_v<Type>
Type IdentAllocator<Type, bConcurrent>::allocate() {
    if constexpr (bConcurrent) {
        std::unique_lock lock(mtx_);
        if (const auto iter = set_.begin(); iter != set_.end()) {
            const auto res = *iter;
            set_.erase(iter);

            ++usage_;
            return res;
        }
    } else {
        if (const auto iter = set_.begin(); iter != set_.end()) {
            const auto res = *iter;
            set_.erase(iter);

            ++usage_;
            return res;
        }
    }

    ++usage_;
    return ++next_;
}

// template<class Type, bool bConcurrent>
// requires std::is_integral_v<Type>
// Type IdentAllocator<Type, bConcurrent>::allocate() {
//     if (const auto iter = set_.begin(); iter != set_.end()) {
//         const auto res = *iter;
//         set_.erase(iter);
//
//         ++usage_;
//         return res;
//     }
//
//     ++usage_;
//     return ++next_;
// }

template<class Type, bool bConcurrent>
requires std::is_integral_v<Type>
void IdentAllocator<Type, bConcurrent>::recycle(Type id) {
    if constexpr (bConcurrent) {
        std::unique_lock lock(mtx_);
        set_.emplace(id);
    } else {
        set_.emplace(id);
    }

    --usage_;
    if constexpr (bConcurrent) {
        usage_ = usage_.load() > 0 ? usage_.load() : 0;
    } else {
        usage_ = usage_ > 0 ? usage_ : 0;
    }
}

// template<class Type, bool bConcurrent>
// requires std::is_integral_v<Type>
// inline void IdentAllocator<Type, bConcurrent>::recycle(Type id) {
//     set_.emplace(id);
//
//     --usage_;
//     if constexpr (bConcurrent) {
//         usage_ = usage_.load() > 0 ? usage_.load() : 0;
//     } else {
//         usage_ = usage_ > 0 ? usage_ : 0;
//     }
// }

template<class Type, bool bConcurrent>
requires std::is_integral_v<Type>
Type IdentAllocator<Type, bConcurrent>::usage() const {
    if constexpr (bConcurrent) {
        return usage_.load();
    }
    return usage_;
}
