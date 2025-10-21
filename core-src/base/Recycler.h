#pragma once

#include "Common.h"
#include "ConcurrentQueue.h"

#include <memory>
#include <vector>
#include <cmath>


using std::weak_ptr;


template<class T>
using kClearType = std::remove_cvref_t<std::remove_pointer_t<std::remove_all_extents_t<T>>>;


template<class T>
class Recycler : public std::enable_shared_from_this<Recycler<T>> {

    static constexpr int    kRecyclerHalfCollect            = 256;
    static constexpr int    kRecyclerFullCollect            = 512;
    static constexpr float  kRecyclerCollectThreshold       = 0.3f;
    static constexpr float  kRecyclerCollectRate            = 0.5f;
    static constexpr int    kRecyclerMinimumCapacity        = 64;

    friend class Handle;

public:
    using Type = kClearType<T>;

    class Handle {
        weak_ptr<Recycler> owner;
    public:
        Handle() = delete;
        explicit Handle(const weak_ptr<Recycler> &wPtr)
            : owner(wPtr) {}

        void Recycle(Type *ptr) {
            if (const auto temp = owner.lock()) {
                temp->Recycle(ptr);
                return;
            }

            delete ptr;
        }
    };

    Recycler()
        : usage_(-1),
          half_collect_(kRecyclerHalfCollect),
          full_collect_(kRecyclerFullCollect),
          minimum_capacity_(kRecyclerMinimumCapacity),
          collect_threshold_(kRecyclerCollectThreshold),
          collect_rate_(kRecyclerCollectRate) {

    }

    virtual ~Recycler() {
        std::vector<Type *> bulk(16);

        while (true) {
            size_t num = queue_.try_dequeue_bulk(bulk.data(), 16);

            if (num == 0)
                break;

            for (; num > 0; --num) {
                delete bulk[num - 1];
            }
        }
    }

    DISABLE_COPY_MOVE(Recycler)

    void SetHalfCollect(const int num) {
        half_collect_ = num;
    }

    void SetFullCollect(const int num) {
        full_collect_ = num;
    }

    void SetMinimumCapacity(const int num) {
        minimum_capacity_ = num;
    }

    void SetCollectThreshold(const double threshold) {
        collect_threshold_ = threshold;
    }

    void SetCollectRate(const double rate) {
        collect_rate_ = rate;
    }

    void Initial(const size_t capacity = kRecyclerMinimumCapacity) {
        // Recycle has already initial
        if (usage_.load(std::memory_order_acquire) >= 0) {
            return;
        }

        std::vector<Type *> bulk(capacity);

        const Handle handle(this->weak_from_this());

        for (auto &ptr : bulk) {
            ptr = this->Create(handle);
        }

        queue_.enqueue_bulk(bulk.data(), capacity);
        usage_.store(0, std::memory_order_release);
    }

    Type *Acquire() {
        if (usage_.load(std::memory_order_acquire) < 0) {
            throw std::runtime_error("Recycler is not yet initial");
        }

        Type *result = nullptr;

        if (queue_.TryDequeue(result)) {
            usage_.fetch_add(1, std::memory_order_relaxed);
            return result;
        }

        const Handle handle(this->weak_from_this());

        result = this->Create(handle);
        usage_.fetch_add(1, std::memory_order_relaxed);

        return result;
    }

    void Shrink() {
        const size_t usage = usage_.load(std::memory_order_relaxed);
        const size_t idle = queue_.GetSizeApprox();
        const size_t total = usage + idle;

        if (idle < half_collect_)
            return;

        const double usage_rate = (static_cast<double>(usage) / static_cast<double>(total));

        if (idle < full_collect_ && usage_rate > collect_threshold_)
            return;

        // Calculate How Many Element To Be Released
        auto num = static_cast<size_t>(std::floor(static_cast<float>(total) * collect_rate_));

        // Check Rest Count Greater Than Zero
        if (total - num < minimum_capacity_)
            num = total - minimum_capacity_;

        if (num <= 0)
            return;

        // Take The Elements Into Vector
        std::vector<Type *> del(num);
        num = queue_.TryDequeueBulk(del.data(), num);

        for (; num > 0; --num) {
            delete del[num - 1];
        }
    }

protected:
    virtual Type *Create(const Handle &) const = 0;

private:
    void Recycle(Type *ptr) {
        if (!ptr)
            return;

        queue_.Enqueue(ptr);
        usage_.fetch_sub(1, std::memory_order_relaxed);
    }

private:
    moodycamel::ConcurrentQueue<Type *> queue_;

    std::atomic<int64_t> usage_;

    size_t half_collect_;
    size_t full_collect_;
    size_t minimum_capacity_;
    double collect_threshold_;
    double collect_rate_;
};
