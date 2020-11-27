//
// Created by wangrl on 2020/11/26.
//

#pragma once

enum class SVGTag {
    kCircle,

    kText,
};

class SVGNode : public SkRefCnt {
public:
    ~SVGNode() override;

    SVGTag tag() const { return fTag; }

private:

    SVGTag fTag;
};
