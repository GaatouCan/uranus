#include "SharedLibrary.h"

namespace uranus {
    SharedLibrary::SharedLibrary()
        : ctrl_(nullptr),
          handle_(nullptr) {
    }

    SharedLibrary::~SharedLibrary() {
        release();
    }

    SharedLibrary::SharedLibrary(const std::string_view sv)
        : SharedLibrary(std::filesystem::path(sv)) {

    }

    SharedLibrary::SharedLibrary(const std::filesystem::path &path) {
        ctrl_ = new SharedControl();

#if defined(_WIN32) || defined(_WIN64)
        ctrl_->handle = LoadLibrary(path.string().c_str());
#else
        ctrl_->handle = dlopen(path.string().c_str(), RTLD_LAZY);
#endif

        if (!ctrl_->handle) {
            throw std::runtime_error("dlopen failed");
        }

        ctrl_->refCount.store(1, std::memory_order_relaxed);
        ctrl_->path_ = path;

        handle_ = ctrl_->handle;
    }

    SharedLibrary::SharedLibrary(const std::string &str)
        : SharedLibrary(std::filesystem::path(str)) {
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

    std::filesystem::path SharedLibrary::path() const {
        if (ctrl_ != nullptr) {
            return ctrl_->path_;
        }
        return {};
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

    bool SharedLibrary::tryRelease() {
        if (!ctrl_)
            return true;

        if (size_t expected = 1; ctrl_->refCount.compare_exchange_strong(expected, 0, std::memory_order_acq_rel)) {
#if defined(_WIN32) || defined(_WIN64)
            FreeLibrary(ctrl_->handle);
#else
            dlclose(ctrl_->handle);
#endif

            delete ctrl_;

            ctrl_ = nullptr;
            handle_ = nullptr;

            return true;
        }

        return false;
    }

    SharedLibrary::operator bool() const {
        return available();
    }

    bool SharedLibrary::operator==(const SharedLibrary &rhs) const {
        return ctrl_ == rhs.ctrl_;
    }

    void SharedLibrary::release() {
        if (ctrl_ && ctrl_->refCount.fetch_sub(1, std::memory_order_acq_rel) == 1) {
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
