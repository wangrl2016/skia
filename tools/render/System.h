//
// Created by wangrl on 2020/9/18.
//

#pragma once

namespace render {
    class System {
    public:
        virtual ~System() = default;

        virtual void init() {}

        virtual void update() = 0;

        virtual void finalize() {}
    };
}
