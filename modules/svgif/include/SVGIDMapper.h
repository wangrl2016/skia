//
// Created by wangrl on 2020/11/26.
//

#pragma once

#include "include/core/SkRefCnt.h"
#include "include/private/SkTHash.h"

class SkString;

class SVGNode;

using SVGIDMapper = SkTHashMap<SkString, sk_sp<SVGNode>>;
