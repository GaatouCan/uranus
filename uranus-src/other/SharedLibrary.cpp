#include "SharedLibrary.h"

#include <stdexcept>
#include <spdlog/fmt/fmt.h>


SharedLibrary::SharedLibrary()
    : control_(nullptr),
      handle_(nullptr){
}

SharedLibrary::~SharedLibrary() {
    Release();
}

SharedLibrary::SharedLibrary(const string &path) {
    control_ = new ControlBlock();
    control_->ref_count = 1;

#if defined(_WIN32) || defined(_WIN64)
    control_->handle = LoadLibrary(path.data());
    if (!control_->handle) {
        throw std::runtime_error(fmt::format("{} - Fail To Load Dynamic Library[{}]", __FUNCTION__, path));
    }
#else
    control_->handle = dlopen(path.data(), RTLD_LAZY);
    if (!control_->handle) {
        throw std::runtime_error(fmt::format("{} - Fail To Load Dynamic Library[{}]", __FUNCTION__, path));
    }
#endif

    handle_ = control_->handle;
}

SharedLibrary::SharedLibrary(const std::filesystem::path &path)
    : SharedLibrary(path.string()) {
}

SharedLibrary::SharedLibrary(const SharedLibrary &rhs) {
    control_ = rhs.control_;
    handle_ = rhs.handle_;
    if (control_) {
        control_->ref_count.fetch_add(1, std::memory_order_relaxed);
    }
}

SharedLibrary &SharedLibrary::operator=(const SharedLibrary &rhs) {
    if (this != &rhs) {
        Release();
        control_ = rhs.control_;
        handle_ = rhs.handle_;

        if (control_) {
            ++control_->ref_count;
        }
    }
    return *this;
}

SharedLibrary::SharedLibrary(SharedLibrary &&rhs) noexcept {
    control_ = rhs.control_;
    handle_ = rhs.handle_;
    rhs.control_ = nullptr;
    rhs.handle_ = nullptr;
}

SharedLibrary &SharedLibrary::operator=(SharedLibrary &&rhs) noexcept {
    if (this != &rhs) {
        Release();
        control_ = rhs.control_;
        handle_ = rhs.handle_;
        rhs.control_ = nullptr;
        rhs.handle_ = nullptr;
    }
    return *this;
}

size_t SharedLibrary::GetUseCount() const noexcept {
    return control_ ? control_->ref_count.load() : 0;
}

bool SharedLibrary::IsValid() const noexcept {
    return control_ != nullptr && handle_ != nullptr;
}

SharedLibrary::operator bool() const noexcept {
    return IsValid();
}

void SharedLibrary::Swap(SharedLibrary &rhs) {
    std::swap(control_, rhs.control_);
    std::swap(handle_, rhs.handle_);
}

void SharedLibrary::Reset() {
    SharedLibrary().Swap(*this);
}

bool SharedLibrary::operator==(const SharedLibrary &rhs) const {
    return control_ == rhs.control_;
}


void SharedLibrary::Release() {
    if (control_ && control_->ref_count.fetch_sub(1, std::memory_order_acq_rel) == 0) {
#if defined(_WIN32) || defined(_WIN64)
        FreeLibrary(control_->handle);
#else
        dlclose(control_->handle);
#endif

        delete control_;
    }
    control_ = nullptr;
    handle_ = nullptr;
}
