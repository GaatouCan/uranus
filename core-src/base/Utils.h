#pragma once

#include "Common.h"
#include "Types.h"

#include <filesystem>
#include <functional>
#include <thread>
#include <map>


#if defined(_WIN32) || defined(_WIN64)
#include <Windows.h>
using AModuleHandle = HMODULE;
#else
#include <dlfcn.h>
using AModuleHandle = void *;
#endif


using AThreadID = std::thread::id;

const auto NowTimePoint = std::chrono::system_clock::now;

namespace utils {
    CORE_API void TraverseFolder(const std::string &folder, const std::function<void(const std::filesystem::directory_entry &)> &func);

    CORE_API std::string StringReplace(std::string source, char toReplace, char replacement);

    CORE_API int64_t ThreadIDToInt(AThreadID tid);

    CORE_API std::string PascalToUnderline(const std::string &src);

    CORE_API std::vector<uint8_t> HexToBytes(const std::string &hex);

    CORE_API long long UnixTime();
    CORE_API long long ToUnixTime(SystemTimePoint point);

    CORE_API int64_t SetBit(int64_t, int32_t);
    CORE_API int64_t ClearBit(int64_t, int32_t);
    CORE_API int64_t ToggleBit(int64_t, int32_t);
    CORE_API bool CheckBit(int64_t, int32_t);

    CORE_API std::vector<std::string> SplitString(const std::string &src, char delimiter);
    CORE_API std::vector<int> SplitStringToInt(const std::string &src, char delimiter);

    /**
     * Get Day Of The Week
     * @param point Time Point, Default Now
     * @return From 0 To 6, Means Sunday(0) To StaterDay(6)
     */
    CORE_API unsigned GetDayOfWeek(SystemTimePoint point = NowTimePoint());
    CORE_API unsigned GetDayOfMonth(SystemTimePoint point = NowTimePoint());
    CORE_API int GetDayOfYear(SystemTimePoint point = NowTimePoint());

    /**
     * 往日不再
     * @param former 较前的时间点
     * @param latter 较后的时间点 默认当前时间点
     * @return 经过的天数 同一天为0
     */
    CORE_API int GetDaysGone(SystemTimePoint former, SystemTimePoint latter = NowTimePoint());
    CORE_API SystemTimePoint GetDayZeroTime(SystemTimePoint point = NowTimePoint());

    CORE_API bool IsSameWeek(SystemTimePoint former, SystemTimePoint latter = NowTimePoint());
    CORE_API bool IsSameMonth(SystemTimePoint former, SystemTimePoint latter = NowTimePoint());

    CORE_API int RandomDraw(const std::map<int, int> &pool);

    // template<class T>
    // void CleanUpWeakPointerSet(std::unordered_set<std::weak_ptr<T>, FWeakPointerHash<T>, FWeakPointerEqual<T>> &set) {
    //     for (auto it = set.begin(); it != set.end();) {
    //         if (it->expired()) {
    //             it = set.erase(it);
    //         } else {
    //             ++it;
    //         }
    //     }
    // }
}

template<typename T>
struct WeakPointerHash {
    size_t operator()(const std::weak_ptr<T> &wPtr) const {
        auto ptr = wPtr.lock();
        return std::hash<T *>()(ptr.get());
    }
};

template<typename T>
struct WeakPointerEqual {
    bool operator()(const std::weak_ptr<T> &lhs, const std::weak_ptr<T> &rhs) const {
        auto lhsPtr = lhs.lock();
        auto rhsPtr = rhs.lock();
        return lhsPtr == rhsPtr;
    }
};