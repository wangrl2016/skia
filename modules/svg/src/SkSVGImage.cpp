//
// Created by wangrl on 2020/11/27.
//

#include "modules/svg/include/SkSVGRenderContext.h"
#include "modules/svg/include/SkSVGImage.h"
#include "modules/svg/include/SkSVGValue.h"

SkSVGImage::SkSVGImage() : INHERITED(SkSVGTag::kImage) {}

void SkSVGImage::appendChild(sk_sp<SkSVGNode>) {
    SkDebugf("cannot append child nodes to an SVG image.\n");
}

void SkSVGImage::onRender(const SkSVGRenderContext& ctx) const {

}

void SkSVGImage::setImageX(const SkSVGLength& x) {
    fX = x;
}

void SkSVGImage::setImageY(const SkSVGLength& y) {
    fY = y;
}

void SkSVGImage::setImageWidth(const SkSVGLength& w) {
    fWidth = w;
}

void SkSVGImage::setImageHeight(const SkSVGLength& h) {
    fHeight = h;
}

SkPath SkSVGImage::onAsPath(const SkSVGRenderContext&) const {
    return SkPath();
}

void SkSVGImage::onSetAttribute(SkSVGAttribute attr, const SkSVGValue& v) {
    switch (attr) {
        case SkSVGAttribute::kX:
            if (const auto* ix = v.as<SkSVGLengthValue>()) {
                this->setImageX(*ix);
            }
            break;
        case SkSVGAttribute::kY:
            if (const auto* iy = v.as<SkSVGLengthValue>()) {
                this->setImageY(*iy);
            }
            break;
        case SkSVGAttribute::kWidth:
            if (const auto* iw = v.as<SkSVGLengthValue>()) {
                this->setImageWidth(*iw);
            }
            break;
        case SkSVGAttribute::kHeight:
            if (const auto* ih = v.as<SkSVGLengthValue>()) {
                this->setImageHeight(*ih);
            }
            break;
        default:
            SkDebugf("Error attribute %d\n", attr);
    }
}
