//
// Created by wangrl on 2020/9/18.
//

#pragma once

namespace render {
    struct RenderComponent {
    public:
        RenderComponent() {}

    private:
        std::shared_ptr<SkSurface> mSurface;

        // 输出的图片
        std::shared_ptr<SkImage> mImage;
    };
}
