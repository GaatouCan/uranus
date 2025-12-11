#include "BufferHeap.h"

#include <mimalloc.h>

namespace uranus::http {
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