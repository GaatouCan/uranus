#pragma once

#include "actor.export.h"

#include <base/Message.h>
#include <base/Recycler.h>
#include <vector>

namespace uranus::actor {

    namespace detail {

        class ACTOR_API BufferHeap final {

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

    class ACTOR_API Package final : public Message {

        DECLARE_MESSAGE_POOL_GET(Package)

        explicit Package(const PackageRecyclerHandle &handle);
        ~Package() override;

    public:
        enum PackageType {
            kFromClient     = 1,
            kFromPlayer     = 1 << 1,
            kFromService    = 1 << 2,
            kToClient       = 1 << 3,
            kToPlayer       = 1 << 4,
            kToService      = 1 << 5,
            kRequest        = 1 << 6,
            kResponse       = 1 << 7,
        };

        Package() = delete;

        void setId(int32_t id);
        [[nodiscard]] int64_t getId() const;

        void recycle();

    private:
        PackageRecyclerHandle handle_;

    public:
        int64_t id_;
        std::vector<uint8_t, detail::BufferAllocator<uint8_t>> payload_;
    };

    DECLARE_MESSAGE_POOL(Package)

    struct ACTOR_API Envelope final {
        int32_t type;
        uint32_t source;

        uint32_t session;
        MessageHandle message;

        Envelope();

        Envelope(int32_t ty, uint32_t src, MessageHandle &&msg);
        Envelope(int32_t ty, uint32_t src, uint32_t sess, MessageHandle &&msg);

        Envelope(const Envelope &) = delete;
        Envelope &operator=(const Envelope &) = delete;

        Envelope(Envelope &&rhs) noexcept;
        Envelope &operator=(Envelope &&rhs) noexcept;
    };
}
