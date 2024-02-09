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

#ifndef ES2PANDA_IR_EXPRESSION_UNARY_EXPRESSION_H
#define ES2PANDA_IR_EXPRESSION_UNARY_EXPRESSION_H

#include "ir/expression.h"
#include "lexer/token/tokenType.h"

namespace ark::es2panda::compiler {
class PandaGen;
class ETSGen;
}  // namespace ark::es2panda::compiler

namespace ark::es2panda::checker {
class TSChecker;
class Type;
class ETSAnalyzer;
class TSAnalyzer;
}  // namespace ark::es2panda::checker

namespace ark::es2panda::ir {
class UnaryExpression : public Expression {
public:
    UnaryExpression() = delete;
    ~UnaryExpression() override = default;

    NO_COPY_SEMANTIC(UnaryExpression);
    NO_MOVE_SEMANTIC(UnaryExpression);

    explicit UnaryExpression(Expression *const argument, lexer::TokenType const unaryOperator)
        : Expression(AstNodeType::UNARY_EXPRESSION), argument_(argument), operator_(unaryOperator)
    {
    }
    // NOTE (somasimon): these friend relationships can be removed once there are getters for private fields
    friend class checker::ETSAnalyzer;
    friend class checker::TSAnalyzer;

    [[nodiscard]] lexer::TokenType OperatorType() const noexcept
    {
        return operator_;
    }

    [[nodiscard]] const Expression *Argument() const noexcept
    {
        return argument_;
    }

    // NOLINTNEXTLINE(google-default-arguments)
    [[nodiscard]] UnaryExpression *Clone(ArenaAllocator *allocator, AstNode *parent = nullptr) override;

    void TransformChildren(const NodeTransformer &cb) override;
    void Iterate(const NodeTraverser &cb) const override;
    void Dump(ir::AstDumper *dumper) const override;
    void Dump(ir::SrcDumper *dumper) const override;
    void Compile(compiler::PandaGen *pg) const override;
    void Compile(compiler::ETSGen *etsg) const override;
    checker::Type *Check(checker::TSChecker *checker) override;
    checker::Type *Check([[maybe_unused]] checker::ETSChecker *checker) override;

    void Accept(ASTVisitorT *v) override
    {
        v->Accept(this);
    }

private:
    Expression *argument_;
    lexer::TokenType operator_;
};
}  // namespace ark::es2panda::ir

#endif
