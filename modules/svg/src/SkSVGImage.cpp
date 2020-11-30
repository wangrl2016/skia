//
// Created by wangrl on 2020/11/27.
//

#include "modules/svg/include/SkSVGRenderContext.h"
#include "modules/svg/include/SkSVGImage.h"

SkSVGImage::SkSVGImage() : INHERITED(SkSVGTag::kImage) {}

void SkSVGImage::appendChild(sk_sp<SkSVGNode>) {
    SkDebugf("cannot append child nodes to an SVG image.\n");
}

void SkSVGImage::onRender(const SkSVGRenderContext& ctx) const {

}

void SkSVGImage::setImageX(const SkSVGLength& ix) {
    fX = ix;
}

void SkSVGImage::setImageY(const SkSVGLength& iy) {
    fY = iy;
}

void SkSVGImage::setImageWidth(const SkSVGLength& iw) {
    fWidth = iw;
}

void SkSVGImage::setImageHeight(const SkSVGLength& ih) {
    fHeight = ih;
}

SkPath SkSVGImage::onAsPath(const SkSVGRenderContext&) const {
    return SkPath();
}
