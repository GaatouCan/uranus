#include "PackageCodec.h"

namespace uranus {

    PackageCodec::PackageCodec(ConnectionBase &conn)
        : MessageCodec(conn) {
    }

    PackageCodec::~PackageCodec() {
    }

    awaitable<error_code> PackageCodec::encode(HandleType &&msg) {
        // TODO:
        co_return error_code{};
    }

    awaitable<tuple<error_code, MessageCodec<Package>::HandleType>> PackageCodec::decode() {
        // TODO
        co_return make_tuple(error_code{}, nullptr);
    }
}