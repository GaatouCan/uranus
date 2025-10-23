#pragma once

#include "Common.h"

#include <cstdint>


namespace uranus {
    /**
     * The most basic data structure of the framework;
     * It used to pass data between the actors
     */
    struct CORE_API Message final {

        enum MessageType {
            kToService      = 1,
            kToPlayer       = 1 << 1,
            kToClient       = 1 << 2,
            kToServer       = 1 << 3,
            kFromClient     = 1 << 4,
            kFromPlayer     = 1 << 5,
            kFromService    = 1 << 6,
            kFromServer     = 1 << 7,
            kRequest        = 1 << 8,
            kResponse       = 1 << 9,
        };

        /// Define what to do about this message
        int32_t type        = 0;

        /// If it is used to remote call,
        /// this field is the unique key to the session
        int32_t session     = 0;

        /// Record who send this message
        int64_t source      = 0;

        /// The pointer to the custom protocol
        void *  data        = nullptr;

        /// The length of the custom protocol,
        /// It can be zero if you do not need to check the size of the protocol
        size_t  length      = 0;

        Message()   = default;
        ~Message()  = default;

        Message(const Message &rhs) {
            type    = rhs.type;
            session = rhs.session;
            source  = rhs.source;
            data    = rhs.data;
            length  = rhs.length;
        }

        Message &operator=(const Message &rhs) {
            if (this != &rhs) {
                type    = rhs.type;
                session = rhs.session;
                source  = rhs.source;
                data    = rhs.data;
                length  = rhs.length;
            }
            return *this;
        }

        Message(Message &&rhs) noexcept {
            type    = rhs.type;
            session = rhs.session;
            source  = rhs.source;
            data    = rhs.data;
            length  = rhs.length;

            rhs.type        = 0;
            rhs.session     = 0;
            rhs.source      = 0;
            rhs.data        = nullptr;
            rhs.length      = 0;
        }

        Message &operator=(Message &&rhs) noexcept {
            if (this != &rhs) {
                type    = rhs.type;
                session = rhs.session;
                source  = rhs.source;
                data    = rhs.data;
                length  = rhs.length;

                rhs.type        = 0;
                rhs.session     = 0;
                rhs.source      = 0;
                rhs.data        = nullptr;
                rhs.length      = 0;
            }
            return *this;
        }
    };
}
