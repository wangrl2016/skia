//
// Created by wangrl on 2020/11/24.
//

#pragma once

namespace render {
    struct ImageOutput {

        // 保存Skia渲染出来的图片信息
        sk_sp <SkImage> skImage;

        ImageOutput(sk_sp <SkImage> image) : skImage(image) {}
    };
}
