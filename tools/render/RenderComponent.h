//
// Created by wangrl on 2020/9/18.
//

#pragma once

#include "include/core/SkSurface.h"
#include "include/core/SkImage.h"

namespace render {
    struct RenderComponent {
        sk_sp<SkSurface> mSurface;

        // 输出的图片
        sk_sp<SkImage> mImage;

        int mFrameCount = 0;

        RenderComponent() {}
    };
}
