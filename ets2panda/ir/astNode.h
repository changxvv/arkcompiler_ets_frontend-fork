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

#ifndef ES2PANDA_IR_AST_NODE_H
#define ES2PANDA_IR_AST_NODE_H

#include "ir/astNodeFlags.h"
#include "ir/astNodeMapping.h"
#include "lexer/token/sourceLocation.h"
#include "util/enumbitops.h"

#include <functional>
#include "macros.h"

namespace panda::es2panda::compiler {
class PandaGen;
class ETSGen;
}  // namespace panda::es2panda::compiler

namespace panda::es2panda::checker {
class TSChecker;
class ETSChecker;
class Type;
}  // namespace panda::es2panda::checker

namespace panda::es2panda::varbinder {
class Variable;
class Scope;
}  // namespace panda::es2panda::varbinder

namespace panda::es2panda::ir {
// NOLINTBEGIN(modernize-avoid-c-arrays)
inline constexpr char const CLONE_ALLOCATION_ERROR[] = "Unsuccessful allocation during cloning.";
// NOLINTEND(modernize-avoid-c-arrays)

class AstNode;
class TypeNode;

using NodeTransformer = std::function<AstNode *(AstNode *)>;
using NodeTraverser = std::function<void(AstNode *)>;
using NodePredicate = std::function<bool(AstNode *)>;

enum class AstNodeType {
// NOLINTNEXTLINE(cppcoreguidelines-macro-usage)
#define DECLARE_NODE_TYPES(nodeType, className) nodeType,
    AST_NODE_MAPPING(DECLARE_NODE_TYPES)
#undef DECLARE_NODE_TYPES
// NOLINTNEXTLINE(cppcoreguidelines-macro-usage)
#define DECLARE_NODE_TYPES(nodeType1, nodeType2, baseClass, reinterpretClass) nodeType1, nodeType2,
        AST_NODE_REINTERPRET_MAPPING(DECLARE_NODE_TYPES)
#undef DECLARE_NODE_TYPES
};

DEFINE_BITOPS(AstNodeFlags)

DEFINE_BITOPS(ModifierFlags)

DEFINE_BITOPS(ScriptFunctionFlags)

DEFINE_BITOPS(BoxingUnboxingFlags)

// Forward declarations
class AstDumper;
class Expression;
class Statement;
class ClassElement;

// NOLINTNEXTLINE(cppcoreguidelines-macro-usage)
#define DECLARE_CLASSES(nodeType, className) class className;
AST_NODE_MAPPING(DECLARE_CLASSES)
#undef DECLARE_CLASSES

// NOLINTNEXTLINE(cppcoreguidelines-macro-usage)
#define DECLARE_CLASSES(nodeType1, nodeType2, baseClass, reinterpretClass) class baseClass;
AST_NODE_REINTERPRET_MAPPING(DECLARE_CLASSES)
#undef DECLARE_CLASSES

class AstNode {
public:
    explicit AstNode(AstNodeType type) : type_(type) {};
    explicit AstNode(AstNodeType type, ModifierFlags flags) : type_(type), flags_(flags) {};
    virtual ~AstNode() = default;

    AstNode() = delete;
    NO_COPY_OPERATOR(AstNode);
    NO_MOVE_SEMANTIC(AstNode);

    bool IsProgram() const
    {
        return parent_ == nullptr;
    }

// NOLINTNEXTLINE(cppcoreguidelines-macro-usage)
#define DECLARE_IS_CHECKS(nodeType, className) \
    bool Is##className() const                 \
    {                                          \
        return type_ == AstNodeType::nodeType; \
    }
    AST_NODE_MAPPING(DECLARE_IS_CHECKS)
#undef DECLARE_IS_CHECKS

// NOLINTNEXTLINE(cppcoreguidelines-macro-usage)
#define DECLARE_IS_CHECKS(nodeType1, nodeType2, baseClass, reinterpretClass) \
    bool Is##baseClass() const                                               \
    {                                                                        \
        return type_ == AstNodeType::nodeType1;                              \
    }                                                                        \
    bool Is##reinterpretClass() const                                        \
    {                                                                        \
        return type_ == AstNodeType::nodeType2;                              \
    }
    AST_NODE_REINTERPRET_MAPPING(DECLARE_IS_CHECKS)
#undef DECLARE_IS_CHECKS

    [[nodiscard]] virtual bool IsStatement() const noexcept
    {
        return false;
    }

    [[nodiscard]] virtual bool IsExpression() const noexcept
    {
        return false;
    }

