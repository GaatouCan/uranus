#pragma once

#include "base.export.h"

#include <memory>

namespace uranus {

    class BASE_API Message {

    public:
        class Deleter {

        public:
            using Functor = void (*)(Message *) noexcept;

            constexpr Deleter() noexcept
                : del_(&defaultImpl) { }

            explicit constexpr Deleter(const Functor del) noexcept: del_(del) {

            }

            static constexpr Deleter make() noexcept {
                return Deleter(&defaultImpl);
            }

            template<typename T>
            requires requires(T *t)
            {
                { t->recycle() } -> std::convertible_to<void>;
            } && std::is_base_of_v<Message, T>
            static constexpr Deleter recyclerAdapter () noexcept {
                return Deleter(&recycleImpl<T>);
            }

            constexpr void operator()(Message *p) const noexcept {
                std::invoke(del_, p);
            }

        private:
            static constexpr void defaultImpl(Message *p) noexcept {
                delete p;
            }

            template<typename T>
            static constexpr void recycleImpl(Message *p) noexcept {
                dynamic_cast<T *>(p)->recycle();
            }

        private:
            Functor del_;
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


#define DECLARE_MESSAGE_POOL_GET(type)                      \
private:                                                    \
    friend class type##Pool;                                \
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
        type##Pool::instance().acquire(),                   \
        Deleter::recyclerAdapter<type>()                    \
    };                                                      \
}
