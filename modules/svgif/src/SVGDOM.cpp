//
// Created by wangrl on 2020/11/26.
//

#include <iostream>

#include "src/xml/SkDOM.h"
#include "modules/svgif/include/SVGIDMapper.h"
#include "modules/svgif/include/SVGDOM.h"
#include "modules/svgif/include/SVGNode.h"

namespace {

    struct ConstructionContext {
        ConstructionContext(SVGIDMapper* mapper) : fParent(nullptr), fIDMapper(mapper) {}

        ConstructionContext(const ConstructionContext& other, const sk_sp<SVGNode>& newParent)
                : fParent(newParent.get()), fIDMapper(other.fIDMapper) {}

        SVGNode* fParent;
        SVGIDMapper* fIDMapper;
    };

} // anonymous namespace


SVGDOM::Builder& SVGDOM::Builder::setFontManager(sk_sp<SkFontMgr> fmgr) {
    fFontMgr = std::move(fmgr);
    return *this;
}

sk_sp<SVGDOM> SVGDOM::Builder::make(SkStream& str) const {
    SkDOM xmlDom;
    if (!xmlDom.build(str)) {
        std::cerr << "XML dom tree build error.\n";
        return nullptr;
    }

    SVGIDMapper mapper;     // 声明
    ConstructionContext ctx(&mapper);

    return sk_sp<SVGDOM>();
}
