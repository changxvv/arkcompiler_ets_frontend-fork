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

#ifndef ES2PANDA_PARSER_INCLUDE_AST_TS_INDEX_SIGNATURE_H
#define ES2PANDA_PARSER_INCLUDE_AST_TS_INDEX_SIGNATURE_H

#include "ir/typeNode.h"

namespace panda::es2panda::checker {
class TSAnalyzer;
}  // namespace panda::es2panda::checker

namespace panda::es2panda::ir {
class TSIndexSignature : public TypedAstNode {
public:
    enum class TSIndexSignatureKind { NUMBER, STRING };

    TSIndexSignature() = delete;
    ~TSIndexSignature() override = default;

    NO_COPY_SEMANTIC(TSIndexSignature);
    NO_MOVE_SEMANTIC(TSIndexSignature);

    explicit TSIndexSignature(Expression *const param, TypeNode *const type_annotation, bool const readonly)
        : TypedAstNode(AstNodeType::TS_INDEX_SIGNATURE),
          param_(param),
          type_annotation_(type_annotation),
          readonly_(readonly)
    {
    }
    // NOTE (csabahurton): friend relationship can be removed once there are getters for private fields
    friend class checker::TSAnalyzer;

    [[nodiscard]] const Expression *Param() const noexcept
    {
        return param_;
    }

    [[nodiscard]] const TypeNode *TypeAnnotation() const noexcept
    {
        return type_annotation_;
    }

    [[nodiscard]] bool Readonly() const noexcept
    {
        return readonly_;
    }

    [[nodiscard]] TSIndexSignatureKind Kind() const noexcept;

    // NOLINTNEXTLINE(google-default-arguments)
    [[nodiscard]] TSIndexSignature *Clone(ArenaAllocator *allocator, AstNode *parent = nullptr) override;

    void TransformChildren(const NodeTransformer &cb) override;
    void Iterate(const NodeTraverser &cb) const override;

    void Dump(ir::AstDumper *dumper) const override;
    void Compile(compiler::PandaGen *pg) const override;
    void Compile(compiler::ETSGen *etsg) const override;
    checker::Type *Check(checker::TSChecker *checker) override;
    checker::Type *Check(checker::ETSChecker *checker) override;

private:
    Expression *param_;
    TypeNode *type_annotation_;
    bool readonly_;
};
}  // namespace panda::es2panda::ir

#endif /* TS_INDEX_SIGNATURE_H */
