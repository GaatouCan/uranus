#pragma once

#include <chrono>
#include <thread>

#include <asio/as_tuple.hpp>
#include <asio/use_awaitable.hpp>
#include <asio/ip/tcp.hpp>
#include <asio/experimental/concurrent_channel.hpp>

#ifdef URANUS_SSL
#include <asio/ssl/stream.hpp>
#endif

namespace uranus {
    using SteadyTimePoint = std::chrono::steady_clock::time_point;
    using SteadyDuration = std::chrono::duration<SteadyTimePoint>;

    using SystemTimePoint = std::chrono::system_clock::time_point;
    using SystemDuration = std::chrono::duration<SystemTimePoint>;

    using ThreadID = std::thread::id;

    using default_token = asio::as_tuple_t<asio::use_awaitable_t<>>;

#ifdef URANUS_SSL
    using TcpSocket = asio::ssl::stream<default_token::as_default_on_t<asio::ip::tcp::socket>>;
#else
    using TcpSocket = default_token::as_default_on_t<asio::ip::tcp::socket>;
#endif

    using TcpAcceptor = default_token::as_default_on_t<asio::ip::tcp::acceptor>;

    template<class T>
    using ConcurrentChannel = default_token::as_default_on_t<asio::experimental::concurrent_channel<void(std::error_code, T)>>;
}