//
// Created by wangrl on 2020/11/24.
//

#pragma once

#include "tools/render/AudioInput.h"

namespace render {

    struct OutputConfig : public MoveOnly {

        // 音频输入
        AudioInput audioInput = AudioInput("", 0.0f, 0.0f);

        // 输出质量控制
        SkISize outputSize = SkISize::Make(0, 0);

        // 输出名字需要有后缀
        // 通常以.mp4结尾
        std::string outputPath = std::string();

        OutputConfig(std::string path) : outputPath(path) {

        }

        OutputConfig(AudioInput input, SkISize size, std::string path) :
                audioInput(input), outputSize(size), outputPath(path) {

        }
    };
}

