#include "PackageCodec.h"

#include "Message.h"
#include "Package.h"

#if defined(_WIN32) || defined(_WIN64)
#include <winsock2.h>
#else
#include <arpa/inet.h>
#include <endian.h>
#endif

namespace uranus::network {
    PackageCodec::PackageCodec(SslStream &stream)
        : stream_(stream){
    }

    PackageCodec::~PackageCodec() {
    }

    awaitable<void> PackageCodec::Encode(Message *msg) {
        if (msg == nullptr)
            co_return;

        auto *pkg = static_cast<Package *>(msg->data);
        // TODO
    }

    awaitable<void> PackageCodec::Decode(Message *msg) {
    }
}
