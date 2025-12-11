#pragma once

#include "FormData.h"
#include "StatusCode.h"

#include <base/Message.h>
#include <base/Recycler.h>


namespace uranus::http {

    class HTTP_API HttpResponse final : public Message {

        friend class HttpServerCodec;

    public:
        HttpResponse() = delete;

        DECLARE_MESSAGE_POOL_GET(HttpResponse)

        explicit HttpResponse(const HttpResponseRecyclerHandle &handle);
        ~HttpResponse() override;

        void recycle();

        [[nodiscard]] int getStatus() const;
        void setStatus(StatusCode code);

        [[nodiscard]] bool hasHeader(const std::string &key) const;
        [[nodiscard]] auto getHeaders(const std::string &key) const -> std::pair<Headers::const_iterator, Headers::const_iterator>;
        [[nodiscard]] std::string getHeader(const std::string &key, size_t id = 0, const std::string &def = "") const;

        void setHeader(const std::string &key, const std::string &value);

        void setContent(const std::string &data, const std::string &type);

    private:
        HttpResponseRecyclerHandle handle_;

        int status_;

        Headers headers_;
        Headers trailers_;

        std::string body_;
        std::string location_;

        size_t contentLength_;
    };

    DECLARE_MESSAGE_POOL(HttpResponse)
}
