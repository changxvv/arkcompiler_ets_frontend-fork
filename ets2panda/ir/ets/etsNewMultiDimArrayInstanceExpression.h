/**
 * Copyright (c) 2021-2023 Huawei Device Co., Ltd.
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

#ifndef ES2PANDA_IR_ETS_NEW_MULTI_DIM_ARRAY_INSTANCE_EXPRESSION_H
#define ES2PANDA_IR_ETS_NEW_MULTI_DIM_ARRAY_INSTANCE_EXPRESSION_H

#include "ir/expression.h"

namespace panda::es2panda::checker {
class Signature;
}  // namespace panda::es2panda::checker

namespace panda::es2panda::ir {

class ETSNewMultiDimArrayInstanceExpression : public Expression {
public:
    ETSNewMultiDimArrayInstanceExpression() = delete;
    ~ETSNewMultiDimArrayInstanceExpression() override = default;

    NO_COPY_SEMANTIC(ETSNewMultiDimArrayInstanceExpression);
    NO_MOVE_SEMANTIC(ETSNewMultiDimArrayInstanceExpression);

    explicit ETSNewMultiDimArrayInstanceExpression(ir::TypeNode *const type_reference,
                                                   ArenaVector<ir::Expression *> &&dimensions)
        : Expression(AstNodeType::ETS_NEW_MULTI_DIM_ARRAY_INSTANCE_EXPRESSION),
          type_reference_(type_reference),
          dimensions_(std::move(dimensions))
    {
    }

    explicit ETSNewMultiDimArrayInstanceExpression(ETSNewMultiDimArrayInstanceExpression const &other,
                                                   ArenaAllocator *allocator);

    ir::TypeNode *TypeReference()
    {
        return type_reference_;
    }

    ir::TypeNode const *TypeReference() const
    {
        return type_reference_;
    }

    ArenaVector<ir::Expression *> &Dimensions()
    {
        return dimensions_;
    }

    ArenaVector<ir::Expression *> const &Dimension() const
    {
        return dimensions_;
    }

    [[nodiscard]] checker::Signature *Signature() noexcept
    {
        return signature_;
    }

    [[nodiscard]] const checker::Signature *Signature() const noexcept
    {
        return signature_;
    }

    // NOLINTNEXTLINE(google-default-arguments)
    [[nodiscard]] ETSNewMultiDimArrayInstanceExpression *Clone(ArenaAllocator *allocator,
                                                               AstNode *parent = nullptr) override;

    void TransformChildren(const NodeTransformer &cb) override;
    void Iterate(const NodeTraverser &cb) const override;

    void Dump(ir::AstDumper *dumper) const override;

    void Compile([[maybe_unused]] compiler::PandaGen *pg) const override;
    void Compile([[maybe_unused]] compiler::ETSGen *etsg) const override;
    checker::Type *Check([[maybe_unused]] checker::TSChecker *checker) override;
    checker::Type *Check([[maybe_unused]] checker::ETSChecker *checker) override;

private:
    ir::TypeNode *type_reference_;
    ArenaVector<ir::Expression *> dimensions_;
    checker::Signature *signature_ {};
};
}  // namespace panda::es2panda::ir

#endif
