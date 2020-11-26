//
// Created by wangrl on 2020/11/26.
//

#pragma once

#include "include/core/SkFontMgr.h"

class SkDom;

class SkStream;

class SVGDOM : public SkRefCnt {
public:
    class Builder final {
    public:
        Builder& setFontManager(sk_sp<SkFontMgr>);

        sk_sp<SVGDOM> make(SkStream&) const;

    private:
        sk_sp<SkFontMgr> fFontMgr;
    };

private:


    const sk_sp<SkFontMgr> fFontMgr;
};