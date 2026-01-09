#pragma once

#include "ActorContext.h"

#include <base/types.h>
#include <base/IdentAllocator.h>

#include <asio/co_spawn.hpp>
#include <memory>
#include <functional>


namespace uranus::actor {

    class BaseActor;

    using std::unique_ptr;
    using std::function;
    using asio::awaitable;
    using asio::co_spawn;

    using ActorDeleter  = function<void(BaseActor *)>;
    using ActorHandle   = unique_ptr<BaseActor, ActorDeleter>;

    /// Actor上下文，负责管理Actor生命周期、消息分发及RPC会话
//     class ACTOR_API BaseActorContext : public std::enable_shared_from_this<BaseActorContext> {
//
//         using SessionHandle = asio::any_completion_handler<void(PackageHandle)>;
//
//         /// 会话节点，用于异步调用（RPC）的回调管理
//         struct ACTOR_API SessionNode {
//             /** 完成处理器 **/
//             asio::any_completion_handler<void(PackageHandle)> handle;
//
//             /** 执行器工作保护 **/
//             asio::executor_work_guard<asio::any_completion_executor> work;
//
//             /** 会话ID **/
//             uint32_t sess;
//
//             SessionNode() = delete;
//             SessionNode(SessionHandle &&h, uint32_t s);
//         };
//
//     public:
//         BaseActorContext() = delete;
//
//         explicit BaseActorContext(asio::io_context &ctx);
//         virtual ~BaseActorContext();
//
//         DISABLE_COPY_MOVE(BaseActorContext)
//
//         void setId(uint32_t id);
//         [[nodiscard]] uint32_t getId() const;
//         [[nodiscard]] AttributeMap &attr();
//
//         void setUpActor(ActorHandle &&handle);
//         [[nodiscard]] BaseActor *getActor() const;
//
//         virtual void run();
//         virtual void terminate();
//
//         [[nodiscard]] bool isRunning() const;
//
//         /// 处理由其他Actor发送过来的信封包
//         void pushEnvelope(Envelope &&envelope);
//
// #pragma region For inner actor to call
//         /// 获取指定名称的服务模块
//         virtual ServerModule *getModule(const std::string &name) const = 0;
//         /// 获取服务列表
//         virtual std::map<std::string, uint32_t> getServiceList() const = 0;
//
//         /// 发送数据包到目标Actor（单向）
//         virtual void send(int ty, uint32_t target, PackageHandle &&pkg) = 0;
//
//         /// 异步调用目标Actor并等待返回结果（RPC）
//         template<asio::completion_token_for<void(PackageHandle)> CompletionToken = asio::use_awaitable_t<>>
//         auto call(int ty, uint32_t target, PackageHandle &&req, CompletionToken &&token);
// #pragma endregion
//
//     protected:
//         /// 向目标Actor发送请求
//         virtual void sendRequest(int ty, uint32_t sess, uint32_t target, PackageHandle &&pkg) = 0;
//
//         /// 向请求方返回响应
//         virtual void sendResponse(int ty, uint32_t sess, uint32_t target, PackageHandle &&pkg) = 0;
//
//         /// 错误码回调处理
//         virtual void onErrorCode(std::error_code ec);
//
//         /// 异常回调处理
//         virtual void onException(std::exception &e);
//
//     private:
//         /// 协程处理函数，负责处理邮箱中的消息
//         awaitable<void> process();
//
//     private:
//         /** ASIO IO上下文引用 **/
//         asio::io_context &ctx_;
//
//         /** Actor实例句柄 **/
//         ActorHandle handle_;
//
//         /** 消息信箱 **/
//         ConcurrentChannel<Envelope> mailbox_;
//
//         /** 会话ID分配器 **/
//         IdentAllocator<uint32_t, true> sessAlloc_;
//
//         /** 正在进行的会话 **/
//         std::unordered_map<uint32_t, unique_ptr<SessionNode>> sessions_;
//         std::mutex sessMutex_;
//
//         AttributeMap attr_;
//
//         /** Actor的唯一ID **/
//         uint32_t id_;
//     };
//
//     template<asio::completion_token_for<void(unique_ptr<Package, Message::Deleter>)> CompletionToken>
//     auto BaseActorContext::call(int ty, uint32_t target, PackageHandle &&req, CompletionToken &&token) {
//         return asio::async_initiate<CompletionToken, void(PackageHandle)>([this](
//              asio::completion_handler_for<void(PackageHandle)> auto handler,
//              int type,
//              const uint32_t dest,
//              PackageHandle &&temp
//         ) mutable {
//              // 如果当前ActorContext未运行，则立即返回
//              if (!isRunning()) {
//                  auto work = asio::make_work_guard(handler);
//                  const auto alloc = asio::get_associated_allocator(handler, asio::recycling_allocator<void>());
//                  asio::dispatch(
//                      work.get_executor(),
//                      asio::bind_allocator(alloc, [handler = std::move(handler)]() mutable {
//                          std::move(handler)(nullptr);
//                      })
//                  );
//                  return;
//              }
//
//              const auto sess = sessAlloc_.allocate();
//
//              // 判断会话id是否合法
//              {
//                  unique_lock lock(sessMutex_);
//
//                  if (sessions_.contains(sess)) {
//                      lock.unlock();
//                      const auto work = asio::make_work_guard(handler);
//                      const auto alloc = asio::get_associated_allocator(handler, asio::recycling_allocator<void>());
//                      asio::dispatch(
//                          work.get_executor(),
//                          asio::bind_allocator(alloc, [handler = std::move(handler)]() mutable {
//                              std::move(handler)(nullptr);
//                          })
//                      );
//                      return;
//                  }
//
//                  // 创建新的会话回调节点
//                  sessions_.emplace(sess, make_unique<SessionNode>(std::move(handler), sess));
//              }
//
//              type |= Package::kRequest;
//              sendRequest(type, sess, dest, std::move(temp));
//          }, token, ty, target, std::move(req));
//     }

