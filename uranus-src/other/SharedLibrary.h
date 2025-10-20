#pragma once

#include <string>
#include <atomic>
#include <filesystem>
#if defined(_WIN32) || defined(_WIN64)
#define WIN32_LEAN_AND_MEAN
#include <windows.h>
using ModuleHandle = HMODULE;
#else
#include <dlfcn.h>
using ModuleHandle = void *;
#endif


using std::atomic;
using std::string;


class SharedLibrary final {

    struct ControlBlock {
        ModuleHandle        handle;
        atomic<size_t>      ref_count;
    };

public:
    SharedLibrary();
    ~SharedLibrary();

    explicit SharedLibrary(const string &path);
    explicit SharedLibrary(const std::filesystem::path &path);

    SharedLibrary(const SharedLibrary &rhs);
    SharedLibrary &operator=(const SharedLibrary &rhs);

    SharedLibrary(SharedLibrary &&rhs) noexcept;
    SharedLibrary &operator=(SharedLibrary &&rhs) noexcept;

    template<typename Func>
    Func GetSymbol(const string &name) const;

    [[nodiscard]] size_t    GetUseCount()   const noexcept;
    [[nodiscard]] bool      IsValid()       const noexcept;

    explicit operator bool() const noexcept;

    void Swap(SharedLibrary &rhs);
    void Reset();

    bool operator==(const SharedLibrary &rhs) const;

private:
    void Release();

private:
    ControlBlock *control_;
    ModuleHandle handle_;
};


template<typename Type>
Type SharedLibrary::GetSymbol(const string &name) const {
    if (!IsValid())
        return nullptr;

#if defined(_WIN32) || defined(_WIN64)
    return reinterpret_cast<Type>(GetProcAddress(handle_, name.c_str()));
#else
    return reinterpret_cast<Type>(dyslm(control_->mHandle, name.c_str()));
#endif
}
