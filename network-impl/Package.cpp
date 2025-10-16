#include "Package.h"

namespace uranus::network {
    mi_heap_t *Package::BufferHeap::heap_ = mi_heap_new();

    void *Package::BufferHeap::Allocate(std::size_t size) {
        return mi_heap_malloc(heap_, size);
    }

    void Package::BufferHeap::Deallocate(void *ptr) {
        if (ptr) {
            mi_free(ptr);
        }
    }

    Package::Package(Recycler<Package>::Handle handle)
        : handle_(handle) {
    }

    Package::~Package() {
    }

    void Package::Recycle() {
        handle_.Recycle(this);
    }

    void Package::SetPackageID(const int32_t id) {
        id_ = id;
    }

    int32_t Package::GetPackageID() const {
        return id_;
    }

    void Package::SetData(const std::span<const uint8_t> data) {
        length_ = data.size();
        payload_.resize(data.size());
        if (!data.empty()) {
            std::memcpy(payload_.data(), data.data(), data.size());
        }
    }

    void Package::SetData(const uint8_t *data, std::size_t len) {
        SetData(std::span(data, len));
    }

    void Package::SetData(const std::vector<uint8_t> &data) {
        SetData(std::span(data));
    }

    void Package::SetData(const std::string &str) {
        length_ = str.size();
        payload_.assign(str.begin(), str.end());
    }

    void Package::SetData(std::string_view sv) {
        length_ = sv.size();
        payload_.assign(sv.begin(), sv.end());
    }

    std::string Package::ToString() const {
        return {payload_.begin(), payload_.end()};
    }
}
