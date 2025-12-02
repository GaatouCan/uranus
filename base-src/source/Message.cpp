#include "Message.h"

namespace uranus {
    // namespace detail {
    //     void *BufferHeap::allocate(const std::size_t size) {
    //         thread_local mi_heap_t *heap = mi_heap_new();
    //         return mi_heap_malloc(heap, size);
    //     }
    //
    //     void BufferHeap::deallocate(void *ptr) {
    //         if (ptr) {
    //             mi_free(ptr);
    //         }
    //     }
    // }
    Message::Message()
        : id_(0) {
    }

    Message::~Message() {
    }

    void Message::setId(const uint64_t id) {
        id_ = id;
    }

    uint64_t Message::getId() const {
        return id_;
    }

    Envelope::Envelope(const uint32_t src, MessageHandle &&msg)
        : source(src),
          session(0),
          message(std::move(msg)) {
    }

    Envelope::Envelope(const uint32_t src, const uint32_t sess, MessageHandle &&msg)
        : source(src),
          session(sess),
          message(std::move(msg)) {
    }

    Envelope::Envelope(Envelope &&rhs) noexcept {
        if (this != &rhs) {
            source = rhs.source;
            session = rhs.session;

            rhs.source = 0;
            rhs.session = 0;

            message = std::move(rhs.message);
        }
    }

    Envelope &Envelope::operator=(Envelope &&rhs) noexcept {
        if (this != &rhs) {
            source = rhs.source;
            session = rhs.session;
            rhs.source = 0;
            rhs.session = 0;
            message = std::move(rhs.message);
        }
        return *this;
    }
}
