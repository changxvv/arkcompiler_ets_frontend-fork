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

#ifndef ES2PANDA_IR_EXPRESSION_LITERAL_STRING_LITERAL_H
#define ES2PANDA_IR_EXPRESSION_LITERAL_STRING_LITERAL_H

#include "ir/expressions/literal.h"
#include "macros.h"
#include "util/ustring.h"

namespace panda::es2panda::ir {
class StringLiteral : public Literal {
public:
    ~StringLiteral() override = default;

    NO_COPY_SEMANTIC(StringLiteral);
    NO_MOVE_SEMANTIC(StringLiteral);

    explicit StringLiteral() : StringLiteral("") {}
    explicit StringLiteral(util::StringView str) : Literal(AstNodeType::STRING_LITERAL), str_(str) {}

    [[nodiscard]] const util::StringView &Str() const noexcept
    {
        return str_;
    }

    bool operator==(const StringLiteral &other) const
    {
        return str_ == other.str_;
    }

    // NOLINTNEXTLINE(google-default-arguments)
    [[nodiscard]] StringLiteral *Clone(ArenaAllocator *allocator, AstNode *parent = nullptr) override;

    void TransformChildren(const NodeTransformer &cb) override;
    void Iterate(const NodeTraverser &cb) const override;
    void Dump(ir::AstDumper *dumper) const override;
    void Compile(compiler::PandaGen *pg) const override;
    void Compile(compiler::ETSGen *etsg) const override;
    checker::Type *Check(checker::TSChecker *checker) override;
    checker::Type *Check([[maybe_unused]] checker::ETSChecker *checker) override;

private:
    util::StringView str_;
};
}  // namespace panda::es2panda::ir

#endif
