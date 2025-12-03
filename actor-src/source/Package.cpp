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
          id_(0) {
    }

    Package::~Package() {
    }

    void Package::recycle() {
        id_ = 0;
        payload_.clear();
        handle_.recycle(this);
    }

    IMPLEMENT_RECYCLER_GET(Package)

    IMPLEMENT_RECYCLER(Package)
}
