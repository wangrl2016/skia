//
// Created by wangrl on 2020/11/27.
//

#include "modules/svg/include/SkSVGRenderContext.h"
#include "modules/svg/include/SkSVGImage.h"
#include "modules/svg/include/SkSVGValue.h"
#include "include/core/SkImage.h"
#include "include/core/SkCanvas.h"

SkSVGImage::SkSVGImage() : INHERITED(SkSVGTag::kImage) {}

void SkSVGImage::appendChild(sk_sp<SkSVGNode>) {
    SkDebugf("cannot append child nodes to an SVG image.\n");
}

void SkSVGImage::onRender(const SkSVGRenderContext& ctx) const {
    auto asset = ctx.resourceProvider()->loadImageAsset("", fLinkHref.c_str(), fLinkHref.c_str());
    if (!asset) {
        SkDebugf("Could not load image asset\n");
    }

    auto frame = asset->getFrame(0);

    auto scaleX = fWidth / frame->bounds().width();
    auto scaleY = fHeight / frame->bounds().height();

    ctx.canvas()->scale(scaleX, scaleY);
    ctx.canvas()->drawImage(frame, fX, fY);
}

void SkSVGImage::setImageX(const SkSVGLength& x) {
    fX = x.value();
}

void SkSVGImage::setImageY(const SkSVGLength& y) {
    fY = y.value();
}

void SkSVGImage::setImageWidth(const SkSVGLength& w) {
    fWidth = w.value();
}

void SkSVGImage::setImageHeight(const SkSVGLength& h) {
    fHeight = h.value();
}

void SkSVGImage::setLinkHref(const SkString& href) {
    fLinkHref = href;
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
        case SkSVGAttribute::kHref:
            if (const auto* href = v.as<SkSVGStringValue>()) {
                this->setLinkHref(*href);
            }
            break;
        default:
            SkDebugf("Error attribute %d\n", attr);
    }
}
