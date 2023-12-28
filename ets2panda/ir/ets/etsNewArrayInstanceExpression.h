/*
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

#ifndef ES2PANDA_IR_ETS_NEW_ARRAY_INSTANCE_EXPRESSION_H
#define ES2PANDA_IR_ETS_NEW_ARRAY_INSTANCE_EXPRESSION_H

#include "ir/expression.h"

namespace panda::es2panda::checker {
class ETSAnalyzer;
class Signature;
}  // namespace panda::es2panda::checker

namespace panda::es2panda::compiler {
class ETSCompiler;
}  // namespace panda::es2panda::compiler

namespace panda::es2panda::ir {

class ETSNewArrayInstanceExpression : public Expression {
public:
    ETSNewArrayInstanceExpression() = delete;
    ~ETSNewArrayInstanceExpression() override = default;

    NO_COPY_SEMANTIC(ETSNewArrayInstanceExpression);
    NO_MOVE_SEMANTIC(ETSNewArrayInstanceExpression);

    explicit ETSNewArrayInstanceExpression(ArenaAllocator *allocator, ir::TypeNode *const type_reference,
                                           ir::Expression *const dimension)
        : Expression(AstNodeType::ETS_NEW_ARRAY_INSTANCE_EXPRESSION),
          type_reference_(type_reference),
          dimension_(dimension),
          allocator_(allocator)
    {
    }
    // NOTE (csabahurton): these friend relationships can be removed once there are getters for private fields
    friend class checker::ETSAnalyzer;
    friend class compiler::ETSCompiler;

    ir::TypeNode *TypeReference()
    {
        return type_reference_;
    }

    ir::TypeNode const *TypeReference() const
    {
        return type_reference_;
    }

    ir::Expression *Dimension()
    {
        return dimension_;
    }

    ir::Expression const *Dimension() const
    {
        return dimension_;
    }

    void SetDimension(ir::Expression *dimension)
    {
        dimension_ = dimension;
    }

    // NOLINTNEXTLINE(google-default-arguments)
    [[nodiscard]] ETSNewArrayInstanceExpression *Clone(ArenaAllocator *allocator, AstNode *parent = nullptr) override;

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
    ir::TypeNode *type_reference_;
    ir::Expression *dimension_;
    checker::Signature *default_constructor_signature_ {};
    ArenaAllocator *allocator_;
};
}  // namespace panda::es2panda::ir

#endif
