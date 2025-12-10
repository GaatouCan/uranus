#pragma once

#include "base.export.h"

#include <memory>

namespace uranus {

    class BASE_API Message {

    public:
        struct Deleter {
            using Functor = void (*)(Message *) noexcept;
            Functor del;

            static constexpr Deleter make() noexcept {
                return { &defaultImpl };
            }

            template<typename T>
            requires requires(T *t)
            {
                { t->recycle() } -> std::convertible_to<void>;
            } && std::is_base_of_v<Message, T>
            static constexpr Deleter recyclerAdapter () noexcept {
                return { &recycleImpl<T> };
            }

            constexpr void operator()(Message *p) const noexcept {
                std::invoke(del, p);
            }


            static constexpr void defaultImpl(Message *p) noexcept {
                delete p;
            }

            template<typename T>
            static constexpr void recycleImpl(Message *p) noexcept {
                dynamic_cast<T *>(p)->recycle();
            }
        };

        template<typename T>
        requires std::is_base_of_v<Message, T>
        using Pointer = std::unique_ptr<T, Deleter>;

        Message();
        virtual ~Message();
    };

    using MessageHandle = Message::Pointer<Message>;
}


#define DECLARE_MESSAGE_POOL_GET(type)                      \
private:                                                    \
    friend class _##type##Pool;                             \
    friend class Recycler<type>;                            \
    using type##RecyclerHandle = Recycler<type>::Handle;    \
public:                                                     \
    static type *get();                                     \
    static auto getHandle();                                \
private:

#define DECLARE_MESSAGE_POOL(type)                          \
DECLARE_RECYCLER(type)                                      \
using type##Handle = Message::Pointer<type>;                \
inline auto type::getHandle() {                             \
    return type##Handle{                                    \
        _##type##Pool::instance().acquire(),                \
        Deleter::recyclerAdapter<type>()                    \
    };                                                      \
}
