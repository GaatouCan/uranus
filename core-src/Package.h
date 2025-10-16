#pragma once

#include "base/Recycler.h"

#include <vector>
#include <mimalloc.h>
#include <span>
#include <string>


class Package final {

#pragma region Buffer Memory Heap
    class BufferHeap final {

    public:
        static void *allocate(std::size_t size);
        static void deallocate(void *ptr);

    private:
        static mi_heap_t *kHeap;
    };

    template<typename T>
    struct BufferAllocator {

        using value_type= std::remove_cvref_t<std::remove_pointer_t<std::remove_all_extents_t<T>>>;

        BufferAllocator() noexcept = default;

        template<typename U>
        explicit BufferAllocator(const BufferAllocator<U> &) noexcept {}

        template<typename U>
        bool operator==(const BufferAllocator<U> &) const noexcept {
            return true;
        }

        template<typename U>
        bool operator!=(const BufferAllocator<U> &) const noexcept {
            return false;
        }

        T *allocate(const std::size_t n) {
            if (n == 0)
                return nullptr;

            if (n > static_cast<std::size_t>(-1) / sizeof(T))
                throw std::bad_alloc();

            T *ptr = static_cast<T *>(BufferHeap::allocate(n * sizeof(T)));
            if (!ptr)
                throw std::bad_alloc();

            return ptr;
        }

        void deallocate(void *ptr, std::size_t size) noexcept {
            BufferHeap::deallocate(ptr);
        }
    };
#pragma endregion

    using Buffer = std::vector<uint8_t, BufferAllocator<uint8_t>>;
    using Destroyer = void (*)(void *);

public:

    enum PackageType {

    };

    struct PackageHeader {
        int32_t type;
        int32_t session;
        int64_t source;
        size_t length;
    };

private:
    friend class PackagePool;

    explicit Package(Recycler<Package>::Handle handle);

public:
    Package() = delete;
    ~Package();

    DISABLE_COPY(Package);

    Package(Package &&rhs) noexcept;
    Package &operator=(Package &&rhs) noexcept;

    void SetData(std::span<const uint8_t> data);
    void SetData(const uint8_t *data, std::size_t length);
    void SetData(const std::vector<uint8_t> &data);
    void SetData(const std::string &str);
    void SetData(std::string_view sv);

    template<class T, class... Args>
    T *CreateObject(Args &&... args) {
        if (sizeof(T) > payload_.size())
            return nullptr;

        T *obj = ::new (payload_.data()) T(std::forward<Args>(args)...);
        destroyer_ = &DestroyObject<T>;

        return obj;
    }

    template<class T>
    T *As() const noexcept {
        return reinterpret_cast<T *>(payload_.data());
    }

    void Recycle();

private:
    template<class T>
    static void DestroyObject(void *ptr) noexcept {
        reinterpret_cast<T *>(ptr)->~T();
    }

private:
    Recycler<Package>::Handle handle_;

public:
    PackageHeader header_;
    Buffer payload_;

    Destroyer destroyer_;
};
