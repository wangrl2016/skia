//
// Created by wangrl on 2020/9/21.
//

#include "src/utils/SkOSPath.h"
#include "include/core/SkString.h"
#include "tools/render/ImageOutputSystem.h"
#include "tools/render/Engine.h"
#include "tools/render/RenderComponent.h"

namespace render {

    void render::ImageOutputSystem::init() {}

    void ImageOutputSystem::update() {
        auto& registry = Engine::registry();
        auto view = registry.view<RenderComponent>();

        for (auto entity : view) {
            auto& image = view.get<RenderComponent>(entity);
            const auto frameFile = SkStringPrintf("%06d.png", image.mFrameCount);
            SkFILEWStream stream(SkOSPath::Join("out/skottie", frameFile.c_str()).c_str());

            if (!stream.isValid()) {
                SkDebugf("Could not open %s for writing\n", frameFile.c_str());
            }

            auto pngData = image.mImage->encodeToData();
            stream.write(pngData->data(), pngData->size());
        }
    }

    void ImageOutputSystem::finalize() {}
}
