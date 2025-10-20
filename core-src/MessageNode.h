#pragma once

#include "ChannelNode.h"


namespace uranus {

    class Message;

    class CORE_API MessageNode : public ChannelNode {

    protected:
        Message *msg_;

    public:
        MessageNode();
        ~MessageNode() override;

        void SetMessage(Message *msg);

        void Execute(ActorContext *ctx) override;
    };
}
