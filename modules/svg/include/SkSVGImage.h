//
// Created by wangrl on 2020/11/27.
//

#pragma once

#include "modules/svg/include/SkSVGTransformableNode.h"

class SkSVGImage final : public SkSVGTransformableNode {
public:
    ~SkSVGImage() override = default;

    void appendChild(sk_sp<SkSVGNode>) override;

protected:
    SkSVGImage(SkSVGTag);

    void onRender(const SkSVGRenderContext&) const final;

private:
    using INHERITED = SkSVGTransformableNode;

};
