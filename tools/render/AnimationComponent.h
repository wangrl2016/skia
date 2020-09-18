//
// Created by wangrl on 2020/9/17.
//

#pragma once

namespace render {
    struct AnimationComponent {
        enum State {
            IDLE, START, STOP
        };

    public:
        AnimationComponent(std::shared_ptr<skottie::Animation> animation) : mAnimation(animation) {}

        std::shared_ptr<skottie::Animation> getAnimation() {
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

    private:
        std::shared_ptr<skottie::Animation> mAnimation;

        State mState = State::IDLE;
    };
}