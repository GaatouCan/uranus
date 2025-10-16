#pragma once

#include <asio.hpp>
#include <chrono>

using default_token = asio::as_tuple_t<asio::use_awaitable_t<>>;

using asio::awaitable;
using asio::co_spawn;
using asio::detached;

using TcpAcceptor   = default_token::as_default_on_t<asio::ip::tcp::acceptor>;
using TcpSocket     = default_token::as_default_on_t<asio::ip::tcp::socket>;
using TcpResolver   = default_token::as_default_on_t<asio::ip::tcp::resolver>;

using SteadyTimer       = default_token::as_default_on_t<asio::steady_timer>;
using SteadyTimePoint   = std::chrono::steady_clock::time_point;
using SteadyDuration    = std::chrono::steady_clock::duration;

using SystemTimer       = default_token::as_default_on_t<asio::system_timer>;
using SystemTimePoint   = std::chrono::system_clock::time_point;
using SystemDuration    = std::chrono::system_clock::duration;

