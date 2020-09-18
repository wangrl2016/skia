//
// Created by wangrl on 2020/9/18.
//

#pragma once

#include "System.h"

namespace render {

    class RenderSystem : public System {
    public:
        void init() override;

        void update() override;

        RenderSystem() {}

    private:

    };

}

