//
// Created by wangrl on 2020/11/24.
//

#pragma once

#include "tools/render/OutputConfig.h"

namespace render {
    struct FFmpeg {

        // audio configure
        static void configAudioInput(entt::entity e, AudioInput* audioInput) {
            auto& r = Engine::registry();

            OutputConfig& outputConfig = r.get<OutputConfig>(e);

            outputConfig.audioInput = *audioInput;
        }
    };
} // namespace render
