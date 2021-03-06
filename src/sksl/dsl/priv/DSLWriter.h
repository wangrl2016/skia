/*
 * Copyright 2020 Google LLC
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#ifndef SKSL_DSLWRITER
#define SKSL_DSLWRITER

#include "src/sksl/dsl/DSLExpression.h"
#include "src/sksl/ir/SkSLExpressionStatement.h"
#include "src/sksl/ir/SkSLProgram.h"
#include "src/sksl/ir/SkSLStatement.h"
#if !defined(SKSL_STANDALONE) && SK_SUPPORT_GPU
#include "src/gpu/glsl/GrGLSLFragmentProcessor.h"
#endif // !defined(SKSL_STANDALONE) && SK_SUPPORT_GPU

#include <stack>

class AutoDSLContext;

namespace SkSL {

class Compiler;
class Context;
class IRGenerator;
class SymbolTable;
class Type;

namespace dsl {

class ErrorHandler;

/**
 * Thread-safe class that tracks per-thread state associated with DSL output. This class is for
 * internal use only.
 */
class DSLWriter {
public:
    DSLWriter(SkSL::Compiler* compiler);

    /**
     * Returns the Compiler used by DSL operations in the current thread.
     */
    static SkSL::Compiler& Compiler() {
        return *Instance().fCompiler;
    }

    /**
     * Returns the IRGenerator used by DSL operations in the current thread.
     */
    static SkSL::IRGenerator& IRGenerator();

    /**
     * Returns the Context used by DSL operations in the current thread.
     */
    static const SkSL::Context& Context();

    /**
     * Returns the SymbolTable of the current thread's IRGenerator.
     */
    static const std::shared_ptr<SkSL::SymbolTable>& SymbolTable();

    /**
     * Reports an error if the argument is null. Returns its argument unmodified.
     */
    static std::unique_ptr<SkSL::Expression> Check(std::unique_ptr<SkSL::Expression> expr);

    static DSLExpression Construct(const SkSL::Type& type, std::vector<DSLExpression> rawArgs);

    /**
     * Sets the ErrorHandler associated with the current thread. This object will be notified when
     * any DSL errors occur. With a null ErrorHandler (the default), any errors will be dumped to
     * stderr and a fatal exception will be generated.
     */
    static void SetErrorHandler(ErrorHandler* errorHandler) {
        Instance().fErrorHandler = errorHandler;
    }

    /**
     * Notifies the current ErrorHandler that a DSL error has occurred. With a null ErrorHandler
     * (the default), any errors will be dumped to stderr and a fatal exception will be generated.
     */
    static void ReportError(const char* msg);

    static DSLWriter& Instance();

    static void SetInstance(std::unique_ptr<DSLWriter> instance);

private:
    SkSL::Program::Settings fSettings;
    SkSL::Compiler* fCompiler;
    ErrorHandler* fErrorHandler = nullptr;

    friend class DSLCore;
    friend class ::AutoDSLContext;
};

} // namespace dsl

} // namespace SkSL

#endif
