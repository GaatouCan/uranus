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

    class BASE_API SharedLibrary final {

        struct SharedControl {
            ModuleHandle handle = nullptr;
            atomic<size_t> refCount = 0;
        };

    public:
        SharedLibrary();
        ~SharedLibrary();

        explicit SharedLibrary(std::string_view sv);
        explicit SharedLibrary(const std::filesystem::path &path);

        SharedLibrary(const SharedLibrary &rhs);
        SharedLibrary &operator=(const SharedLibrary &rhs);

        SharedLibrary(SharedLibrary &&rhs) noexcept;
        SharedLibrary &operator=(SharedLibrary &&rhs) noexcept;

        template<typename Func>
        Func getSymbol(std::string_view sv);

        [[nodiscard]] size_t refCount() const;

        void swap(SharedLibrary &rhs) noexcept;
        void reset();

        [[nodiscard]] bool available() const;
        explicit operator bool() const;

        bool operator==(const SharedLibrary &rhs) const;

    private:
        void release();

    private:
        SharedControl *ctrl_;
        ModuleHandle handle_;
    };

    template<typename Func>
    Func SharedLibrary::getSymbol(std::string_view sv) {
        if (!available())
            return nullptr;

#if defined(_WIN32) || defined(_WIN64)
        return reinterpret_cast<Func>(GetProcAddress(handle_, sv.data()));
#else
        return reinterpret_cast<Func>(dlsym(handle_, sv.data()));
#endif
    }
}
