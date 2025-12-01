#pragma once

#include "base.export.h"
#include "Recycler.h"

#include <vector>
#include <type_traits>


namespace uranus {
    namespace detail {

        class BASE_API BufferHeap final {

        public:
            static void *allocate(std::size_t size);
            static void deallocate(void *ptr);
        };

        template<typename T>
        class BufferAllocator final {
        public:
            using value_type = std::remove_cvref_t<std::remove_pointer_t<std::remove_all_extents_t<T> > >;

            BufferAllocator() = default;

            template<class U>
            explicit BufferAllocator(const BufferAllocator<U> &) noexcept {}

            value_type *allocate(size_t n) {
                auto *ptr = static_cast<value_type *>(BufferHeap::allocate(n * sizeof(value_type)));
                if (ptr != nullptr)
                    return ptr;
                throw std::bad_alloc();
            }

            void deallocate(T *p, std::size_t) noexcept {
                BufferHeap::deallocate(p);
            }
        };
    }

    class BASE_API Message final {

    public:
        DECLARE_RECYCLER_GET(Message)

        Message() = delete;

        explicit Message(const MessageHandle &handle);
        ~Message();

        void recycle();

        [[nodiscard]] uint64_t getId() const;
        [[nodiscard]] const auto &getPayload() const noexcept {
            return payload_;
        }

    private:
        MessageHandle handle_;

    public:
        uint64_t id_;
        std::vector<uint8_t, detail::BufferAllocator<uint8_t>> payload_;
    };

    DECLARE_RECYCLER(Message)

    struct BASE_API Envelope final {
        uint32_t source;
        uint32_t session;
        Message::MessageUnique message;

        Envelope(uint32_t src, Message::MessageUnique &&msg);
        Envelope(uint32_t src, uint32_t sess, Message::MessageUnique &&msg);
    };
}
