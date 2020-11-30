//
// Created by wangrl on 2020/11/27.
//

#include "modules/svg/include/SkSVGRenderContext.h"
#include "modules/svg/include/SkSVGImage.h"

SkSVGImage::SkSVGImage(SkSVGTag t) : INHERITED(t) {

}

void SkSVGImage::appendChild(sk_sp<SkSVGNode>) {
    SkDebugf("cannot append child nodes to an SVG image.\n");
}

void SkSVGImage::onRender(const SkSVGRenderContext& ctx) const {

}

