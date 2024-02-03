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

#ifndef ES2PANDA_PARSER_INCLUDE_AST_METHOD_DEFINITION_H
#define ES2PANDA_PARSER_INCLUDE_AST_METHOD_DEFINITION_H

#include "ir/base/classElement.h"

namespace ark::es2panda::checker {
class ETSAnalyzer;
}  // namespace ark::es2panda::checker

namespace ark::es2panda::ir {

class Expression;
class ScriptFunction;

enum class MethodDefinitionKind { NONE, CONSTRUCTOR, METHOD, EXTENSION_METHOD, GET, SET };

class MethodDefinition : public ClassElement {
public:
    MethodDefinition() = delete;
    ~MethodDefinition() override = default;

    NO_COPY_SEMANTIC(MethodDefinition);
    NO_MOVE_SEMANTIC(MethodDefinition);

    using OverloadsT = ArenaVector<MethodDefinition *>;

    explicit MethodDefinition(MethodDefinitionKind const kind, Expression *const key, Expression *const value,
                              ModifierFlags const modifiers, ArenaAllocator *const allocator, bool const isComputed)
        : ClassElement(AstNodeType::METHOD_DEFINITION, key, value, modifiers, allocator, isComputed),
          kind_(kind),
          overloads_(allocator->Adapter())
    {
        ASSERT(key_ != nullptr && value_ != nullptr);
    }

    // NOTE (csabahurton): these friend relationships can be removed once there are getters for private fields
    friend class checker::ETSAnalyzer;

    MethodDefinitionKind Kind() const
    {
        return kind_;
    }

    [[nodiscard]] bool IsConstructor() const noexcept
    {
        return kind_ == MethodDefinitionKind::CONSTRUCTOR;
    }

    [[nodiscard]] bool IsExtensionMethod() const noexcept
    {
        return kind_ == MethodDefinitionKind::EXTENSION_METHOD;
    }

    [[nodiscard]] const OverloadsT &Overloads() const noexcept
    {
        return overloads_;
    }

    void SetOverloads(OverloadsT &&overloads)
    {
        overloads_ = std::move(overloads);
    }

    void ClearOverloads()
    {
        overloads_.clear();
    }

    void AddOverload(MethodDefinition *const overload)
    {
        overloads_.emplace_back(overload);
    }

    [[nodiscard]] bool HasOverload(MethodDefinition *overload) noexcept
    {
        return std::find(overloads_.begin(), overloads_.end(), overload) != overloads_.end();
    }

    ScriptFunction *Function();
    const ScriptFunction *Function() const;
    PrivateFieldKind ToPrivateFieldKind(bool isStatic) const override;

    [[nodiscard]] MethodDefinition *Clone(ArenaAllocator *allocator, AstNode *parent) override;

    void TransformChildren(const NodeTransformer &cb) override;
    void Iterate(const NodeTraverser &cb) const override;

    void ResolveReferences(const NodeTraverser &cb) const;

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
    MethodDefinitionKind kind_;
    OverloadsT overloads_;
};
}  // namespace ark::es2panda::ir

#endif
