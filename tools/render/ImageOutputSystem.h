//
// Created by wangrl on 2020/9/21.
//

#pragma once

#include "tools/render/System.h"

namespace render {
    class ImageOutputSystem : public System {
    public:
        void init() override;

        void update() override;

        void finalize() override;
    };
}
