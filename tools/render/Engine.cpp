//
// Created by wangrl on 2020/9/18.
//


#include "tools/render/Engine.h"
#include "tools/render/RenderSystem.h"
#include "tools/render/VideoGenerateSystem.h"

namespace render {
    void Engine::init() {
        reset();

        mSystems.emplace_back(new RenderSystem);

        for (auto& sys : systems()) {
            sys->init();
        }
    }

    void Engine::reset() {

    }

    void Engine::update() {
        printf("Engine update\n");
    }

    bool Engine::destroy() {
        return false;
    }

    Engine::Engine() {

    }

    Engine::~Engine() {

    }
}