#pragma once

#include "Package.h"

#include <network/MessageCodec.h>


namespace uranus::actor {

    using network::MessageCodec;
    using network::BaseConnection;
    using asio::awaitable;
    using std::error_code;
    using std::tuple;
    using std::make_tuple;


    class ACTOR_API PackageCodec final : public MessageCodec<Package> {

    public:
#pragma region Error code
        enum class ErrorCode {
            kReadHeaderLength,
            kReadPayloadLength,
            kWriteLength,
        };
#pragma endregion

        explicit PackageCodec(BaseConnection &conn);
        ~PackageCodec() override;

        awaitable<error_code> encode(Package *pkg) override;
        awaitable<ResultTuple> decode() override;
    };

    class ACTOR_API PackageCodecErrorCategory : public std::error_category {

    public:
        [[nodiscard]] constexpr const char *name() const noexcept override {
            return "PackageCodecErrorCategory";
        }

        [[nodiscard]] std::string message(int val) const override;

         static const std::error_category &get() noexcept {
            static PackageCodecErrorCategory _inst;
            return _inst;
        }
    };

    inline std::error_code make_error_code(PackageCodec::ErrorCode ec) noexcept {
        return { static_cast<int>(ec), PackageCodecErrorCategory::get() };
    }
}

template<>
struct std::is_error_code_enum<uranus::actor::PackageCodec::ErrorCode> : true_type {};
