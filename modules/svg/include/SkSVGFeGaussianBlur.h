//
// Created by wangrl on 2020/12/8.
//

#pragma once

#include "modules/svg/include/SkSVGFe.h"


class SkSVGFeGaussianBlur final : public SkSVGFe {
public:
    static sk_sp<SkSVGFeGaussianBlur> Make() {
        return sk_sp<SkSVGFeGaussianBlur>(new SkSVGFeGaussianBlur());
    }

protected:
    sk_sp<SkImageFilter> onMakeImageFilter(const SkSVGRenderContext&,
                                           const SkSVGFilterContext&) const override;

    bool parseAndSetAttribute(const char*, const char*) override;

    std::vector<SkSVGFeInputType> getInputs() const override;

private:
    SkSVGFeGaussianBlur() : INHERITED(SkSVGTag::kFeGaussianBlur) {}

    using INHERITED = SkSVGFe;
};
