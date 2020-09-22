//
// Created by wangrl on 2020/9/17.
//

#pragma once

#include "modules/skottie/include/Skottie.h"

namespace render {
    struct AnimationComponent {
        enum State {
            IDLE, START, STOP
        };

    public:
        AnimationComponent(sk_sp<skottie::Animation> animation) : mAnimation(animation) {}

        sk_sp<skottie::Animation> getAnimation() {
            return mAnimation;
        }

        State getRenderState() {
            return mState;
        }

        static bool finished() {
            const auto view = Engine::registry().view<AnimationComponent>();
            return !view.empty() &&
                   std::all_of(view.begin(), view.end(), [view](auto e) {
                       return view.get(e).getRenderState() == AnimationComponent::State::STOP;
                   });
        }

        int mFps;   // 用户设置的帧率

        float mAdvance;   // 每次前进的时间距离

        float mTimeOffset = 0;

        sk_sp<skottie::Animation> mAnimation;

        State mState = State::IDLE;
    };
}