    virtual bool IsTyped() const
    {
        return false;
    }

// NOLINTNEXTLINE(cppcoreguidelines-macro-usage)
#define DECLARE_AS_CASTS(nodeType, className)             \
    className *As##className()                            \
    {                                                     \
        ASSERT(Is##className());                          \
        return reinterpret_cast<className *>(this);       \
    }                                                     \
    const className *As##className() const                \
    {                                                     \
        ASSERT(Is##className());                          \
        return reinterpret_cast<const className *>(this); \
    }
    AST_NODE_MAPPING(DECLARE_AS_CASTS)
#undef DECLARE_AS_CASTS

// NOLINTNEXTLINE(cppcoreguidelines-macro-usage)
#define DECLARE_AS_CASTS(nodeType1, nodeType2, baseClass, reinterpretClass) \
    baseClass *As##baseClass()                                              \
    {                                                                       \
        ASSERT(Is##baseClass());                                            \
        return reinterpret_cast<baseClass *>(this);                         \
    }                                                                       \
    baseClass *As##reinterpretClass()                                       \
    {                                                                       \
        ASSERT(Is##reinterpretClass());                                     \
        return reinterpret_cast<baseClass *>(this);                         \
    }                                                                       \
    const baseClass *As##baseClass() const                                  \
    {                                                                       \
        ASSERT(Is##baseClass());                                            \
        return reinterpret_cast<const baseClass *>(this);                   \
    }                                                                       \
    const baseClass *As##reinterpretClass() const                           \
    {                                                                       \
        ASSERT(Is##reinterpretClass());                                     \
        return reinterpret_cast<const baseClass *>(this);                   \
    }
    AST_NODE_REINTERPRET_MAPPING(DECLARE_AS_CASTS)
#undef DECLARE_AS_CASTS

    Expression *AsExpression()
    {
        ASSERT(IsExpression());
        return reinterpret_cast<Expression *>(this);
    }

    const Expression *AsExpression() const
    {
        ASSERT(IsExpression());
        return reinterpret_cast<const Expression *>(this);
    }

    Statement *AsStatement()
    {
        ASSERT(IsStatement());
        return reinterpret_cast<Statement *>(this);
    }

    const Statement *AsStatement() const
    {
        ASSERT(IsStatement());
        return reinterpret_cast<const Statement *>(this);
    }

    void SetRange(const lexer::SourceRange &loc) noexcept
    {
        range_ = loc;
    }

    void SetStart(const lexer::SourcePosition &start) noexcept
    {
        range_.start = start;
    }

    void SetEnd(const lexer::SourcePosition &end) noexcept
    {
        range_.end = end;
    }

    [[nodiscard]] const lexer::SourcePosition &Start() const noexcept
    {
        return range_.start;
    }

    [[nodiscard]] const lexer::SourcePosition &End() const noexcept
    {
        return range_.end;
    }

    [[nodiscard]] const lexer::SourceRange &Range() const noexcept
    {
        return range_;
    }

    [[nodiscard]] AstNodeType Type() const noexcept
    {
        return type_;
    }

    [[nodiscard]] AstNode *Parent() noexcept
    {
        return parent_;
    }

    [[nodiscard]] const AstNode *Parent() const noexcept
    {
        return parent_;
    }

    void SetParent(AstNode *const parent) noexcept
    {
        parent_ = parent;
    }

    [[nodiscard]] varbinder::Variable *Variable() const noexcept
    {
        return variable_;
    }

    void SetVariable(varbinder::Variable *const variable) noexcept
    {
        variable_ = variable;
    }

    // When no decorators are allowed, we cannot return a reference to an empty vector.
    virtual const ArenaVector<ir::Decorator *> *DecoratorsPtr() const
    {
        return nullptr;
    }

    virtual void AddDecorators([[maybe_unused]] ArenaVector<ir::Decorator *> &&decorators)
    {
        UNREACHABLE();
    }

    virtual bool CanHaveDecorator([[maybe_unused]] bool in_ts) const
    {
        return false;
    }

    [[nodiscard]] bool IsReadonly() const noexcept
    {
        return (flags_ & ModifierFlags::READONLY) != 0;
    }

    [[nodiscard]] bool IsOptional() const noexcept
    {
        return (flags_ & ModifierFlags::OPTIONAL) != 0;
    }

    [[nodiscard]] bool IsDefinite() const noexcept
    {
        return (flags_ & ModifierFlags::DEFINITE) != 0;
    }

    [[nodiscard]] bool IsConstructor() const noexcept
    {
        return (flags_ & ModifierFlags::CONSTRUCTOR) != 0;
    }

    [[nodiscard]] bool IsOverride() const noexcept
    {
        return (flags_ & ModifierFlags::OVERRIDE) != 0;
    }

    [[nodiscard]] bool IsAsync() const noexcept
    {
        return (flags_ & ModifierFlags::ASYNC) != 0;
    }

    [[nodiscard]] bool IsSynchronized() const noexcept
    {
        return (flags_ & ModifierFlags::SYNCHRONIZED) != 0;
    }

    [[nodiscard]] bool IsNative() const noexcept
    {
        return (flags_ & ModifierFlags::NATIVE) != 0;
    }

    [[nodiscard]] bool IsNullAssignable() const noexcept
    {
        return (flags_ & ModifierFlags::NULL_ASSIGNABLE) != 0;
    }

    [[nodiscard]] bool IsUndefinedAssignable() const noexcept
    {
        return (flags_ & ModifierFlags::UNDEFINED_ASSIGNABLE) != 0;
    }

    [[nodiscard]] bool IsConst() const noexcept
    {
        return (flags_ & ModifierFlags::CONST) != 0;
    }

    [[nodiscard]] bool IsStatic() const noexcept
    {
        return (flags_ & ModifierFlags::STATIC) != 0;
    }

    [[nodiscard]] bool IsFinal() const noexcept
    {
        return (flags_ & ModifierFlags::FINAL) != 0U;
    }

    [[nodiscard]] bool IsAbstract() const noexcept
    {
        return (flags_ & ModifierFlags::ABSTRACT) != 0;
    }

    [[nodiscard]] bool IsPublic() const noexcept
    {
        return (flags_ & ModifierFlags::PUBLIC) != 0;
    }

    [[nodiscard]] bool IsProtected() const noexcept
    {
        return (flags_ & ModifierFlags::PROTECTED) != 0;
    }

    [[nodiscard]] bool IsPrivate() const noexcept
    {
        return (flags_ & ModifierFlags::PRIVATE) != 0;
    }

    [[nodiscard]] bool IsInternal() const noexcept
    {
        return (flags_ & ModifierFlags::INTERNAL) != 0;
    }

    [[nodiscard]] bool IsExported() const noexcept
    {
        return (flags_ & ModifierFlags::EXPORT) != 0;
    }

    [[nodiscard]] bool IsDefaultExported() const noexcept
    {
        return (flags_ & ModifierFlags::DEFAULT_EXPORT) != 0;
    }

    [[nodiscard]] bool IsDeclare() const noexcept
    {
        return (flags_ & ModifierFlags::DECLARE) != 0;
    }

    [[nodiscard]] bool IsIn() const noexcept
    {
        return (flags_ & ModifierFlags::IN) != 0;
    }

    [[nodiscard]] bool IsOut() const noexcept
    {
        return (flags_ & ModifierFlags::OUT) != 0;
    }

    [[nodiscard]] bool IsSetter() const noexcept
    {
        return (flags_ & ModifierFlags::SETTER) != 0;
    }

    void AddModifier(ModifierFlags const flags) noexcept
    {
        flags_ |= flags;
    }

    [[nodiscard]] ModifierFlags Modifiers() noexcept
    {
        return flags_;
    }

    [[nodiscard]] ModifierFlags Modifiers() const noexcept
    {
        return flags_;
    }

    void SetBoxingUnboxingFlags(BoxingUnboxingFlags const flags) const noexcept
    {
        boxing_unboxing_flags_ = flags;
    }

    void AddBoxingUnboxingFlag(BoxingUnboxingFlags const flag) const noexcept
    {
        boxing_unboxing_flags_ |= flag;
    }

    [[nodiscard]] BoxingUnboxingFlags GetBoxingUnboxingFlags() const noexcept
    {
        return boxing_unboxing_flags_;
    }

    ir::ClassElement *AsClassElement()
    {
        ASSERT(IsMethodDefinition() || IsClassProperty() || IsClassStaticBlock());
        return reinterpret_cast<ir::ClassElement *>(this);
    }

    const ir::ClassElement *AsClassElement() const
    {
        ASSERT(IsMethodDefinition() || IsClassProperty() || IsClassStaticBlock());
        return reinterpret_cast<const ir::ClassElement *>(this);
    }

    [[nodiscard]] virtual bool IsScopeBearer() const
    {
        return false;
    }

    virtual varbinder::Scope *Scope() const
    {
        UNREACHABLE();
    }

    [[nodiscard]] ir::BlockStatement *GetTopStatement();
    [[nodiscard]] const ir::BlockStatement *GetTopStatement() const;

    // NOLINTNEXTLINE(google-default-arguments)
    [[nodiscard]] virtual AstNode *Clone([[maybe_unused]] ArenaAllocator *const allocator,
                                         [[maybe_unused]] AstNode *const parent = nullptr)
    {
        UNREACHABLE();
    }

    virtual void TransformChildren(const NodeTransformer &cb) = 0;
    virtual void Iterate(const NodeTraverser &cb) const = 0;
    void TransformChildrenRecursively(const NodeTransformer &cb);
    void IterateRecursively(const NodeTraverser &cb) const;
    bool IsAnyChild(const NodePredicate &cb) const;
    AstNode *FindChild(const NodePredicate &cb) const;

    std::string DumpJSON() const;

    virtual void Dump(ir::AstDumper *dumper) const = 0;
    virtual void Compile([[maybe_unused]] compiler::PandaGen *pg) const = 0;
    virtual void Compile([[maybe_unused]] compiler::ETSGen *etsg) const {};
    virtual checker::Type *Check([[maybe_unused]] checker::TSChecker *checker) = 0;
    virtual checker::Type *Check([[maybe_unused]] checker::ETSChecker *checker) = 0;

protected:
    AstNode(AstNode const &other);

    void SetType(AstNodeType const type) noexcept
    {
        type_ = type;
    }

    // NOLINTBEGIN(misc-non-private-member-variables-in-classes)
    AstNode *parent_ {};
    lexer::SourceRange range_ {};
    AstNodeType type_;
    varbinder::Variable *variable_ {};
    ModifierFlags flags_ {};
    mutable BoxingUnboxingFlags boxing_unboxing_flags_ {};
    // NOLINTEND(misc-non-private-member-variables-in-classes)
};

template <typename T>
class Typed : public T {
public:
    Typed() = delete;
    ~Typed() override = default;

    NO_COPY_OPERATOR(Typed);
    NO_MOVE_SEMANTIC(Typed);

    [[nodiscard]] checker::Type *TsType() noexcept
    {
        return ts_type_;
    }

    [[nodiscard]] const checker::Type *TsType() const noexcept
    {
        return ts_type_;
    }

    void SetTsType(checker::Type *ts_type) noexcept
    {
        ts_type_ = ts_type;
    }

    bool IsTyped() const override
    {
        return true;
    }

protected:
    explicit Typed(AstNodeType const type) : T(type) {}
    explicit Typed(AstNodeType const type, ModifierFlags const flags) : T(type, flags) {}

    // NOTE: when cloning node its type is not copied but removed empty so that it can be re-checked further.
    Typed(Typed const &other) : T(static_cast<T const &>(other)) {}

private:
    checker::Type *ts_type_ {};
};

template <typename T>
class Annotated : public T {
public:
    Annotated() = delete;
    ~Annotated() override = default;

    NO_COPY_OPERATOR(Annotated);
    NO_MOVE_SEMANTIC(Annotated);

    [[nodiscard]] TypeNode *TypeAnnotation() const noexcept
    {
        return type_annotation_;
    }

    void SetTsTypeAnnotation(TypeNode *const type_annotation) noexcept
    {
        type_annotation_ = type_annotation;
    }

protected:
    explicit Annotated(AstNodeType const type, TypeNode *const type_annotation)
        : T(type), type_annotation_(type_annotation)
    {
    }
    explicit Annotated(AstNodeType const type) : T(type) {}
    explicit Annotated(AstNodeType const type, ModifierFlags const flags) : T(type, flags) {}

    Annotated(Annotated const &other) : T(static_cast<T const &>(other)) {}

private:
    TypeNode *type_annotation_ {};
};

class TypedAstNode : public Typed<AstNode> {
public:
    TypedAstNode() = delete;
    ~TypedAstNode() override = default;

    NO_COPY_OPERATOR(TypedAstNode);
    NO_MOVE_SEMANTIC(TypedAstNode);

protected:
    explicit TypedAstNode(AstNodeType const type) : Typed<AstNode>(type) {}
    explicit TypedAstNode(AstNodeType const type, ModifierFlags const flags) : Typed<AstNode>(type, flags) {}

    TypedAstNode(TypedAstNode const &other) : Typed<AstNode>(static_cast<Typed<AstNode> const &>(other)) {}
};

class AnnotatedAstNode : public Annotated<AstNode> {
public:
    AnnotatedAstNode() = delete;
    ~AnnotatedAstNode() override = default;

    NO_COPY_OPERATOR(AnnotatedAstNode);
    NO_MOVE_SEMANTIC(AnnotatedAstNode);

protected:
    explicit AnnotatedAstNode(AstNodeType const type, TypeNode *const type_annotation)
        : Annotated<AstNode>(type, type_annotation)
    {
    }
    explicit AnnotatedAstNode(AstNodeType const type) : Annotated<AstNode>(type) {}
    explicit AnnotatedAstNode(AstNodeType const type, ModifierFlags const flags) : Annotated<AstNode>(type, flags) {}

    AnnotatedAstNode(AnnotatedAstNode const &other) : Annotated<AstNode>(static_cast<Annotated<AstNode> const &>(other))
    {
    }
};
}  // namespace panda::es2panda::ir
#endif
