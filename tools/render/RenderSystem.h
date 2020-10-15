//
// Created by wangrl on 2020/9/18.
//

#pragma once

#include "modules/skottie/include/Skottie.h"
#include "tools/render/System.h"

namespace render {

    class RenderSystem : public System {
    public:
        void init() override;

        void update() override;

        RenderSystem() {}

    private:

        bool handleFrame(skottie::Animation* animation) const;

        void renderFrame(skottie::Animation* animation, SkCanvas* canvas) const;
    };

}
