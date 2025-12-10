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

    Package::~Package() {
    }

    void Package::setId(int32_t id) {
        id_ = id;
    }

    int64_t Package::getId() const {
        return id_;
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
          message(nullptr) {
    }

    Envelope::Envelope(const int32_t ty, const uint32_t src, MessageHandle &&msg)
        : type(ty),
          source(src),
          session(0),
          message(std::move(msg)) {
    }

    Envelope::Envelope(const int32_t ty, const uint32_t src, const uint32_t sess, MessageHandle &&msg)
        : type(ty),
          source(src),
          session(sess),
          message(std::move(msg)) {
    }

    Envelope::Envelope(Envelope &&rhs) noexcept {
        if (this != &rhs) {
            type = rhs.type;
            source = rhs.source;
            session = rhs.session;

            rhs.type = 0;
            rhs.source = 0;
            rhs.session = 0;

            message = std::move(rhs.message);
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

            message = std::move(rhs.message);
        }
        return *this;
    }
}
