//
// Created by wangrl on 2020/9/17.
//

#pragma once

namespace render {
    struct AnimationComponent {
    public:
        AnimationComponent(std::shared_ptr<skottie::Animation> animation) : mAnimation(animation) {}

    private:
        std::shared_ptr<skottie::Animation> mAnimation;
    };
}