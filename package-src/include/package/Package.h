#pragma once

#include "package.export.h"

#include <base/Message.h>
#include <base/Recycler.h>
#include <vector>


namespace uranus {

    namespace detail {

        class PACKAGE_API BufferHeap final {

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

    class PACKAGE_API Package final : public Message {

        DECLARE_MESSAGE_POOL_GET(Package)

        explicit Package(const PackageRecyclerHandle &handle);
        ~Package() override;

    public:
        Package() = delete;

        void recycle();

    private:
        PackageRecyclerHandle handle_;

    public:
        uint32_t id_;
        std::vector<uint8_t, detail::BufferAllocator<uint8_t>> payload_;
    };

    DECLARE_MESSAGE_POOL(Package)
}
