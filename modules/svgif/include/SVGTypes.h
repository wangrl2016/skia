//
// Created by wangrl on 2020/11/26.
//

#pragma once

#include "include/core/SkColor.h"
#include "include/core/SkMatrix.h"
#include "include/core/SkPath.h"
#include "include/core/SkPoint.h"
#include "include/core/SkRect.h"
#include "include/core/SkScalar.h"
#include "include/core/SkString.h"
#include "include/core/SkTypes.h"
#include "include/private/SkTDArray.h"
#include "src/core/SkTLazy.h"

using SVGColorType = SkColor;
using SVGIntegerType = int;
using SVGNumberType = Scalar;
using SVGStringType = SkString;
using SVGViewBoxType = SkRect;
using SVGTransformType = SkMatrix;
using SVGPointsType = SkTDArray<SkPoint>;

enum class SVGPropertyState {
    kUnspecified,
    kInherit,
    kValue,
};

template<typename T, bool kInheritable>
class SVGProperty {
public:
    using ValueT = T;


};
