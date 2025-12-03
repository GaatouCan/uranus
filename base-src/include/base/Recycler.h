#pragma once

#include "ConcurrentQueue.h"

#include <cmath>
#include <stack>
#include <vector>
#include <unordered_map>
#include <shared_mutex>
#include <functional>


namespace uranus {

    using std::vector;
    using std::stack;
    using std::unordered_map;
    using std::shared_mutex;
    using std::shared_lock;
    using std::unique_lock;
    using moodycamel::ConcurrentQueue;
    using ThreadID = std::thread::id;

    template<class T>
    using kClearType = std::remove_cvref_t<std::remove_pointer_t<std::remove_all_extents_t<T>>>;

    template<class T>
    class Recycler {

        static constexpr int    kRecyclerHalfCollect            = 256;
        static constexpr int    kRecyclerFullCollect            = 512;
        static constexpr float  kRecyclerCollectThreshold       = 0.3f;
        static constexpr float  kRecyclerCollectRate            = 0.5f;
        static constexpr int    kRecyclerMinimumCapacity        = 64;

        friend class Handle;

    public:
        using Type = kClearType<T>;

        struct RecyclerDeleter {
            void operator()(Type *p) const noexcept
            requires requires(Type* t) { t->recycle(); }
            {
                if (p) {
                    p->recycle();
                }
            }
        };
        using TypeUniquePtr = std::unique_ptr<T, RecyclerDeleter>;

        class Handle {

            Recycler *owner_;
            ThreadID tid_;

        public:
            Handle() = delete;

            explicit Handle(Recycler *ptr)
                : owner_(ptr) {
                tid_ = kThreadId;
            }

            void recycle(Type *ptr) {
                if (owner_) {
                    owner_->recycle(tid_, ptr);
                    return;
                }
                delete ptr;
            }
        };

        virtual ~Recycler() {
            while (!kStack.empty()) {
                delete kStack.top();
                kStack.pop();
            }

            typename decltype(weakQueue_)::mapped_type weak = nullptr;

            {
                unique_lock lock(mutex_);
                const auto it = weakQueue_.find(kThreadId);
                if (it != weakQueue_.end()) {
                    weak = it->second;
                }
                weakQueue_.erase(it);
            }

            if (weak != nullptr) {
                vector<Type *> bulk(16);

                while (true) {
                    size_t num = weak->try_dequeue_bulk(bulk.data(), 16);

                    if (num == 0)
                        break;

                    while (num-- > 0) {
                        delete bulk[num];
                    }
                }

                delete weak;
            }
        }

        Recycler(const Recycler &) = delete;
        Recycler(Recycler &&) noexcept = delete;

        Recycler &operator=(const Recycler &) = delete;
        Recycler &operator=(Recycler &&) noexcept = delete;

        template<class U>
        explicit Recycler(const Recycler<U> &) = delete;

        template<class U>
        Recycler &operator=(const Recycler<U> &) = delete;

        template<class U>
        explicit Recycler(Recycler<U> &&) noexcept = delete;

        template<class U>
        Recycler &operator=(Recycler<U> &&) noexcept = delete;

        void setHalfCollect(const int num) {
            halfCollect_ = num;
        }

        void setFullCollect(const int num) {
            fullCollect_ = num;
        }

        void setMinimumCapacity(const int num) {
            minimumCapacity_ = num;
        }

        void setCollectThreshold(const double threshold) {
            collectThreshold_ = threshold;
        }

        void setCollectRate(const double rate) {
            collectRate_ = rate;
        }

        Type *acquire() {
            if (kUsage < 0) {
                throw std::runtime_error("Recycler is not yet initial");
            }

            // If local stack not empty
            if (!kStack.empty()) {
                auto *elem = kStack.top();
                kStack.pop();

                ++kUsage;
                return elem;
            }

            // Steal from the weak queue
            if (auto *weak = getWeakQueue(kThreadId)) {
                if (weak->size_approx()) {
                    vector<Type *> bulk(16);

                    while (true) {
                        size_t num = weak->try_dequeue_bulk(bulk.data(), 16);

                        if (num == 0)
                            break;

                        while (num-- > 0)
                            kStack.push(bulk[num]);
                    }

                    // Try pop from local stack again
                    if (!kStack.empty()) {
                        auto *elem = kStack.top();
                        kStack.pop();

                        ++kUsage;
                        return elem;
                    }
                }
            }

            // Create a new one
            auto *elem = this->create(Handle(this));
            ++kUsage;

            return elem;
        }

