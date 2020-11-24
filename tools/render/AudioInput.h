//
// Created by wangrl on 2020/11/24.
//

#pragma once

namespace render {
    struct AudioInput {
        std::string filePath;
        float startTime;
        float duration;

        AudioInput(std::string path, float start, float d) :
                startTime(start), duration(d) {

            if (startTime < 0) {
                startTime = 0;
            }

            if (!path.empty() && (path.size() > 4) &&
                path.substr(path.size() - 4, 4).compare(".mp3") == 0) {
                filePath = path;
            } else {
                filePath = "";
            }
        }
    };

}
