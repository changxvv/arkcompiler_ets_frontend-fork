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

#include "tsEnumDeclaration.h"

#include "checker/TSchecker.h"
#include "compiler/core/ETSGen.h"
#include "compiler/core/pandagen.h"
#include "varbinder/scope.h"
#include "util/helpers.h"

namespace panda::es2panda::ir {
void TSEnumDeclaration::TransformChildren(const NodeTransformer &cb)
{
    for (auto *&it : decorators_) {
        it = cb(it)->AsDecorator();
    }

    key_ = cb(key_)->AsIdentifier();

    for (auto *&it : members_) {
        it = cb(it);
    }
}

void TSEnumDeclaration::Iterate(const NodeTraverser &cb) const
{
    for (auto *it : decorators_) {
        cb(it);
    }

    cb(key_);

    for (auto *it : members_) {
        cb(it);
    }
}

void TSEnumDeclaration::Dump(ir::AstDumper *dumper) const
{
    dumper->Add({{"type", "TSEnumDeclaration"},
                 {"decorators", AstDumper::Optional(decorators_)},
                 {"id", key_},
                 {"members", members_},
                 {"const", is_const_}});
}

int32_t ToInt(double num)
{
    if (num >= std::numeric_limits<int32_t>::min() && num <= std::numeric_limits<int32_t>::max()) {
        return static_cast<int32_t>(num);
    }

    // NOTE: aszilagyi. Perform ECMA defined toInt conversion

    return 0;
}

uint32_t ToUInt(double num)
{
    if (num >= std::numeric_limits<uint32_t>::min() && num <= std::numeric_limits<uint32_t>::max()) {
        return static_cast<int32_t>(num);
    }

    // NOTE: aszilagyi. Perform ECMA defined toInt conversion

    return 0;
}

varbinder::EnumMemberResult EvaluateIdentifier(checker::TSChecker *checker, varbinder::EnumVariable *enum_var,
                                               const ir::Identifier *expr)
{
    if (expr->Name() == "NaN") {
        return std::nan("");
    }
    if (expr->Name() == "Infinity") {
        return std::numeric_limits<double>::infinity();
    }

    varbinder::Variable *enum_member = expr->AsIdentifier()->Variable();

    if (enum_member == nullptr) {
        checker->ThrowTypeError({"Cannot find name ", expr->AsIdentifier()->Name()},
                                enum_var->Declaration()->Node()->Start());
    }

    if (enum_member->IsEnumVariable()) {
        varbinder::EnumVariable *expr_enum_var = enum_member->AsEnumVariable();
        if (std::holds_alternative<bool>(expr_enum_var->Value())) {
            checker->ThrowTypeError(
                "A member initializer in a enum declaration cannot reference members declared after it, "
                "including "
                "members defined in other enums.",
                enum_var->Declaration()->Node()->Start());
        }

        return expr_enum_var->Value();
    }

    return false;
}

varbinder::EnumMemberResult EvaluateUnaryExpression(checker::TSChecker *checker, varbinder::EnumVariable *enum_var,
                                                    const ir::UnaryExpression *expr)
{
    varbinder::EnumMemberResult value = TSEnumDeclaration::EvaluateEnumMember(checker, enum_var, expr->Argument());
    if (!std::holds_alternative<double>(value)) {
        return false;
    }

    switch (expr->OperatorType()) {
        case lexer::TokenType::PUNCTUATOR_PLUS: {
            return std::get<double>(value);
        }
        case lexer::TokenType::PUNCTUATOR_MINUS: {
            return -std::get<double>(value);
        }
        case lexer::TokenType::PUNCTUATOR_TILDE: {
            return static_cast<double>(~ToInt(std::get<double>(value)));  // NOLINT(hicpp-signed-bitwise)
        }
        default: {
            break;
        }
    }

    return false;
}

varbinder::EnumMemberResult EvaluateMemberExpression(checker::TSChecker *checker,
                                                     [[maybe_unused]] varbinder::EnumVariable *enum_var,
                                                     ir::MemberExpression *expr)
{
    if (checker::TSChecker::IsConstantMemberAccess(expr->AsExpression())) {
        if (expr->Check(checker)->TypeFlags() == checker::TypeFlag::ENUM) {
            util::StringView name;
            if (!expr->IsComputed()) {
                name = expr->Property()->AsIdentifier()->Name();
            } else {
                ASSERT(checker::TSChecker::IsStringLike(expr->Property()));
                name = reinterpret_cast<const ir::StringLiteral *>(expr->Property())->Str();
            }

            // NOTE: aszilagyi.
        }
    }

    return false;
}

varbinder::EnumMemberResult EvaluateBinaryExpression(checker::TSChecker *checker, varbinder::EnumVariable *enum_var,
                                                     const ir::BinaryExpression *expr)
{
    varbinder::EnumMemberResult left =
        TSEnumDeclaration::EvaluateEnumMember(checker, enum_var, expr->AsBinaryExpression()->Left());
    varbinder::EnumMemberResult right =
        TSEnumDeclaration::EvaluateEnumMember(checker, enum_var, expr->AsBinaryExpression()->Right());
    if (std::holds_alternative<double>(left) && std::holds_alternative<double>(right)) {
        switch (expr->AsBinaryExpression()->OperatorType()) {
            case lexer::TokenType::PUNCTUATOR_BITWISE_OR: {
                return static_cast<double>(ToUInt(std::get<double>(left)) | ToUInt(std::get<double>(right)));
            }
            case lexer::TokenType::PUNCTUATOR_BITWISE_AND: {
                return static_cast<double>(ToUInt(std::get<double>(left)) & ToUInt(std::get<double>(right)));
            }
            case lexer::TokenType::PUNCTUATOR_BITWISE_XOR: {
                return static_cast<double>(ToUInt(std::get<double>(left)) ^ ToUInt(std::get<double>(right)));
            }
            case lexer::TokenType::PUNCTUATOR_LEFT_SHIFT: {  // NOLINTNEXTLINE(hicpp-signed-bitwise)
                return static_cast<double>(ToInt(std::get<double>(left)) << ToUInt(std::get<double>(right)));
            }
            case lexer::TokenType::PUNCTUATOR_RIGHT_SHIFT: {  // NOLINTNEXTLINE(hicpp-signed-bitwise)
                return static_cast<double>(ToInt(std::get<double>(left)) >> ToUInt(std::get<double>(right)));
            }
            case lexer::TokenType::PUNCTUATOR_UNSIGNED_RIGHT_SHIFT: {
                return static_cast<double>(ToUInt(std::get<double>(left)) >> ToUInt(std::get<double>(right)));
            }
            case lexer::TokenType::PUNCTUATOR_PLUS: {
                return std::get<double>(left) + std::get<double>(right);
            }
            case lexer::TokenType::PUNCTUATOR_MINUS: {
                return std::get<double>(left) - std::get<double>(right);
            }
            case lexer::TokenType::PUNCTUATOR_MULTIPLY: {
                return std::get<double>(left) * std::get<double>(right);
            }
            case lexer::TokenType::PUNCTUATOR_DIVIDE: {
                return std::get<double>(left) / std::get<double>(right);
            }
            case lexer::TokenType::PUNCTUATOR_MOD: {
                return std::fmod(std::get<double>(left), std::get<double>(right));
            }
            case lexer::TokenType::PUNCTUATOR_EXPONENTIATION: {
                return std::pow(std::get<double>(left), std::get<double>(right));
            }
            default: {
                break;
            }
        }

        return false;
    }

    if (std::holds_alternative<util::StringView>(left) && std::holds_alternative<util::StringView>(right) &&
        expr->AsBinaryExpression()->OperatorType() == lexer::TokenType::PUNCTUATOR_PLUS) {
        std::stringstream ss;
        ss << std::get<util::StringView>(left) << std::get<util::StringView>(right);

        util::UString res(ss.str(), checker->Allocator());
        return res.View();
    }

    return false;
}

varbinder::EnumMemberResult TSEnumDeclaration::EvaluateEnumMember(checker::TSChecker *checker,
                                                                  varbinder::EnumVariable *enum_var,
                                                                  const ir::AstNode *expr)
{
    switch (expr->Type()) {
        case ir::AstNodeType::UNARY_EXPRESSION: {
            return EvaluateUnaryExpression(checker, enum_var, expr->AsUnaryExpression());
        }
        case ir::AstNodeType::BINARY_EXPRESSION: {
            return EvaluateBinaryExpression(checker, enum_var, expr->AsBinaryExpression());
        }
        case ir::AstNodeType::NUMBER_LITERAL: {
            return expr->AsNumberLiteral()->Number().GetDouble();
        }
        case ir::AstNodeType::STRING_LITERAL: {
            return expr->AsStringLiteral()->Str();
        }
        case ir::AstNodeType::IDENTIFIER: {
            return EvaluateIdentifier(checker, enum_var, expr->AsIdentifier());
        }
        case ir::AstNodeType::MEMBER_EXPRESSION: {
            return EvaluateEnumMember(checker, enum_var, expr->AsMemberExpression());
        }
        default:
            break;
    }

    return false;
}

bool IsComputedEnumMember(const ir::Expression *init)
{
    if (init->IsLiteral()) {
        return !init->AsLiteral()->IsStringLiteral() && !init->AsLiteral()->IsNumberLiteral();
    }

    if (init->IsTemplateLiteral()) {
        return !init->AsTemplateLiteral()->Quasis().empty();
    }

    return true;
}

void AddEnumValueDeclaration(checker::TSChecker *checker, double number, varbinder::EnumVariable *variable)
{
    variable->SetTsType(checker->GlobalNumberType());

    util::StringView member_str = util::Helpers::ToStringView(checker->Allocator(), number);

    varbinder::LocalScope *enum_scope = checker->Scope()->AsLocalScope();
    varbinder::Variable *res = enum_scope->FindLocal(member_str, varbinder::ResolveBindingOptions::BINDINGS);
    varbinder::EnumVariable *enum_var = nullptr;

    if (res == nullptr) {
        auto *decl = checker->Allocator()->New<varbinder::EnumDecl>(member_str);
        decl->BindNode(variable->Declaration()->Node());
        enum_scope->AddDecl(checker->Allocator(), decl, ScriptExtension::TS);
        res = enum_scope->FindLocal(member_str, varbinder::ResolveBindingOptions::BINDINGS);
        ASSERT(res && res->IsEnumVariable());
        enum_var = res->AsEnumVariable();
        enum_var->AsEnumVariable()->SetBackReference();
        enum_var->SetTsType(checker->GlobalStringType());
    } else {
        ASSERT(res->IsEnumVariable());
        enum_var = res->AsEnumVariable();
        auto *decl = checker->Allocator()->New<varbinder::EnumDecl>(member_str);
        decl->BindNode(variable->Declaration()->Node());
        enum_var->ResetDecl(decl);
    }

    enum_var->SetValue(variable->Declaration()->Name());
}

void InferEnumVariableType(checker::TSChecker *checker, varbinder::EnumVariable *variable, double *value,
                           bool *init_next, bool *is_literal_enum, bool is_const_enum,
                           const ir::Expression *computed_expr)
{
    const ir::Expression *init = variable->Declaration()->Node()->AsTSEnumMember()->Init();

    if (init == nullptr && *init_next) {
        checker->ThrowTypeError("Enum member must have initializer.", variable->Declaration()->Node()->Start());
    }

    if (init == nullptr && !*init_next) {
        variable->SetValue(++(*value));
        AddEnumValueDeclaration(checker, *value, variable);
        return;
    }

    ASSERT(init);

    if (IsComputedEnumMember(init)) {
        if (*is_literal_enum) {
            checker->ThrowTypeError("Computed values are not permitted in an enum with string valued members.",
                                    init->Start());
        }

        computed_expr = init;
    }

    varbinder::EnumMemberResult res = TSEnumDeclaration::EvaluateEnumMember(checker, variable, init);
    if (std::holds_alternative<util::StringView>(res)) {
        if (computed_expr != nullptr) {
            checker->ThrowTypeError("Computed values are not permitted in an enum with string valued members.",
                                    computed_expr->Start());
        }

        *is_literal_enum = true;
        variable->SetTsType(checker->GlobalStringType());
        *init_next = true;
        return;
    }

    if (std::holds_alternative<bool>(res)) {
        if (is_const_enum) {
            checker->ThrowTypeError(
                "const enum member initializers can only contain literal values and other computed enum "
                "values.",
                init->Start());
        }

        *init_next = true;
        return;
    }

    ASSERT(std::holds_alternative<double>(res));
    variable->SetValue(res);

    *value = std::get<double>(res);
    if (is_const_enum) {
        if (std::isnan(*value)) {
            checker->ThrowTypeError("'const' enum member initializer was evaluated to disallowed value 'NaN'.",
                                    init->Start());
        }

        if (std::isinf(*value)) {
            checker->ThrowTypeError("'const' enum member initializer was evaluated to a non-finite value.",
                                    init->Start());
        }
    }

    *init_next = false;
    AddEnumValueDeclaration(checker, *value, variable);
}

void TSEnumDeclaration::Compile(compiler::PandaGen *pg) const
{
    pg->GetAstCompiler()->Compile(this);
}

void TSEnumDeclaration::Compile(compiler::ETSGen *etsg) const
{
    etsg->GetAstCompiler()->Compile(this);
}

checker::Type *TSEnumDeclaration::Check(checker::TSChecker *checker)
{
    return checker->GetAnalyzer()->Check(this);
}

checker::Type *TSEnumDeclaration::Check(checker::ETSChecker *const checker)
{
    return checker->GetAnalyzer()->Check(this);
}
}  // namespace panda::es2panda::ir
