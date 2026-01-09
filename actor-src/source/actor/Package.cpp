#include "Package.h"

#include <mimalloc.h>

namespace uranus::actor {

    namespace detail {
        void *BufferHeap::allocate(const std::size_t size) {
            thread_local mi_heap_t *heap = mi_heap_new();
            return mi_heap_malloc(heap, size);
        }

        void BufferHeap::deallocate(void *ptr) {
            if (ptr) {
                mi_free(ptr);
            }
        }
    }

    Package::Package(const PackageRecyclerHandle &handle)
        : handle_(handle),
          id_(-1) {
    }

    Package::~Package() = default;

    void Package::setId(const int64_t id) {
        id_ = id;
    }

    int64_t Package::getId() const {
        return id_;
    }

    void Package::setData(const std::string &data) {
        payload_.clear();
        payload_.resize(data.size());
        std::memcpy(payload_.data(), data.data(), data.size());
    }

    void Package::setData(const std::vector<uint8_t> &bytes) {
        payload_.clear();
        payload_.resize(bytes.size());
        std::memcpy(payload_.data(), bytes.data(), bytes.size());
    }

    void Package::setData(const uint8_t *data, const size_t length) {
        payload_.clear();
        payload_.resize(length);
        std::memcpy(payload_.data(), data, length);
    }

    std::string Package::toString() const {
        return { payload_.begin(), payload_.end() };
    }

    void Package::recycle() {
        id_ = -1;
        payload_.clear();
        handle_.recycle(this);
    }

    IMPLEMENT_RECYCLER_GET(Package)

    IMPLEMENT_RECYCLER(Package)

    Envelope::Envelope()
        : type(0),
          source(0),
          session(0),
          package(nullptr) {
    }

    Envelope::Envelope(const int32_t ty, const int64_t src, PackageHandle &&pkg)
        : type(ty),
          source(src),
          session(0),
          package(std::move(pkg)) {
    }

    Envelope::Envelope(const int32_t ty, const int64_t src, const int64_t sess, PackageHandle &&pkg)
        : type(ty),
          source(src),
          session(sess),
          package(std::move(pkg)) {
    }

    Envelope::Envelope(Envelope &&rhs) noexcept {
        if (this != &rhs) {
            type = rhs.type;
            source = rhs.source;
            session = rhs.session;

            rhs.type = 0;
            rhs.source = 0;
            rhs.session = 0;

            package = std::move(rhs.package);
        }
    }

    Envelope &Envelope::operator=(Envelope &&rhs) noexcept {
        if (this != &rhs) {
            type = rhs.type;
            source = rhs.source;
            session = rhs.session;

            rhs.type = 0;
            rhs.source = 0;
            rhs.session = 0;

            package = std::move(rhs.package);
        }
        return *this;
    }
}
