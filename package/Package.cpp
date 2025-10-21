#include "Package.h"
#include "Message.h"

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
        : handle_(std::move(handle)),
          header_() {
    }

    Package::~Package() = default;

    void Package::Recycle() {
        handle_.Recycle(this);
    }

    void Package::SetPackageID(const int32_t id) {
        header_.id = id;
    }

    int32_t Package::GetPackageID() const {
        return header_.id;
    }

    void Package::SetData(const std::span<const uint8_t> data) {
        header_.length = static_cast<int32_t>(data.size());
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
        header_.length = static_cast<int32_t>(str.size());
        payload_.assign(str.begin(), str.end());
    }

    void Package::SetData(std::string_view sv) {
        header_.length = static_cast<int32_t>(sv.size());
        payload_.assign(sv.begin(), sv.end());
    }

    std::string Package::ToString() const {
        return {payload_.begin(), payload_.end()};
    }

    void Package::ReleaseMessage(const Message *msg) {
        if (msg == nullptr)
            return;

        if (msg->data == nullptr) {
            delete msg;
            return;
        }

        // if (msg->length != sizeof(Package)) {
        //     delete msg;
        //     return;
        // }

        auto *pkg = static_cast<Package *>(msg->data);
        pkg->Recycle();

        delete msg;
    }
}
