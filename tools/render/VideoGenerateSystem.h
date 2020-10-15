//
// Created by wangrl on 2020/9/18.
//

#pragma once

#include "tools/render/System.h"

namespace render {

    class VideoGenerateSystem : public System {
    public:

        VideoGenerateSystem() = default;

        void init() override;

        void update() override;

        void finalize() override;
    };

}
