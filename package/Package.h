#pragma once

#include "base/Recycler.h"

#include <mimalloc.h>
#include <span>

namespace uranus {
    struct Message;
}

namespace uranus::network {

    class NETWORK_API Package final {

#pragma region Buffer Heap
        class BufferHeap final {

        public:
            static void *Allocate(std::size_t size);
            static void Deallocate(void *ptr);

        private:
            static mi_heap_t *heap_;
        };

        template<class T>
        class BufferAllocator final {
        public:
            using value_type = kClearType<T>;

            BufferAllocator() = default;

            template <class U>
            BufferAllocator(const BufferAllocator<U> &) noexcept {}

            value_type *allocate(std::size_t n) {
                auto *ptr = static_cast<value_type *>(BufferHeap::Allocate(n * sizeof(value_type)));
                if (ptr != nullptr)
                    return ptr;
                throw std::bad_alloc();
            }

            void deallocate(T* p, std::size_t) noexcept {
                BufferHeap::Deallocate(p);
            }
        };
#pragma endregion
        using Buffer = std::vector<uint8_t, BufferAllocator<uint8_t>>;

        friend class PackagePool;

        explicit Package(Recycler<Package>::Handle handle);

    public:
        struct PackageHeader final {
            int32_t id;
            int32_t length;
        };

        Package() = delete;
        ~Package();

        void Recycle();

        void SetPackageID(int32_t id);
        [[nodiscard]] int32_t GetPackageID() const;

        void SetData(std::span<const uint8_t> data);
        void SetData(const uint8_t* data, std::size_t len);
        void SetData(const std::vector<uint8_t>& data);

        void SetData(const std::string& str);
        void SetData(std::string_view sv);

        [[nodiscard]] std::string ToString() const;

        static constexpr size_t kPackageHeaderSize = sizeof(PackageHeader);

        static void ReleaseMessage(const Message *msg);

    private:
        Recycler<Package>::Handle handle_;

    public:
        PackageHeader header_;
        Buffer payload_;
    };
}
