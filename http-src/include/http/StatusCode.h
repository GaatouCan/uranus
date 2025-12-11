#pragma once

#include <system_error>

namespace uranus::http {
    enum class StatusCode {
        // Information responses
        kContinue           = 100,
        kSwitchingProtocol  = 101,
        kProcessing         = 102,
        kEarlyHints         = 103,

        // Successful responses
        kOk                             = 200,
        kCreated                        = 201,
        kAccepted                       = 202,
        kNonAuthoritativeInformation    = 203,
        kNoContent                      = 204,
        kResetContent                   = 205,
        kPartialContent                 = 206,
        kMultiStatus                    = 207,
        kAlreadyReported                = 208,
        kIMUsed                         = 209,

        // Redirection messages
        kMultipleChoices    = 300,
        kMovedPermanently   = 301,
        kFound              = 302,
        kSeeOther           = 303,
        kNotModified        = 304,
        kUseProxy           = 305,
        kUnused             = 306,
        kTemporaryRedirect  = 307,
        kPermanentRedirect  = 308,

        // Client error responses
        kBadRequest                     = 400,
        kUnauthorized                   = 401,
        kPaymentRequired                = 402,
        kForbidden                      = 403,
        kNotFound                       = 404,
        kMethodNotAllowed               = 405,
        kNotAcceptable                  = 406,
        kProxyAuthenticationRequired    = 407,
        kRequestTimeout                 = 408,
        kConflict                       = 409,
        kGone                           = 410,
        kLengthRequired                 = 411,
        kPreconditionFailed             = 412,
        kPayloadTooLarge                = 413,
        kUriTooLong                     = 414,
        kUnsupportedMediaType           = 415,
        kRangeNotSatisfiable            = 416,
        kExpectationFailed              = 417,
        kImATeapot                      = 418,
        kMisdirectedRequest             = 421,
        kUnprocessableContent           = 422,
        kLocked                         = 423,
        kFailedDependency               = 424,
        kTooEarly                       = 425,
        kUpgradeRequired                = 426,
        kPreconditionRequired           = 428,
        kTooManyRequests                = 429,
        kRequestHeaderFieldsTooLarge    = 431,
        kUnavailableForLegalReasons     = 451,

        // Server error responses
        kInternalServerError            = 500,
        kNotImplemented                 = 501,
        kBadGateway                     = 502,
        kServiceUnavailable             = 503,
        kGatewayTimeout                 = 504,
        kHttpVersionNotSupported        = 505,
        kVariantAlsoNegotiates          = 506,
        kInsufficientStorage            = 507,
        kLoopDetected                   = 508,
        kNotExtended                    = 510,
        kNetworkAuthenticationRequired  = 511,
    };

