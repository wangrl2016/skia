//
// Created by wangrl on 2020/12/8.
//

#include "include/effects/SkImageFilters.h"
#include "modules/svg/include/SkSVGFeGaussianBlur.h"

sk_sp<SkImageFilter>
SkSVGFeGaussianBlur::onMakeImageFilter(const SkSVGRenderContext&, const SkSVGFilterContext&) const {
    return sk_sp<SkImageFilter>();
}

bool SkSVGFeGaussianBlur::parseAndSetAttribute(const char*, const char*) {
    return false;
}
