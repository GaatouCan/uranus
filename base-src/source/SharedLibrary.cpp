#include "SharedLibrary.h"

namespace uranus {
    SharedLibrary::SharedLibrary()
        : ctrl_(nullptr),
          handle_(nullptr) {
    }

    SharedLibrary::~SharedLibrary() {
        release();
    }

    SharedLibrary::SharedLibrary(const std::string_view sv) {
        ctrl_ = new SharedControl();
        ctrl_->refCount = 1;

#if defined(_WIN32) || defined(_WIN64)
        ctrl_->handle = LoadLibrary(sv.data());
#else
        ctrl_->handle = dlopen(sv.data(), RTLD_LAZY);
#endif

        if (!ctrl_->handle) {
            throw std::runtime_error("dlopen failed");
        }

        handle_ = ctrl_->handle;
    }

    SharedLibrary::SharedLibrary(const std::filesystem::path &path)
        : SharedLibrary(std::string_view(path.string())) {
    }

    SharedLibrary::SharedLibrary(const SharedLibrary &rhs) {
        ctrl_ = rhs.ctrl_;
        handle_ = rhs.handle_;

        if (ctrl_) {
            ctrl_->refCount.fetch_add(1, std::memory_order_relaxed);
        }
    }

    SharedLibrary &SharedLibrary::operator=(const SharedLibrary &rhs) {
        if (this != &rhs) {
            release();

            ctrl_ = rhs.ctrl_;
            handle_ = rhs.handle_;

            if (ctrl_) {
                ctrl_->refCount.fetch_add(1, std::memory_order_relaxed);
            }
        }

        return *this;
    }

    SharedLibrary::SharedLibrary(SharedLibrary &&rhs) noexcept {
        ctrl_ = rhs.ctrl_;
        handle_ = rhs.handle_;

        rhs.ctrl_ = nullptr;
        rhs.handle_ = nullptr;
    }

    SharedLibrary &SharedLibrary::operator=(SharedLibrary &&rhs) noexcept {
        if (this != &rhs) {
            release();

            ctrl_ = rhs.ctrl_;
            handle_ = rhs.handle_;

            rhs.ctrl_ = nullptr;
            rhs.handle_ = nullptr;
        }

        return *this;
    }

    size_t SharedLibrary::refCount() const {
        return ctrl_ ? ctrl_->refCount.load() : 0;
    }

    bool SharedLibrary::available() const {
        return ctrl_ != nullptr && handle_ != nullptr;
    }

    void SharedLibrary::swap(SharedLibrary &rhs) noexcept {
        std::swap(ctrl_, rhs.ctrl_);
        std::swap(handle_, rhs.handle_);
    }

    void SharedLibrary::reset() {
        SharedLibrary().swap(*this);
    }

    SharedLibrary::operator bool() const {
        return available();
    }

    bool SharedLibrary::operator==(const SharedLibrary &rhs) const {
        return ctrl_ == rhs.ctrl_;
    }

    void SharedLibrary::release() {
        if (ctrl_ && ctrl_->refCount.fetch_sub(1, std::memory_order_acq_rel) == 0) {
#if defined(_WIN32) || defined(_WIN64)
            FreeLibrary(ctrl_->handle);
#else
            dlclose(ctrl_->handle);
#endif
            delete ctrl_;
        }

        ctrl_ = nullptr;
        handle_ = nullptr;
    }
}
