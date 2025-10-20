#pragma once
#include <system_error>

namespace uranus::network {
    enum class PackageErrc {
        kOk = 0,
        kNullMessage = 1,
        kEncodeLengthError = 2,
        kPayloadTooLarge = 3,
        kPayloadLengthError = 4,
        kDecodeLengthError = 5,
    };

    class PackageCategory : public std::error_category {
    public:
        const char *name() const noexcept override {
            return "PackageCategory";
        }

        std::string message(int ev) const override {
            switch (static_cast<PackageErrc>(ev)) {
                case PackageErrc::kOk: return "Success";
                case PackageErrc::kNullMessage: return "Message pointer is null";
                case PackageErrc::kEncodeLengthError: return "Encode length error";
                case PackageErrc::kPayloadTooLarge: return "Payload too large";
                case PackageErrc::kPayloadLengthError: return "Payload length error";
                case PackageErrc::kDecodeLengthError: return "Decode length error";
                default: return "Unknown error";
            }
        }

        static PackageCategory &Instance() {
            static PackageCategory instance;
            return instance;
        }
    };

    inline std::error_code MakeErrorCode(PackageErrc err) {
        return {static_cast<int>(err), uranus::network::PackageCategory::Instance()};
    }
}

namespace std {
    template<>
    struct is_error_code_enum<uranus::network::PackageErrc> : true_type {
    };
}
