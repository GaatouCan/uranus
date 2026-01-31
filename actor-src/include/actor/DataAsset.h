#pragma once

namespace uranus::actor {

    class DataAsset {

    public:
        DataAsset() = default;
        virtual ~DataAsset() = default;

        virtual DataAsset *clone() = 0;
    };
}
