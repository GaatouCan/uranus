#pragma once

#include "FormData.h"

#include <base/Message.h>
#include <base/Recycler.h>


namespace uranus::http {

    class HTTP_API HttpRequest final : public Message {

        friend class HttpServerCodec;

    public:
        HttpRequest() = delete;

        DECLARE_MESSAGE_POOL_GET(HttpRequest)

        explicit HttpRequest(const HttpRequestRecyclerHandle &handle);
        ~HttpRequest() override;

        void recycle();

        [[nodiscard]] std::string toString() const;

        [[nodiscard]] int getMethod() const;
        [[nodiscard]] const std::string& getUrl() const;

        [[nodiscard]] bool hasHeader(const std::string& key) const;
        [[nodiscard]] auto getHeaders(const std::string &key) const -> std::pair<Headers::const_iterator, Headers::const_iterator>;
        [[nodiscard]] std::string getHeader(const std::string &key, size_t id = 0, const std::string &def = "") const;
        [[nodiscard]] size_t getHeaderValueAsULongLong(const std::string &key, size_t id = 0, size_t def = 0) const;

        [[nodiscard]] size_t getHeaderValueCount(const std::string &key) const;

        [[nodiscard]] bool hasParam(const std::string &key) const;
        [[nodiscard]] std::string getParam(const std::string &key, size_t id = 0) const;
        [[nodiscard]] size_t getParamValueCount(const std::string &key) const;

        [[nodiscard]] bool isMultipartFormData() const;

        [[nodiscard]] std::string getBearerTokenAuth() const;

        [[nodiscard]] bool isChunkedTransferEncoding() const;
    private:
        HttpRequestRecyclerHandle handle_;

    public:
        int method_;
        std::string target_;
        std::string version_;
        std::string path_;

        std::string matchedRoute_;

        Params      params_;
        Headers     headers_;
        Headers     tailers_;

        std::string body_;
        MultipartFormData form_;

        std::string remoteAddr_;
        int         remotePort_;

        PathParams  pathParams_;
        Ranges      ranges_;

        std::vector<std::string> acceptContentType_;
    };

    DECLARE_MESSAGE_POOL(HttpRequest)
}
