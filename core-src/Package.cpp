#include "Package.h"

mi_heap_t *Package::BufferHeap::kHeap = mi_heap_new();

void *Package::BufferHeap::allocate(std::size_t size) {
    return mi_heap_malloc(kHeap, size ? size : 1);
}

void Package::BufferHeap::deallocate(void *ptr) {
    if (ptr) {
        mi_free(ptr);
    }
}

Package::Package(Recycler<Package>::Handle handle)
    : handle_(std::move(handle)),
      header_(),
      destroyer_(nullptr) {
    memset(&header_, 0, sizeof(header_));
}

Package::~Package() {
}

Package::Package(Package &&rhs) noexcept
    : handle_(std::move(rhs.handle_)),
      header_(),
      destroyer_(nullptr) {
    memcpy(&header_, &rhs.header_, sizeof(header_));
    memset(&rhs.header_, 0, sizeof(rhs.header_));

    payload_ = std::move(rhs.payload_);
    destroyer_ = rhs.destroyer_;
}

Package &Package::operator=(Package &&rhs) noexcept {
    if (this != &rhs) {
        handle_ = std::move(rhs.handle_);

        memcpy(&header_, &rhs.header_, sizeof(header_));
        memset(&rhs.header_, 0, sizeof(rhs.header_));

        payload_ = std::move(rhs.payload_);
        destroyer_ = rhs.destroyer_;
    }
    return *this;
}

void Package::SetData(std::span<const uint8_t> data) {
    header_.length = data.size();
    payload_.resize(header_.length);
    if (!data.empty()) {
        std::memcpy(payload_.data(), data.data(), data.size());
    }
}

void Package::SetData(const uint8_t *data, const std::size_t length) {
    SetData(std::span(data, length));
}

void Package::SetData(const std::vector<uint8_t> &data) {
    SetData(std::span(data));
}

void Package::SetData(const std::string &str) {
    SetData(reinterpret_cast<const uint8_t *>(str.data()), str.size());
}

void Package::SetData(std::string_view sv) {
    SetData(reinterpret_cast<const uint8_t *>(sv.data()), sv.size());
}

void Package::Recycle() {
    if (destroyer_ != nullptr) {
        std::invoke(destroyer_, payload_.data());
    }
    handle_.Recycle(this);
}
