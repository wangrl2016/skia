//
// Created by wangrl on 2020/11/27.
//

#include <SkSVGImage.h>

SkSVGImage::SkSVGImage(SkSVGTag t) : INHERITED(t) {

}

void SkSVGImage::appendChild(sk_sp<SkSVGNode>) {
    SkDebugf("cannot append child nodes to an SVG image.\n");
}