    class ACTOR_API BaseActorContext : public ActorContext, public std::enable_shared_from_this<BaseActorContext> {

        struct ACTOR_API SessionNode {
            asio::any_completion_handler<void(PackageHandle)> handle;
            asio::executor_work_guard<asio::any_completion_executor> work;
            SteadyTimer timer;

            int64_t sess;

            SessionNode() = delete;
            SessionNode(SessionHandle &&h, uint32_t s);
        };

    public:
        BaseActorContext() = delete;

        BaseActorContext(asio::io_context &ctx, ActorHandle &&handle);
        ~BaseActorContext() override;

        DISABLE_COPY_MOVE(BaseActorContext)

        AttributeMap &attr() override;

        virtual void run();
        virtual void terminate();

        [[nodiscard]] virtual bool isRunning() const;

        [[nodiscard]] BaseActor *getActor() const;

        template<class T>
        requires std::derived_from<T, BaseActor>
        T &getActor() const;

        void pushEnvelope(Envelope &&envelope);

    protected:
        void createSession(int ty, int64_t target, PackageHandle &&req, SessionHandle &&handle) override;

        virtual void sendRequest(int ty, int64_t sess, int64_t target, PackageHandle &&pkg) = 0;
        virtual void sendResponse(int ty, int64_t sess, int64_t target, PackageHandle &&pkg) = 0;

    private:
        awaitable<void> process();

    private:
        asio::io_context &ctx_;
        ExecutorStrand strand_;

        ActorHandle handle_;

        ConcurrentChannel<Envelope> mailbox_;

        IdentAllocator<int64_t, true> sessAlloc_;

        std::unordered_map<int64_t, unique_ptr<SessionNode>> sessions_;
        std::mutex sessMutex_;

        AttributeMap attr_;
    };

    template<class T>
    requires std::derived_from<T, BaseActor>
    T &BaseActorContext::getActor() const {
        if (handle_ == nullptr)
            std::abort();
        return dynamic_cast<T &>(*handle_.get());
    }
}
