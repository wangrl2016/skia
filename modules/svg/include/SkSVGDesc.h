//
// Created by wangrl on 2020/12/8.
//

#pragma once

#include "modules/svg/include/SkSVGHiddenContainer.h"

class SkSVGDesc : public SkSVGHiddenContainer {
public:
    static sk_sp<SkSVGDesc> Make() { return sk_sp<SkSVGDesc>(new SkSVGDesc()); }

private:
    SkSVGDesc() : INHERITED(SkSVGTag::kDesc) {}

    using INHERITED = SkSVGHiddenContainer;
};
