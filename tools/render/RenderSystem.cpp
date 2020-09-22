//
// Created by wangrl on 2020/9/18.
//

#include "tools/render/RenderSystem.h"
#include "tools/render/Engine.h"
#include "tools/render/AnimationComponent.h"
#include "tools/render/RenderComponent.h"
#include "include/core/SkSize.h"
#include "include/core/SkSurface.h"

namespace render {
    void RenderSystem::init() {
        auto& registry = render::Engine::registry();
        auto view = registry.view<render::AnimationComponent,
                render::RenderComponent>();

        for (auto entity : view) {
            auto& animation = view.get<AnimationComponent>(entity);
            SkISize dim = animation.getAnimation()->size().toRound();

            auto& surface = view.get<RenderComponent>(entity).mSurface;
            if (!surface) {
                 surface = SkSurface::MakeRasterN32Premul(dim.width(), dim.height());
                SkDebugf("Use cpu surface to render\n");
            }
        }
    }

    void RenderSystem::update() {

        auto& registry = Engine::registry();
        auto view = registry.view<AnimationComponent, RenderComponent>();
        for (auto entity : view) {
            auto& animation = view.get<AnimationComponent>(entity);

            if (animation.getRenderState() == AnimationComponent::State::IDLE) {
                static constexpr double kMaxFrames = 1000000;
                animation.mAdvance = 1 / std::min(animation.getAnimation()->duration() * animation.mFps,
                                                  kMaxFrames);
                animation.mState = AnimationComponent::State::START;
            }
            if (animation.mState == AnimationComponent::State::START) {
                animation.mTimeOffset += animation.mAdvance;
                if (animation.mTimeOffset <= 1.0) {
                    animation.getAnimation()->seek(animation.mTimeOffset);

                    if (handleFrame(animation.getAnimation().get())) {
                        auto& registry = Engine::registry();
                        auto view = registry.view<RenderComponent>();
                        for (auto entity : view) {
                            auto& render = view.get<RenderComponent>(entity);
                            render.mImage = render.mSurface->makeImageSnapshot();
                            render.mFrameCount += 1;
                        }
                    }
                } else {
                    animation.mState = AnimationComponent::State::STOP;
                }
            }
        }

    }

    bool RenderSystem::handleFrame(skottie::Animation* animation) const {
        auto& registry = Engine::registry();
        auto view = registry.view<AnimationComponent, RenderComponent>();
        for (auto entity : view) {
            sk_sp<SkSurface> surface = view.get<RenderComponent>(entity).mSurface;
            if (!surface) {
                SkDebugf("Render surface is null\n");
                return false;
            }
            auto* canvas = surface->getCanvas();
            renderFrame(animation, canvas);
        }
        return true;
    }

    void RenderSystem::renderFrame(skottie::Animation* animation, SkCanvas* canvas) const {
        SkAutoCanvasRestore acr(canvas, true);
        canvas->concat(SkMatrix::MakeRectToRect(
                SkRect{0, 0, animation->size().width(), animation->size().height()},
                SkRect::MakeIWH(animation->size().width(), animation->size().height()),
                SkMatrix::kCenter_ScaleToFit));

        canvas->clear(SK_ColorTRANSPARENT);
        animation->render(canvas);
    }
}
