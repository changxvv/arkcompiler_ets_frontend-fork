/**
 * Copyright (c) 2021 - 2023 Huawei Device Co., Ltd.
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

#ifndef ES2PANDA_IR_EXPRESSION_CLASS_EXPRESSION_H
#define ES2PANDA_IR_EXPRESSION_CLASS_EXPRESSION_H

#include "ir/expression.h"

namespace panda::es2panda::ir {
class ClassDefinition;

class ClassExpression : public Expression {
public:
    ClassExpression() = delete;
    ~ClassExpression() override = default;

    NO_COPY_SEMANTIC(ClassExpression);
    NO_MOVE_SEMANTIC(ClassExpression);

    explicit ClassExpression(ClassDefinition *const def) : Expression(AstNodeType::CLASS_EXPRESSION), def_(def) {}

    [[nodiscard]] const ClassDefinition *Definition() const noexcept
    {
        return def_;
    }

    // NOLINTNEXTLINE(google-default-arguments)
    [[nodiscard]] ClassExpression *Clone(ArenaAllocator *allocator, AstNode *parent = nullptr) override;

    void TransformChildren(const NodeTransformer &cb) override;
    void Iterate(const NodeTraverser &cb) const override;

    void Dump(ir::AstDumper *dumper) const override;
    void Dump(ir::SrcDumper *dumper) const override;
    void Compile(compiler::PandaGen *pg) const override;
    void Compile(compiler::ETSGen *etsg) const override;
    checker::Type *Check(checker::TSChecker *checker) override;
    checker::Type *Check(checker::ETSChecker *checker) override;

    void Accept(ASTVisitorT *v) override
    {
        v->Accept(this);
    }

private:
    ClassDefinition *def_;
};
}  // namespace panda::es2panda::ir

#endif
