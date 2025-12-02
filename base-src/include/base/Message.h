#pragma once

#include "base.export.h"

#include <cstdint>
#include <memory>

namespace uranus {
    // namespace detail {
    //
    //     class BASE_API BufferHeap final {
    //
    //     public:
    //         static void *allocate(std::size_t size);
    //         static void deallocate(void *ptr);
    //     };
    //
    //     template<typename T>
    //     class BufferAllocator final {
    //     public:
    //         using value_type = std::remove_cvref_t<std::remove_pointer_t<std::remove_all_extents_t<T> > >;
    //
    //         BufferAllocator() = default;
    //
    //         template<class U>
    //         explicit BufferAllocator(const BufferAllocator<U> &) noexcept {}
    //
    //         value_type *allocate(size_t n) {
    //             auto *ptr = static_cast<value_type *>(BufferHeap::allocate(n * sizeof(value_type)));
    //             if (ptr != nullptr)
    //                 return ptr;
    //             throw std::bad_alloc();
    //         }
    //
    //         void deallocate(T *p, std::size_t) noexcept {
    //             BufferHeap::deallocate(p);
    //         }
    //     };
    // }

    class BASE_API Message {

    public:
        struct Deleter {
            using Functor = void (*)(Message *);
            Functor del;

            static Deleter make() {
                return { [](Message *p) { delete p; } };
            }

            template<typename T>
            requires requires(T *t)
            {
                { t->recycle() } -> std::convertible_to<void>;
            } && std::is_base_of_v<Message, T>
            static Deleter recyclerAdapter () {
                return { [](Message *p) {
                    dynamic_cast<T *>(p)->recycle();
                } };
            }

            void operator()(Message *p) const noexcept {
                del(p);
            }
        };

        template<typename T>
        requires std::is_base_of_v<Message, T>
        using Pointer = std::unique_ptr<T, Deleter>;

        Message();
        virtual ~Message();

        void setId(uint64_t id);
        [[nodiscard]] uint64_t getId() const;

    public:
        uint64_t id_;
    };

    using MessageHandle = Message::Pointer<Message>;

    struct BASE_API Envelope final {
        uint32_t source;
        uint32_t session;
        MessageHandle message;

        Envelope(uint32_t src, MessageHandle &&msg);
        Envelope(uint32_t src, uint32_t sess, MessageHandle &&msg);
    };
}
