/*
 * Copyright 2020 Google LLC
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#ifndef SKSL_DSL_EXPRESSION
#define SKSL_DSL_EXPRESSION

#include "include/core/SkTypes.h"
#include "src/sksl/ir/SkSLIRNode.h"

#include <cstdint>
#include <memory>

namespace SkSL {

class Expression;

namespace dsl {

class DSLExpression;

/**
 * Represents an expression such as 'cos(x)' or 'a + b'.
 */
class DSLExpression {
public:
    DSLExpression(const DSLExpression&) = delete;

    DSLExpression(DSLExpression&&) = default;

    DSLExpression();

    /**
     * Creates an expression representing a literal float.
     */
    DSLExpression(float value);

    /**
     * Creates an expression representing a literal float.
     */
    DSLExpression(double value)
        : DSLExpression((float) value) {}

    /**
     * Creates an expression representing a literal int.
     */
    DSLExpression(int value);

    /**
     * Creates an expression representing a literal bool.
     */
    DSLExpression(bool value);

    ~DSLExpression();

    /**
     * Invalidates this object and returns the SkSL expression it represents.
     */
    std::unique_ptr<SkSL::Expression> release();

private:
    DSLExpression(std::unique_ptr<SkSL::Expression> expression);

    std::unique_ptr<SkSL::Expression> fExpression;

    friend class DSLWriter;
};

} // namespace dsl

} // namespace SkSL

#endif
