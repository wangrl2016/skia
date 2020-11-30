//
// Created by wangrl on 2020/11/27.
//

#pragma once

#include "modules/svg/include/SkSVGTransformableNode.h"

class SkSVGImage final : public SkSVGTransformableNode {
public:
    ~SkSVGImage() override = default;

    void appendChild(sk_sp<SkSVGNode>) override;

    static sk_sp<SkSVGImage> Make() { return sk_sp<SkSVGImage>(new SkSVGImage()); }

    // 设置image标签的属性值
    void setImageX(const SkSVGLength&);

    void setImageY(const SkSVGLength&);

    void setImageWidth(const SkSVGLength&);

    void setImageHeight(const SkSVGLength&);

    void setLinkHref(const SkString&);

    SkPath onAsPath(const SkSVGRenderContext&) const override;

protected:
    void onRender(const SkSVGRenderContext&) const final;

    void onSetAttribute(SkSVGAttribute, const SkSVGValue&) override;

private:
    using INHERITED = SkSVGTransformableNode;

    SkSVGImage();

    SkSVGLength fX = SkSVGLength(0);
    SkSVGLength fY = SkSVGLength(0);
    SkSVGLength fWidth = SkSVGLength(0);
    SkSVGLength fHeight = SkSVGLength(0);

    SkString fLinkHref = SkSVGStringType();

};
