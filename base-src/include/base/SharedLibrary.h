#pragma once

#include "base.export.h"

#include <string>
#include <atomic>
#include <filesystem>

#if defined(_WIN32) || defined(_WIN64)
#include <windows.h>
using ModuleHandle = HMODULE;
#else
#include <dlfcn.h>
using ModuleHandle = void *;
#endif

namespace uranus {

    using std::atomic;
    using std::string;

    /**
     * 动态库句柄封装
     */
    class BASE_API SharedLibrary final {

        struct SharedControl {
            ModuleHandle handle = nullptr;
            atomic<size_t> refCount = 0;
            std::filesystem::path path_;
        };

    public:
        SharedLibrary();
        ~SharedLibrary();

        explicit SharedLibrary(const std::filesystem::path &path);

        explicit SharedLibrary(const std::string &str);
        explicit SharedLibrary(std::string_view sv);

        SharedLibrary(const SharedLibrary &rhs);
        SharedLibrary &operator=(const SharedLibrary &rhs);

        SharedLibrary(SharedLibrary &&rhs) noexcept;
        SharedLibrary &operator=(SharedLibrary &&rhs) noexcept;

        template<typename Func>
        Func getSymbol(std::string_view sv) const;

        /// 获取当前引用计数
        [[nodiscard]] size_t refCount() const;

        [[nodiscard]] std::filesystem::path path() const;

        void swap(SharedLibrary &rhs) noexcept;
        void reset();

        bool tryRelease();

        [[nodiscard]] bool available() const;
        explicit operator bool() const;

        bool operator==(const SharedLibrary &rhs) const;

    private:
        void release();

    private:
        /** 引用计数控制块 **/
        SharedControl *ctrl_;

        /** 内部句柄指针 **/
        ModuleHandle handle_;
    };

    template<typename Func>
    Func SharedLibrary::getSymbol(std::string_view sv) const {
        if (!available())
            return nullptr;

#if defined(_WIN32) || defined(_WIN64)
        return reinterpret_cast<Func>(GetProcAddress(handle_, sv.data()));
#else
        return reinterpret_cast<Func>(dlsym(handle_, sv.data()));
#endif
    }
}
