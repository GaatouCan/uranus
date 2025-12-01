#include "Message.h"

#include <mimalloc.h>

namespace uranus {
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


    Message::Message(const MessageHandle &handle)
        : handle_(handle),
          id_(0) {
    }

    void Message::recycle() {
        id_ = 0;
        handle_.recycle(this);
    }

    uint64_t Message::getId() const {
        return id_;
    }

    Message::~Message() {
    }

    IMPLEMENT_RECYCLER(Message)

    IMPLEMENT_RECYCLER_GET(Message);

    Envelope::Envelope(const uint32_t src, Message::MessageUnique &&msg)
        : source(src),
          session(0),
          message(std::move(msg)) {
    }

    Envelope::Envelope(const uint32_t src, const uint32_t sess, Message::MessageUnique &&msg)
        : source(src),
          session(sess),
          message(std::move(msg)) {
    }
}
