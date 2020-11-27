//
// Created by wangrl on 2020/11/26.
//

#include <iostream>

#include "src/xml/SkDOM.h"
#include "modules/svgif/include/SVGIDMapper.h"
#include "modules/svgif/include/SVGDOM.h"
#include "modules/svgif/include/SVGNode.h"
#include "modules/svgif/include/SVGText.h"

namespace {

    struct ConstructionContext {
        ConstructionContext(SVGIDMapper* mapper) : fParent(nullptr), fIDMapper(mapper) {}

        ConstructionContext(const ConstructionContext& other, const sk_sp<SVGNode>& newParent)
                : fParent(newParent.get()), fIDMapper(other.fIDMapper) {}

        SVGNode* fParent;
        SVGIDMapper* fIDMapper;
    };

    sk_sp<SVGNode> construct_svg_node(const SkDOM& dom, const ConstructionContext& ctx,
                                      const SkDOM::Node* xmlNode) {
        // 获取根目录元素
        const char* elem = dom.getName(xmlNode);
        const SkDOM::Type elemType = dom.getType(xmlNode);
        SkDebugf("Root node name %s, type %d\n", elem, elemType);

        if (elemType == SkDOM::kText_Type) {    // 包含文字
            SkASSERT(dom.countChildren(xmlNode) == 0);
            // TODO: add type conversion helper to SkSVGNode
            if (ctx.fParent->tag() == SVGTag::kText) {
                static_cast<SVGText*>(ctx.fParent)->setText(SkString(dom.getName(xmlNode)));
            }
            return nullptr
        }

        SkASSERT(elemType == SkDOM::kElement_Type)

        return nullptr;
    }

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

    auto root = construct_svg_node(xmlDom, ctx, xmlDom.getRootNode());

    return sk_sp<SVGDOM>();
}