    constexpr const char *StatusMessage(const int status) {
        switch (static_cast<StatusCode>(status)) {
            case StatusCode::kContinue:             return "Continue";
            case StatusCode::kSwitchingProtocol:    return "Switching Protocol";
            case StatusCode::kProcessing:           return "Processing";
            case StatusCode::kEarlyHints:           return "Early Hints";

            case StatusCode::kOk:                           return "OK";
            case StatusCode::kCreated:                      return "Created";
            case StatusCode::kAccepted:                     return "Accepted";
            case StatusCode::kNonAuthoritativeInformation:  return "Non-Authoritative Information";
            case StatusCode::kNoContent:                    return "No Content";
            case StatusCode::kResetContent:                 return "Reset Content";
            case StatusCode::kPartialContent:               return "Partial Content";
            case StatusCode::kMultiStatus:                  return "Multi-Status";
            case StatusCode::kAlreadyReported:              return "Already Reported";
            case StatusCode::kIMUsed:                       return "IM Used";

            case StatusCode::kMultipleChoices:      return "Multiple Choices";
            case StatusCode::kMovedPermanently:     return "Moved Permanently";
            case StatusCode::kFound:                return "Found";
            case StatusCode::kSeeOther:             return "See Other";
            case StatusCode::kNotModified:          return "Not Modified";
            case StatusCode::kUseProxy:             return "Use Proxy";
            case StatusCode::kUnused:               return "Unused";
            case StatusCode::kTemporaryRedirect:    return "Temporary Redirect";
            case StatusCode::kPermanentRedirect:    return "Permanent Redirect";

            case StatusCode::kBadRequest:                   return "Bad Request";
            case StatusCode::kUnauthorized:                 return "Unauthorized";
            case StatusCode::kPaymentRequired:              return "Payment Required";
            case StatusCode::kForbidden:                    return "Forbidden";
            case StatusCode::kNotFound:                     return "Not Found";
            case StatusCode::kMethodNotAllowed:             return "Method Not Allowed";
            case StatusCode::kNotAcceptable:                return "Not Acceptable";
            case StatusCode::kProxyAuthenticationRequired:  return "Proxy Authentication Required";
            case StatusCode::kRequestTimeout:               return "Request Timeout";
            case StatusCode::kConflict:                     return "Conflict";
            case StatusCode::kGone:                         return "Gone";
            case StatusCode::kLengthRequired:               return "Length Required";
            case StatusCode::kPreconditionFailed:           return "Precondition Failed";
            case StatusCode::kPayloadTooLarge:              return "Payload Too Large";
            case StatusCode::kUriTooLong:                   return "URI Too Long";
            case StatusCode::kUnsupportedMediaType:         return "Unsupported Media Type";
            case StatusCode::kRangeNotSatisfiable:          return "Range Not Satisfiable";
            case StatusCode::kExpectationFailed:            return "Expectation Failed";
            case StatusCode::kImATeapot:                    return "I'm a teapot";
            case StatusCode::kMisdirectedRequest:           return "Misdirected Request";
            case StatusCode::kUnprocessableContent:         return "Unprocessable Content";
            case StatusCode::kLocked:                       return "Locked";
            case StatusCode::kFailedDependency:             return "Failed Dependency";
            case StatusCode::kTooEarly:                     return "Too Early";
            case StatusCode::kUpgradeRequired:              return "Upgrade Required";
            case StatusCode::kPreconditionRequired:         return "Precondition Required";
            case StatusCode::kTooManyRequests:              return "Too Many Requests";
            case StatusCode::kRequestHeaderFieldsTooLarge:  return "Request Header Fields Too Large";
            case StatusCode::kUnavailableForLegalReasons:   return "Unavailable For Legal Reasons";

            case StatusCode::kNotImplemented:                   return "Not Implemented";
            case StatusCode::kBadGateway:                       return "Bad Gateway";
            case StatusCode::kServiceUnavailable:               return "Service Unavailable";
            case StatusCode::kGatewayTimeout:                   return "Gateway Timeout";
            case StatusCode::kHttpVersionNotSupported:          return "HTTP Version Not Supported";
            case StatusCode::kVariantAlsoNegotiates:            return "Variant Also Negotiates";
            case StatusCode::kInsufficientStorage:              return "Insufficient Storage";
            case StatusCode::kLoopDetected:                     return "Loop Detected";
            case StatusCode::kNotExtended:                      return "Not Extended";
            case StatusCode::kNetworkAuthenticationRequired:    return "Network Authentication Required";

            default:
            case StatusCode::kInternalServerError: return "Internal Server Error";
        }
    }

    class StatusCodeCategory final : public std::error_category {
    public:
        [[nodiscard]] constexpr const char *name() const noexcept override {
            return "http::StatusCode";
        }

        [[nodiscard]] std::string message(const int ev) const override {
            return StatusMessage(ev);
        }
    };

    inline const std::error_category& status_category() noexcept {
        static StatusCodeCategory inst;
        return inst;
    }

    inline std::error_code make_error_code(StatusCode code) noexcept {
        return { static_cast<int>(code), status_category() };
    }

    enum class CodecErrorCode {
        kMessageNull,
        kTypeNotSupported,
        kFrameNotMasked,
        kOpcodeNotContinuation,
        kLengthEqualZero,
    };

    class CodecErrorCategory final : public std::error_category {
        [[nodiscard]] constexpr const char *name() const noexcept override {
            return "http::CodecError";
        }

        [[nodiscard]] std::string message(const int ev) const override {
            switch (static_cast<CodecErrorCode>(ev)) {
                case CodecErrorCode::kMessageNull:              return "Message Null Pointer";
                case CodecErrorCode::kTypeNotSupported:         return "Message Type Is Not Supported";
                case CodecErrorCode::kFrameNotMasked:           return "WebSocket Frame Is Not Masked";
                case CodecErrorCode::kOpcodeNotContinuation:    return "WebSocket Opcode Is Not Continuation";
                case CodecErrorCode::kLengthEqualZero:          return "Read Data Length Zero";
            }
            return "";
        }
    };

    inline std::error_code make_error_code(CodecErrorCode code) noexcept {
        return { static_cast<int>(code), status_category() };
    }
}

namespace std {
    template<>
    struct is_error_code_enum<uranus::http::StatusCode> : true_type {};

    template<>
    struct is_error_code_enum<uranus::http::CodecErrorCode> : true_type {};
}