        TypeUniquePtr acquireUnique() {
            auto *ptr = acquire();
            if (ptr != nullptr) {
                return { ptr };
            }
            return nullptr;
        }

        void shrink() {
            const size_t idle = kStack.size();
            const size_t total = kUsage + idle;

            if (idle < halfCollect_)
                return;

            if (const double usageRate = (static_cast<double>(kUsage) / static_cast<double>(total));
                idle < fullCollect_ && usageRate > collectThreshold_)
                return;

            // Calculate How Many Element To Be Released
            auto num = static_cast<size_t>(std::floor(static_cast<float>(total) * collectRate_));

            // Check Rest Count Greater Than Zero
            if (total - num < minimumCapacity_)
                num = total - minimumCapacity_;

            if (num <= 0)
                return;

            while (num-- > 0 && !kStack.empty()) {
                delete kStack.top();
                kStack.pop();
            }
        }

    protected:
        Recycler()
            : halfCollect_(kRecyclerHalfCollect),
              fullCollect_(kRecyclerFullCollect),
              minimumCapacity_(kRecyclerMinimumCapacity),
              collectThreshold_(kRecyclerCollectThreshold),
              collectRate_(kRecyclerCollectRate) {

        }

        [[nodiscard]] static bool initialized() noexcept {
            return kThreadId != std::thread::id() && kUsage >= 0;
        }

        void initial(const size_t capacity = kRecyclerMinimumCapacity) {
            if (kThreadId == std::thread::id()) {
                kThreadId = std::this_thread::get_id();
            }

            // Recycle has already initial
            if (kUsage >= 0) {
                return;
            }

            for (auto idx = capacity; idx > 0; --idx) {
                auto *elem = this->create(Handle(this));
                kStack.push(elem);
            }

            kUsage = 0;

            unique_lock lock(mutex_);
            weakQueue_[kThreadId] = new ConcurrentQueue<Type *>();
        }

        virtual Type *create(const Handle &) const = 0;

    private:
        void recycle(const ThreadID tid, Type *ptr) {
            if (!ptr)
                return;

            if (tid == kThreadId) {
                kStack.push(ptr);
                --kUsage;
                return;
            }

            if (auto *weak = getWeakQueue(tid)) {
                weak->enqueue(ptr);
                return;
            }

            delete ptr;
        }

        auto getWeakQueue(const ThreadID tid) const {
            shared_lock lock(mutex_);
            const auto it = weakQueue_.find(tid);
            return it != weakQueue_.end() ? it->second : nullptr;
        }

    private:
        static thread_local stack<Type *, vector<Type *>> kStack;
        static thread_local ThreadID kThreadId;
        static thread_local int64_t kUsage;

        size_t halfCollect_;
        size_t fullCollect_;
        size_t minimumCapacity_;
        double collectThreshold_;
        double collectRate_;

        mutable shared_mutex mutex_;
        unordered_map<ThreadID, ConcurrentQueue<Type *> *> weakQueue_;
    };


    template<class T>
    thread_local stack<kClearType<T> *, vector<kClearType<T> *>> Recycler<T>::kStack;

    template<class T>
    thread_local ThreadID Recycler<T>::kThreadId;

    template<class T>
    thread_local int64_t Recycler<T>::kUsage = -1;
}


#define DECLARE_RECYCLER(type)                          \
class type##Pool final : public Recycler<type> {        \
public:                                                 \
    static type##Pool &instance();                      \
protected:                                              \
    type *create(const Handle &handle) const override;  \
};


#define IMPLEMENT_RECYCLER(type)                        \
type##Pool &type##Pool::instance() {                    \
    static type##Pool inst;                             \
    if (!initialized())                                 \
        inst.initial();                                 \
    return inst;                                        \
    }                                                   \
type *type##Pool::create(const Handle &handle) const {  \
    return new type(handle);                            \
}


#define DECLARE_RECYCLER_GET(type)                          \
private:                                                    \
    friend class type##Pool;                                \
    using type##RecyclerHandle = Recycler<type>::Handle;    \
public:                                                     \
    static type *get();


#define IMPLEMENT_RECYCLER_GET(type)            \
type *type::get() {                             \
    return type##Pool::instance().acquire();    \
}
