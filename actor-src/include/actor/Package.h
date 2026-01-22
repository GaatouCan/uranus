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
            kEvent          = 1 << 8,
        };

        Package() = delete;

        void setId(int64_t id);
        [[nodiscard]] int64_t getId() const;

        void setData(const std::string &data);
        void setData(const std::vector<uint8_t> &bytes);
        void setData(const uint8_t *data, size_t length);

        [[nodiscard]] std::string toString() const;

        void recycle();

        void copy(Message &other) const override;

    private:
        PackageRecyclerHandle handle_;

    public:
        int64_t id_;
        std::vector<uint8_t, detail::BufferAllocator<uint8_t>> payload_;
    };

    DECLARE_MESSAGE_POOL(Package)

    struct ACTOR_API Envelope final {
        int32_t type;

        int64_t source;
        int64_t session;

        PackageHandle package;

        Envelope();

        Envelope(int32_t ty, int64_t src, PackageHandle &&pkg);
        Envelope(int32_t ty, int64_t src, int64_t sess, PackageHandle &&pkg);

        Envelope(const Envelope &) = delete;
        Envelope &operator=(const Envelope &) = delete;

        Envelope(Envelope &&rhs) noexcept;
        Envelope &operator=(Envelope &&rhs) noexcept;
    };
}
