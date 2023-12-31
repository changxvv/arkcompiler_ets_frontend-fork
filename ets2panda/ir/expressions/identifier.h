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

#ifndef ES2PANDA_IR_EXPRESSION_IDENTIFIER_H
#define ES2PANDA_IR_EXPRESSION_IDENTIFIER_H

#include "ir/expression.h"
#include "util/ustring.h"
#include "ir/validationInfo.h"

namespace panda::es2panda::varbinder {
class Variable;
}  // namespace panda::es2panda::varbinder

namespace panda::es2panda::ir {
enum class IdentifierFlags : uint32_t {
    NONE = 0U,
    OPTIONAL = 1U << 0U,
    REFERENCE = 1U << 1U,
    TDZ = 1U << 2U,
    PRIVATE = 1U << 3U,
    GET = 1U << 4U,
    SET = 1U << 5U,
    IGNORE_BOX = 1U << 6U,
};

DEFINE_BITOPS(IdentifierFlags)

class Identifier : public AnnotatedExpression {
private:
    struct Tag {};

public:
    Identifier() = delete;
    ~Identifier() override = default;

    NO_COPY_SEMANTIC(Identifier);
    NO_MOVE_SEMANTIC(Identifier);

public:
    explicit Identifier(ArenaAllocator *const allocator) : Identifier("", allocator) {}
    explicit Identifier(util::StringView const name, ArenaAllocator *const allocator)
        : AnnotatedExpression(AstNodeType::IDENTIFIER), name_(name), decorators_(allocator->Adapter())
    {
    }

    explicit Identifier(util::StringView const name, TypeNode *const type_annotation, ArenaAllocator *const allocator)
        : AnnotatedExpression(AstNodeType::IDENTIFIER, type_annotation), name_(name), decorators_(allocator->Adapter())
    {
    }

    explicit Identifier(Tag tag, Identifier const &other, ArenaAllocator *allocator);

    [[nodiscard]] const util::StringView &Name() const noexcept
    {
        return name_;
    }

    [[nodiscard]] util::StringView &Name() noexcept
    {
        return name_;
    }

    void SetName(const util::StringView &new_name) noexcept
    {
        name_ = new_name;
    }

    [[nodiscard]] const ArenaVector<Decorator *> &Decorators() const noexcept
    {
        return decorators_;
    }

    const ArenaVector<Decorator *> *DecoratorsPtr() const override
    {
        return &Decorators();
    }

    [[nodiscard]] bool IsOptional() const noexcept
    {
        return (flags_ & IdentifierFlags::OPTIONAL) != 0;
    }

    void SetOptional(bool const optional) noexcept
    {
        if (optional) {
            flags_ |= IdentifierFlags::OPTIONAL;
        } else {
            flags_ &= ~IdentifierFlags::OPTIONAL;
        }
    }

    [[nodiscard]] bool IsReference() const noexcept
    {
        return (flags_ & IdentifierFlags::REFERENCE) != 0;
    }

    void SetReference(bool const is_reference = true) noexcept
    {
        if (is_reference) {
            flags_ |= IdentifierFlags::REFERENCE;
        } else {
            flags_ &= ~IdentifierFlags::REFERENCE;
        }
    }

    [[nodiscard]] bool IsTdz() const noexcept
    {
        return (flags_ & IdentifierFlags::TDZ) != 0;
    }

    void SetTdz() noexcept
    {
        flags_ |= IdentifierFlags::TDZ;
    }

    void SetAccessor() noexcept
    {
        flags_ |= IdentifierFlags::GET;
    }

    [[nodiscard]] bool IsAccessor() const noexcept
    {
        return (flags_ & IdentifierFlags::GET) != 0;
    }

    void SetMutator() noexcept
    {
        flags_ |= IdentifierFlags::SET;
    }

    [[nodiscard]] bool IsMutator() const noexcept
    {
        return (flags_ & IdentifierFlags::SET) != 0;
    }

    [[nodiscard]] bool IsPrivateIdent() const noexcept
    {
        return (flags_ & IdentifierFlags::PRIVATE) != 0;
    }

    void SetPrivate(bool const is_private) noexcept
    {
        if (is_private) {
            flags_ |= IdentifierFlags::PRIVATE;
        } else {
            flags_ &= ~IdentifierFlags::PRIVATE;
        }
    }

    [[nodiscard]] bool IsIgnoreBox() const noexcept
    {
        return (flags_ & IdentifierFlags::IGNORE_BOX) != 0;
    }

    void SetIgnoreBox() noexcept
    {
        flags_ |= IdentifierFlags::IGNORE_BOX;
    }

    [[nodiscard]] varbinder::Variable *Variable() const noexcept
    {
        return variable_;
    }

    void SetVariable(varbinder::Variable *const variable) noexcept
    {
        variable_ = variable;
    }

    [[nodiscard]] varbinder::Variable *Variable() noexcept
    {
        return variable_;
    }

    void AddDecorators([[maybe_unused]] ArenaVector<ir::Decorator *> &&decorators) override
    {
        decorators_ = std::move(decorators);
    }

    // NOLINTNEXTLINE(google-default-arguments)
    [[nodiscard]] Identifier *Clone(ArenaAllocator *allocator, AstNode *parent = nullptr) override;

    bool CanHaveDecorator([[maybe_unused]] bool in_ts) const override
    {
        return true;
    }

    [[nodiscard]] ValidationInfo ValidateExpression();

    void TransformChildren(const NodeTransformer &cb) override;
    void Iterate(const NodeTraverser &cb) const override;
    void Dump(ir::AstDumper *dumper) const override;
    void Compile([[maybe_unused]] compiler::PandaGen *pg) const override;
    void Compile([[maybe_unused]] compiler::ETSGen *etsg) const override;
    checker::Type *Check([[maybe_unused]] checker::TSChecker *checker) override;
    checker::Type *Check([[maybe_unused]] checker::ETSChecker *checker) override;

private:
    util::StringView name_;
    IdentifierFlags flags_ {IdentifierFlags::NONE};
    ArenaVector<Decorator *> decorators_;
    varbinder::Variable *variable_ {};
};
}  // namespace panda::es2panda::ir

#endif
