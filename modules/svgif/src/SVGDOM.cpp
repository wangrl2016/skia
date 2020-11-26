//
// Created by wangrl on 2020/11/26.
//



#include <modules/svgif/include/SVGDOM.h>

SVGDOM::Builder& SVGDOM::Builder::setFontManager(sk_sp<SkFontMgr> fmgr) {
    fFontMgr = std::move(fmgr);
    return *this;
}

sk_sp<SVGDOM> SVGDOM::Builder::make(SkStream& str) const {
    return sk_sp<SVGDOM>();
}
