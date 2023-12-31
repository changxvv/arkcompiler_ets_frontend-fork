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

#ifndef ES2PANDA_IR_EXPRESSION_ETS_PARAMETER_EXPRESSION_H
#define ES2PANDA_IR_EXPRESSION_ETS_PARAMETER_EXPRESSION_H

#include "ir/expression.h"

namespace panda::es2panda::checker {
class ETSAnalyzer;
}  // namespace panda::es2panda::checker

namespace panda::es2panda::ir {
// NOLINTBEGIN(modernize-avoid-c-arrays)
inline constexpr char const PROXY_PARAMETER_NAME[] = "$proxy_mask$";
// NOLINTEND(modernize-avoid-c-arrays)

class ETSParameterExpression final : public Expression {
public:
    ETSParameterExpression() = delete;
    ~ETSParameterExpression() override = default;

    NO_COPY_SEMANTIC(ETSParameterExpression);
    NO_MOVE_SEMANTIC(ETSParameterExpression);

    explicit ETSParameterExpression(AnnotatedExpression *ident_or_spread, Expression *initializer);

    // NOTE (csabahurton): friend relationship can be removed once there are getters for private fields
    friend class checker::ETSAnalyzer;

    [[nodiscard]] const Identifier *Ident() const noexcept;
    [[nodiscard]] Identifier *Ident() noexcept;

    [[nodiscard]] const SpreadElement *RestParameter() const noexcept;
    [[nodiscard]] SpreadElement *RestParameter() noexcept;

    [[nodiscard]] const Expression *Initializer() const noexcept;
    [[nodiscard]] Expression *Initializer() noexcept;

    void SetLexerSaved(util::StringView s) noexcept;
    [[nodiscard]] util::StringView LexerSaved() const noexcept;

    [[nodiscard]] varbinder::Variable *Variable() const noexcept;
    void SetVariable(varbinder::Variable *variable) noexcept;

    [[nodiscard]] TypeNode const *TypeAnnotation() const noexcept;
    [[nodiscard]] TypeNode *TypeAnnotation() noexcept;

    [[nodiscard]] bool IsDefault() const noexcept
    {
        return initializer_ != nullptr;
    }

    [[nodiscard]] bool IsRestParameter() const noexcept
    {
        return spread_ != nullptr;
    }

    [[nodiscard]] std::size_t GetRequiredParams() const noexcept
    {
        return extra_value_;
    }

    void SetRequiredParams(std::size_t const value) noexcept
    {
        extra_value_ = value;
    }

    // NOLINTNEXTLINE(google-default-arguments)
    [[nodiscard]] ETSParameterExpression *Clone(ArenaAllocator *allocator, AstNode *parent = nullptr) override;

    void Iterate(const NodeTraverser &cb) const override;
    void TransformChildren(const NodeTransformer &cb) override;
    void Dump(ir::AstDumper *dumper) const override;
    void Compile(compiler::PandaGen *pg) const override;
    void Compile(compiler::ETSGen *etsg) const override;
    checker::Type *Check(checker::TSChecker *checker) override;
    checker::Type *Check(checker::ETSChecker *checker) override;

private:
    Identifier *ident_;
    Expression *initializer_;
    SpreadElement *spread_ = nullptr;
    util::StringView saved_lexer_ = "";
    std::size_t extra_value_ = 0U;
};
}  // namespace panda::es2panda::ir

#endif
