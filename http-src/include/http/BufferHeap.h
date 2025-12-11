#pragma once

#include "http.export.h"

#include <new>
#include <type_traits>


namespace uranus::http {
    class HTTP_API BufferHeap final {
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

        value_type *allocate(std::size_t n) {
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