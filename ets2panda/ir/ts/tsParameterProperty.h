/**
 * Copyright (c) 2021 Huawei Device Co., Ltd.
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 * http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */

#ifndef ES2PANDA_IR_TS_PARAMETER_PROPERTY_H
#define ES2PANDA_IR_TS_PARAMETER_PROPERTY_H

#include "ir/expression.h"
#include "lexer/token/tokenType.h"

namespace panda::es2panda::ir {
enum class AccessibilityOption { NO_OPTS, PUBLIC, PRIVATE, PROTECTED };

class TSParameterProperty : public Expression {
public:
    explicit TSParameterProperty(AccessibilityOption accessibility, Expression *parameter, bool readonly,
                                 bool is_static, bool is_export)
        : Expression(AstNodeType::TS_PARAMETER_PROPERTY),
          accessibility_(accessibility),
          parameter_(parameter),
          readonly_(readonly),
          static_(is_static),
          export_(is_export)
    {
    }

    AccessibilityOption Accessibility() const
    {
        return accessibility_;
    }

    bool Readonly() const
    {
        return readonly_;
    }

    bool IsStatic() const
    {
        return static_;
    }

    bool IsExport() const
    {
        return export_;
    }

    const Expression *Parameter() const
    {
        return parameter_;
    }

    void TransformChildren(const NodeTransformer &cb) override;
    void Iterate(const NodeTraverser &cb) const override;
    void Dump(ir::AstDumper *dumper) const override;
    void Compile([[maybe_unused]] compiler::PandaGen *pg) const override;
    checker::Type *Check([[maybe_unused]] checker::TSChecker *checker) override;
    checker::Type *Check([[maybe_unused]] checker::ETSChecker *checker) override;

private:
    AccessibilityOption accessibility_;
    Expression *parameter_;
    bool readonly_;
    bool static_;
    bool export_;
};
}  // namespace panda::es2panda::ir

#endif
