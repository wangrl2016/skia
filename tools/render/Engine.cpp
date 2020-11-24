//
// Created by wangrl on 2020/9/18.
//


#include "tools/render/Engine.h"
#include "tools/render/RenderSystem.h"
#include "tools/render/VideoGenerateSystem.h"
#include "tools/render/ImageOutputSystem.h"

namespace render {
    void Engine::init() {
        reset();

        mSystems.emplace_back(new RenderSystem);
        mSystems.emplace_back(new ImageOutputSystem);
        // 增加图像输出测试系统
        // mSystems.emplace_back(new VideoGenerateSystem);
        mSystems.emplace_back(new VideoGenerateSystem);

        for (auto& sys : systems()) {
            sys->init();
        }
    }

    void Engine::reset() {

    }

    void Engine::update() {
        for (auto& sys : systems()) {
            sys->update();
        }
    }

    bool Engine::destroy() {
        for (auto& sys : systems()) {
            sys->finalize();
        }

        // Destroy all the system in the engine.
        mSystems.erase(mSystems.begin(), mSystems.end());

        // Destroy all the components in the engine.
        reset();

        printf("Engine shutting down...\n");

        return true;
    }

    Engine::Engine() {

    }

    Engine::~Engine() {

    }
}  // namespace render
