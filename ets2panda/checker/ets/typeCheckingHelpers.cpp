/**
 * Copyright (c) 2021-2024 Huawei Device Co., Ltd.
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

#include "compiler/lowering/scopesInit/scopesInitPhase.h"
#include "varbinder/variableFlags.h"
#include "checker/checker.h"
#include "checker/checkerContext.h"
#include "checker/ets/narrowingWideningConverter.h"
#include "checker/types/globalTypesHolder.h"
#include "checker/types/ets/etsObjectType.h"
#include "ir/astNode.h"
#include "lexer/token/tokenType.h"
#include "ir/base/catchClause.h"
#include "ir/expression.h"
#include "ir/typeNode.h"
#include "ir/base/scriptFunction.h"
#include "ir/base/classProperty.h"
#include "ir/base/methodDefinition.h"
#include "ir/statements/blockStatement.h"
#include "ir/statements/classDeclaration.h"
#include "ir/statements/variableDeclarator.h"
#include "ir/statements/switchCaseStatement.h"
#include "ir/expressions/identifier.h"
#include "ir/expressions/arrayExpression.h"
#include "ir/expressions/objectExpression.h"
#include "ir/expressions/callExpression.h"
#include "ir/expressions/memberExpression.h"
#include "ir/expressions/literals/booleanLiteral.h"
#include "ir/expressions/literals/charLiteral.h"
#include "ir/expressions/binaryExpression.h"
#include "ir/expressions/assignmentExpression.h"
#include "ir/expressions/arrowFunctionExpression.h"
#include "ir/expressions/literals/numberLiteral.h"
#include "ir/expressions/literals/undefinedLiteral.h"
#include "ir/expressions/literals/nullLiteral.h"
#include "ir/statements/labelledStatement.h"
#include "ir/statements/tryStatement.h"
#include "ir/ets/etsFunctionType.h"
#include "ir/ets/etsNewClassInstanceExpression.h"
#include "ir/ets/etsParameterExpression.h"
#include "ir/ts/tsAsExpression.h"
#include "ir/ts/tsTypeAliasDeclaration.h"
#include "ir/ts/tsEnumMember.h"
#include "ir/ts/tsTypeParameter.h"
#include "ir/ets/etsTypeReference.h"
#include "ir/ets/etsTypeReferencePart.h"
#include "ir/ets/etsPrimitiveType.h"
#include "ir/ts/tsQualifiedName.h"
#include "varbinder/variable.h"
#include "varbinder/scope.h"
#include "varbinder/declaration.h"
#include "parser/ETSparser.h"
#include "parser/program/program.h"
#include "checker/ETSchecker.h"
#include "varbinder/ETSBinder.h"
#include "checker/ets/typeRelationContext.h"
#include "checker/ets/boxingConverter.h"
#include "checker/ets/unboxingConverter.h"
#include "checker/types/ets/types.h"
#include "util/helpers.h"

namespace ark::es2panda::checker {
void ETSChecker::CheckTruthinessOfType(ir::Expression *expr)
{
    auto *const testType = expr->Check(this);
    auto *const conditionType = ETSBuiltinTypeAsConditionalType(testType);

    if (conditionType == nullptr || !conditionType->IsConditionalExprType()) {
        ThrowTypeError("Condition must be of possible condition type", expr->Start());
    }

    if (conditionType->IsETSVoidType()) {
        ThrowTypeError("An expression of type 'void' cannot be tested for truthiness", expr->Start());
    }

    if (conditionType->HasTypeFlag(TypeFlag::ETS_PRIMITIVE)) {
        FlagExpressionWithUnboxing(testType, conditionType, expr);
    }

    expr->SetTsType(conditionType);
}

void ETSChecker::CheckNonNullish(ir::Expression const *expr)
{
    if (expr->TsType()->PossiblyETSNullish()) {
        ThrowTypeError("Value is possibly nullish.", expr->Start());
    }
}

Type *ETSChecker::GetNonNullishType(Type *type)
{
    if (type->DefinitelyNotETSNullish()) {
        return type;
    }
    if (type->IsETSTypeParameter()) {
        return Allocator()->New<ETSNonNullishType>(type->AsETSTypeParameter());
    }

    if (type->IsETSNullType() || type->IsETSUndefinedType()) {
        return GetGlobalTypesHolder()->GlobalBuiltinNeverType();
    }

    ArenaVector<Type *> copied(Allocator()->Adapter());
    for (auto const &t : type->AsETSUnionType()->ConstituentTypes()) {
        if (t->IsETSNullType() || t->IsETSUndefinedType()) {
            continue;
        }
        copied.push_back(GetNonNullishType(t));
    }
    return copied.empty() ? GetGlobalTypesHolder()->GlobalBuiltinNeverType() : CreateETSUnionType(std::move(copied));
}

Type *ETSChecker::RemoveNullType(Type *const type)
{
    if (type->DefinitelyNotETSNullish() || type->IsETSUndefinedType()) {
        return type;
    }

    if (type->IsETSTypeParameter()) {
        return Allocator()->New<ETSNonNullishType>(type->AsETSTypeParameter());
    }

    if (type->IsETSNullType()) {
        return GetGlobalTypesHolder()->GlobalBuiltinNeverType();
    }

    ASSERT(type->IsETSUnionType());
    ArenaVector<Type *> copiedTypes(Allocator()->Adapter());

    for (auto *constituentType : type->AsETSUnionType()->ConstituentTypes()) {
        if (!constituentType->IsETSNullType()) {
            copiedTypes.push_back(RemoveNullType(constituentType));
        }
    }

    return copiedTypes.empty() ? GetGlobalTypesHolder()->GlobalBuiltinNeverType()
                               : CreateETSUnionType(std::move(copiedTypes));
}

Type *ETSChecker::RemoveUndefinedType(Type *const type)
{
    if (type->DefinitelyNotETSNullish() || type->IsETSNullType()) {
        return type;
    }

    if (type->IsETSTypeParameter()) {
        return Allocator()->New<ETSNonNullishType>(type->AsETSTypeParameter());
    }

    if (type->IsETSUndefinedType()) {
        return GetGlobalTypesHolder()->GlobalBuiltinNeverType();
    }

    ASSERT(type->IsETSUnionType());
    ArenaVector<Type *> copiedTypes(Allocator()->Adapter());

    for (auto *constituentType : type->AsETSUnionType()->ConstituentTypes()) {
        if (!constituentType->IsETSUndefinedType()) {
            copiedTypes.push_back(RemoveUndefinedType(constituentType));
        }
    }

    return copiedTypes.empty() ? GetGlobalTypesHolder()->GlobalBuiltinNeverType()
                               : CreateETSUnionType(std::move(copiedTypes));
}

// NOTE(vpukhov): can be implemented with relation if etscompiler will support it
template <bool VISIT_NONNULLISH, typename P>
static bool MatchConstituentOrConstraint(P const &pred, const Type *type)
{
    auto const traverse = [](P const &p, const Type *t) {
        return MatchConstituentOrConstraint<VISIT_NONNULLISH, P>(p, t);
    };
    if (pred(type)) {
        return true;
    }
    if (type->IsETSUnionType()) {
        for (auto const &ctype : type->AsETSUnionType()->ConstituentTypes()) {
            if (traverse(pred, ctype)) {
                return true;
            }
        }
        return false;
    }
    if (type->IsETSTypeParameter()) {
        return traverse(pred, type->AsETSTypeParameter()->GetConstraintType());
    }
    if constexpr (VISIT_NONNULLISH) {
        if (type->IsETSNonNullishType()) {
            auto tparam = type->AsETSNonNullishType()->GetUnderlying();
            return traverse(pred, tparam->GetConstraintType());
        }
    }
    return false;
}

bool Type::PossiblyETSNull() const
{
    const auto pred = [](const Type *t) { return t->IsETSNullType(); };
    return MatchConstituentOrConstraint<false>(pred, this);
}

bool Type::PossiblyETSUndefined() const
{
    const auto pred = [](const Type *t) { return t->IsETSUndefinedType(); };
    return MatchConstituentOrConstraint<false>(pred, this);
}

bool Type::PossiblyETSNullish() const
{
    const auto pred = [](const Type *t) { return t->IsETSNullType() || t->IsETSUndefinedType(); };
    return MatchConstituentOrConstraint<false>(pred, this);
}

bool Type::DefinitelyETSNullish() const
{
    const auto pred = [](const Type *t) {
        return !(t->IsTypeParameter() || t->IsETSUnionType() || t->IsETSNullType() || t->IsETSUndefinedType());
    };
    return !MatchConstituentOrConstraint<false>(pred, this);
}

bool Type::DefinitelyNotETSNullish() const
{
    return !PossiblyETSNullish();
}

bool Type::PossiblyETSString() const
{
    const auto pred = [](const Type *t) {
        return t->IsETSStringType() || (t->IsETSObjectType() && t->AsETSObjectType()->IsGlobalETSObjectType());
    };
    return MatchConstituentOrConstraint<true>(pred, this);
}

bool Type::IsETSReferenceType() const
{
    return IsETSObjectType() || IsETSArrayType() || IsETSNullType() || IsETSUndefinedType() || IsETSStringType() ||
           IsETSTypeParameter() || IsETSUnionType() || IsETSNonNullishType() || IsETSBigIntType();
}

bool Type::IsETSUnboxableObject() const
{
    return IsETSObjectType() && AsETSObjectType()->HasObjectFlag(ETSObjectFlags::UNBOXABLE_TYPE);
}

bool ETSChecker::IsConstantExpression(ir::Expression *expr, Type *type)
{
    return (type->HasTypeFlag(TypeFlag::CONSTANT) && (expr->IsIdentifier() || expr->IsMemberExpression()));
}

Type *ETSChecker::GetNonConstantTypeFromPrimitiveType(Type *type) const
{
    if (type->IsETSStringType()) {
        return GlobalBuiltinETSStringType();
    }

    if (!type->HasTypeFlag(TypeFlag::ETS_PRIMITIVE)) {
        return type;
    }

    if (type->HasTypeFlag(TypeFlag::LONG)) {
        return GlobalLongType();
    }

    if (type->HasTypeFlag(TypeFlag::BYTE)) {
        return GlobalByteType();
    }

    if (type->HasTypeFlag(TypeFlag::SHORT)) {
        return GlobalShortType();
    }

    if (type->HasTypeFlag(TypeFlag::CHAR)) {
        return GlobalCharType();
    }

    if (type->HasTypeFlag(TypeFlag::INT)) {
        return GlobalIntType();
    }

    if (type->HasTypeFlag(TypeFlag::FLOAT)) {
        return GlobalFloatType();
    }

    if (type->HasTypeFlag(TypeFlag::DOUBLE)) {
        return GlobalDoubleType();
    }

    if (type->IsETSBooleanType()) {
        return GlobalETSBooleanType();
    }
    return type;
}

Type *ETSChecker::GetTypeOfSetterGetter(varbinder::Variable *const var)
{
    auto *propType = var->TsType()->AsETSFunctionType();
    if (propType->HasTypeFlag(checker::TypeFlag::GETTER)) {
        return propType->FindGetter()->ReturnType();
    }
    return propType->FindSetter()->Params()[0]->TsType();
}

void ETSChecker::IterateInVariableContext(varbinder::Variable *const var)
{
    // Before computing the given variables type, we have to make a new checker context frame so that the checking is
    // done in the proper context, and have to enter the scope where the given variable is declared, so reference
    // resolution works properly
    auto *iter = var->Declaration()->Node()->Parent();
    while (iter != nullptr) {
        if (iter->IsMethodDefinition()) {
            auto *methodDef = iter->AsMethodDefinition();
            ASSERT(methodDef->TsType());
            Context().SetContainingSignature(methodDef->Function()->Signature());
        }

        if (iter->IsClassDefinition()) {
            auto *classDef = iter->AsClassDefinition();
            ETSObjectType *containingClass {};

            if (classDef->TsType() == nullptr) {
                containingClass = BuildBasicClassProperties(classDef);
                ResolveDeclaredMembersOfObject(containingClass);
            } else {
                containingClass = classDef->TsType()->AsETSObjectType();
            }

            ASSERT(classDef->TsType());
            Context().SetContainingClass(containingClass);
        }

        iter = iter->Parent();
    }
}

Type *ETSChecker::GetTypeOfVariable(varbinder::Variable *const var)
{
    if (IsVariableGetterSetter(var)) {
        return GetTypeOfSetterGetter(var);
    }

    if (var->TsType() != nullptr) {
        return var->TsType();
    }

    // NOTE: kbaladurin. forbid usage of imported entities as types without declarations
    if (VarBinder()->AsETSBinder()->IsDynamicModuleVariable(var)) {
        auto *importData = VarBinder()->AsETSBinder()->DynamicImportDataForVar(var);
        if (importData->import->IsPureDynamic()) {
            return GlobalBuiltinDynamicType(importData->import->Language());
        }
    }

    checker::SavedCheckerContext savedContext(this, CheckerStatus::NO_OPTS);
    checker::ScopeContext scopeCtx(this, var->GetScope());
    IterateInVariableContext(var);

    switch (var->Declaration()->Type()) {
        case varbinder::DeclType::CLASS: {
            auto *classDef = var->Declaration()->Node()->AsClassDefinition();
            BuildBasicClassProperties(classDef);
            return classDef->TsType();
        }
        case varbinder::DeclType::ENUM_LITERAL:
        case varbinder::DeclType::CONST:
        case varbinder::DeclType::LET:
        case varbinder::DeclType::VAR: {
            auto *declNode = var->Declaration()->Node();

            if (var->Declaration()->Node()->IsIdentifier()) {
                declNode = declNode->Parent();
            }

            return declNode->Check(this);
        }
        case varbinder::DeclType::FUNC: {
            return var->Declaration()->Node()->Check(this);
        }
        case varbinder::DeclType::IMPORT: {
            return var->Declaration()->Node()->Check(this);
        }
        case varbinder::DeclType::TYPE_ALIAS: {
            return GetTypeFromTypeAliasReference(var);
        }
        case varbinder::DeclType::INTERFACE: {
            return BuildBasicInterfaceProperties(var->Declaration()->Node()->AsTSInterfaceDeclaration());
        }
        default: {
            UNREACHABLE();
        }
    }

    return var->TsType();
}

// Determine if unchecked cast is needed and yield guaranteed source type
Type *ETSChecker::GuaranteedTypeForUncheckedCast(Type *base, Type *substituted)
{
    // Apparent type acts as effective representation for type.
    //  For T extends SomeClass|undefined
    //  Apparent(Int|T|null) is Int|SomeClass|undefined|null
    auto *appBase = GetApparentType(base);
    auto *appSubst = GetApparentType(substituted);
    // Base is supertype of Substituted AND Substituted is supertype of Base
    return Relation()->IsIdenticalTo(appSubst, appBase) ? nullptr : appBase;
}

// Determine if substituted property access requires cast from erased type
Type *ETSChecker::GuaranteedTypeForUncheckedPropertyAccess(varbinder::Variable *const prop)
{
    if (IsVariableStatic(prop)) {
        return nullptr;
    }
    if (IsVariableGetterSetter(prop)) {
        auto *method = prop->TsType()->AsETSFunctionType();
        if (!method->HasTypeFlag(checker::TypeFlag::GETTER)) {
            return nullptr;
        }
        return GuaranteedTypeForUncheckedCallReturn(method->FindGetter());
    }
    // NOTE(vpukhov): mark ETSDynamicType properties
    if (prop->Declaration() == nullptr || prop->Declaration()->Node() == nullptr) {
        return nullptr;
    }

    auto *baseProp = prop->Declaration()->Node()->IsClassProperty()
                         ? prop->Declaration()->Node()->AsClassProperty()->Id()->Variable()
                         : prop->Declaration()->Node()->AsMethodDefinition()->Variable();
    if (baseProp == prop) {
        return nullptr;
    }
    return GuaranteedTypeForUncheckedCast(GetTypeOfVariable(baseProp), GetTypeOfVariable(prop));
}

// Determine if substituted method cast requires cast from erased type
Type *ETSChecker::GuaranteedTypeForUncheckedCallReturn(Signature *sig)
{
    if (sig->HasSignatureFlag(checker::SignatureFlags::THIS_RETURN_TYPE)) {
        return sig->ReturnType();
    }
    auto *baseSig = sig->Function()->Signature();
    if (baseSig == sig) {
        return nullptr;
    }
    return GuaranteedTypeForUncheckedCast(baseSig->ReturnType(), sig->ReturnType());
}

void ETSChecker::CheckEtsFunctionType(ir::Identifier *const ident, ir::Identifier const *const id,
                                      ir::TypeNode const *const annotation)
{
    if (annotation == nullptr) {
        ThrowTypeError(
            {"Cannot infer type for ", id->Name(), " because method reference needs an explicit target type"},
            id->Start());
    }

    const auto *const targetType = GetTypeOfVariable(id->Variable());
    ASSERT(targetType != nullptr);

    if (!targetType->IsETSObjectType() || !targetType->AsETSObjectType()->HasObjectFlag(ETSObjectFlags::FUNCTIONAL)) {
        ThrowTypeError("Initializers type is not assignable to the target type", ident->Start());
    }
}

Type *ETSChecker::GetTypeFromTypeAliasReference(varbinder::Variable *var)
{
    if (var->TsType() != nullptr) {
        return var->TsType();
    }

    auto *const aliasTypeNode = var->Declaration()->Node()->AsTSTypeAliasDeclaration();
    TypeStackElement tse(this, aliasTypeNode, "Circular type alias reference", aliasTypeNode->Start());
    aliasTypeNode->Check(this);
    auto *const aliasedType = aliasTypeNode->TypeAnnotation()->GetType(this);

    var->SetTsType(aliasedType);
    return aliasedType;
}

Type *ETSChecker::GetTypeFromInterfaceReference(varbinder::Variable *var)
{
    if (var->TsType() != nullptr) {
        return var->TsType();
    }

    auto *interfaceType = BuildBasicInterfaceProperties(var->Declaration()->Node()->AsTSInterfaceDeclaration());
    var->SetTsType(interfaceType);
    return interfaceType;
}

Type *ETSChecker::GetTypeFromClassReference(varbinder::Variable *var)
{
    if (var->TsType() != nullptr) {
        return var->TsType();
    }

    auto *classType = BuildBasicClassProperties(var->Declaration()->Node()->AsClassDefinition());
    var->SetTsType(classType);
    return classType;
}

Type *ETSChecker::GetTypeFromEnumReference([[maybe_unused]] varbinder::Variable *var)
{
    if (var->TsType() != nullptr) {
        return var->TsType();
    }

    auto const *const enumDecl = var->Declaration()->Node()->AsTSEnumDeclaration();
    if (auto *const itemInit = enumDecl->Members().front()->AsTSEnumMember()->Init(); itemInit->IsNumberLiteral()) {
        // SUPPRESS_CSA_NEXTLINE(alpha.core.AllocatorETSCheckerHint)
        return CreateETSEnumType(enumDecl);
    } else if (itemInit->IsStringLiteral()) {  // NOLINT(readability-else-after-return)
        // SUPPRESS_CSA_NEXTLINE(alpha.core.AllocatorETSCheckerHint)
        return CreateETSStringEnumType(enumDecl);
    } else {  // NOLINT(readability-else-after-return)
        ThrowTypeError("Invalid enumeration value type.", enumDecl->Start());
    }
}

Type *ETSChecker::GetTypeFromTypeParameterReference(varbinder::LocalVariable *var, const lexer::SourcePosition &pos)
{
    ASSERT(var->Declaration()->Node()->IsTSTypeParameter());
    if ((var->Declaration()->Node()->AsTSTypeParameter()->Parent()->Parent()->IsClassDefinition() ||
         var->Declaration()->Node()->AsTSTypeParameter()->Parent()->Parent()->IsTSInterfaceDeclaration()) &&
        HasStatus(CheckerStatus::IN_STATIC_CONTEXT)) {
        ThrowTypeError({"Cannot make a static reference to the non-static type ", var->Name()}, pos);
    }

    return var->TsType();
}

bool ETSChecker::IsTypeBuiltinType(const Type *type) const
{
    if (!type->IsETSObjectType()) {
        return false;
    }

    switch (type->AsETSObjectType()->BuiltInKind()) {
        case ETSObjectFlags::BUILTIN_BOOLEAN:
        case ETSObjectFlags::BUILTIN_BYTE:
        case ETSObjectFlags::BUILTIN_SHORT:
        case ETSObjectFlags::BUILTIN_CHAR:
        case ETSObjectFlags::BUILTIN_INT:
        case ETSObjectFlags::BUILTIN_LONG:
        case ETSObjectFlags::BUILTIN_FLOAT:
        case ETSObjectFlags::BUILTIN_DOUBLE: {
            return true;
        }
        default:
            return false;
    }
}

Type *ETSChecker::ETSBuiltinTypeAsPrimitiveType(Type *objectType)
{
    if (objectType == nullptr) {
        return nullptr;
    }

    if (objectType->HasTypeFlag(TypeFlag::ETS_PRIMITIVE) || objectType->HasTypeFlag(TypeFlag::ETS_ENUM) ||
        objectType->HasTypeFlag(TypeFlag::ETS_STRING_ENUM)) {
        return objectType;
    }

    if (!objectType->IsETSObjectType() ||
        !objectType->AsETSObjectType()->HasObjectFlag(ETSObjectFlags::UNBOXABLE_TYPE)) {
        return nullptr;
    }

    auto savedResult = Relation()->IsTrue();
    Relation()->Result(false);

    UnboxingConverter converter = UnboxingConverter(AsETSChecker(), Relation(), objectType, objectType);
    Relation()->Result(savedResult);
    return converter.Result();
}

Type *ETSChecker::ETSBuiltinTypeAsConditionalType(Type *const objectType)
{
    if ((objectType == nullptr) || !objectType->IsConditionalExprType()) {
        return nullptr;
    }

    if (auto *unboxed = ETSBuiltinTypeAsPrimitiveType(objectType); unboxed != nullptr) {
        return unboxed;
    }

    return objectType;
}

Type *ETSChecker::PrimitiveTypeAsETSBuiltinType(Type *objectType)
{
    if (objectType == nullptr) {
        return nullptr;
    }

    if (objectType->IsETSObjectType() && objectType->AsETSObjectType()->HasObjectFlag(ETSObjectFlags::UNBOXABLE_TYPE)) {
        return objectType;
    }

    if (!objectType->HasTypeFlag(TypeFlag::ETS_PRIMITIVE)) {
        return nullptr;
    }

    auto savedResult = Relation()->IsTrue();
    Relation()->Result(false);

    BoxingConverter converter = BoxingConverter(AsETSChecker(), Relation(), objectType,
                                                Checker::GetGlobalTypesHolder()->GlobalIntegerBuiltinType());
    Relation()->Result(savedResult);
    return converter.Result();
}

Type *ETSChecker::MaybePromotedBuiltinType(Type *type) const
{
    return type->HasTypeFlag(TypeFlag::ETS_PRIMITIVE) && !type->IsETSVoidType()
               ? checker::BoxingConverter::ETSTypeFromSource(this, type)
               : type;
}

Type const *ETSChecker::MaybePromotedBuiltinType(Type const *type) const
{
    return type->HasTypeFlag(TypeFlag::ETS_PRIMITIVE) ? checker::BoxingConverter::ETSTypeFromSource(this, type) : type;
}

Type *ETSChecker::MaybePrimitiveBuiltinType(Type *type) const
{
    return type->IsETSObjectType() ? UnboxingConverter::GlobalTypeFromSource(this, type->AsETSObjectType()) : type;
}

Type *ETSChecker::MaybeBoxedType(const varbinder::Variable *var, ArenaAllocator *allocator) const
{
    auto *varType = var->TsType();
    if (var->HasFlag(varbinder::VariableFlags::BOXED)) {
        switch (TypeKind(varType)) {
            case TypeFlag::ETS_BOOLEAN:
                return GetGlobalTypesHolder()->GlobalBooleanBoxBuiltinType();
            case TypeFlag::BYTE:
                return GetGlobalTypesHolder()->GlobalByteBoxBuiltinType();
            case TypeFlag::CHAR:
                return GetGlobalTypesHolder()->GlobalCharBoxBuiltinType();
            case TypeFlag::SHORT:
                return GetGlobalTypesHolder()->GlobalShortBoxBuiltinType();
            case TypeFlag::INT:
                return GetGlobalTypesHolder()->GlobalIntBoxBuiltinType();
            case TypeFlag::LONG:
                return GetGlobalTypesHolder()->GlobalLongBoxBuiltinType();
            case TypeFlag::FLOAT:
                return GetGlobalTypesHolder()->GlobalFloatBoxBuiltinType();
            case TypeFlag::DOUBLE:
                return GetGlobalTypesHolder()->GlobalDoubleBoxBuiltinType();
            default: {
                Type *box = GetGlobalTypesHolder()->GlobalBoxBuiltinType()->Instantiate(allocator, Relation(),
                                                                                        GetGlobalTypesHolder());
                box->AddTypeFlag(checker::TypeFlag::GENERIC);
                box->AsETSObjectType()->TypeArguments().emplace_back(varType);
                return box;
            }
        }
    }
    return varType;
}

ir::BoxingUnboxingFlags ETSChecker::GetBoxingFlag(Type *const boxingType)
{
    auto typeKind = TypeKind(ETSBuiltinTypeAsPrimitiveType(boxingType));
    switch (typeKind) {
        case TypeFlag::ETS_BOOLEAN: {
            return ir::BoxingUnboxingFlags::BOX_TO_BOOLEAN;
        }
        case TypeFlag::BYTE: {
            return ir::BoxingUnboxingFlags::BOX_TO_BYTE;
        }
        case TypeFlag::CHAR: {
            return ir::BoxingUnboxingFlags::BOX_TO_CHAR;
        }
        case TypeFlag::SHORT: {
            return ir::BoxingUnboxingFlags::BOX_TO_SHORT;
        }
        case TypeFlag::INT: {
            return ir::BoxingUnboxingFlags::BOX_TO_INT;
        }
        case TypeFlag::LONG: {
            return ir::BoxingUnboxingFlags::BOX_TO_LONG;
        }
        case TypeFlag::FLOAT: {
            return ir::BoxingUnboxingFlags::BOX_TO_FLOAT;
        }
        case TypeFlag::DOUBLE: {
            return ir::BoxingUnboxingFlags::BOX_TO_DOUBLE;
        }
        default:
            UNREACHABLE();
    }
}

Type *ETSChecker::GetBoxedType(ir::BoxingUnboxingFlags flag) const
{
    switch (flag) {
        case ir::BoxingUnboxingFlags::UNBOX_TO_BOOLEAN: {
            return GlobalETSBooleanType();
        }
        case ir::BoxingUnboxingFlags::UNBOX_TO_BYTE: {
            return GlobalByteType();
        }
        case ir::BoxingUnboxingFlags::UNBOX_TO_CHAR: {
            return GlobalCharType();
        }
        case ir::BoxingUnboxingFlags::UNBOX_TO_SHORT: {
            return GlobalShortType();
        }
        case ir::BoxingUnboxingFlags::UNBOX_TO_INT: {
            return GlobalIntType();
        }
        case ir::BoxingUnboxingFlags::UNBOX_TO_LONG: {
            return GlobalLongType();
        }
        case ir::BoxingUnboxingFlags::UNBOX_TO_FLOAT: {
            return GlobalFloatType();
        }
        case ir::BoxingUnboxingFlags::UNBOX_TO_DOUBLE: {
            return GlobalDoubleType();
        }
        default:
            UNREACHABLE();
    }
}

ir::BoxingUnboxingFlags ETSChecker::GetUnboxingFlag(Type const *const unboxingType) const
{
    auto typeKind = TypeKind(unboxingType);
    switch (typeKind) {
        case TypeFlag::ETS_BOOLEAN: {
            return ir::BoxingUnboxingFlags::UNBOX_TO_BOOLEAN;
        }
        case TypeFlag::BYTE: {
            return ir::BoxingUnboxingFlags::UNBOX_TO_BYTE;
        }
        case TypeFlag::CHAR: {
            return ir::BoxingUnboxingFlags::UNBOX_TO_CHAR;
        }
        case TypeFlag::SHORT: {
            return ir::BoxingUnboxingFlags::UNBOX_TO_SHORT;
        }
        case TypeFlag::INT: {
            return ir::BoxingUnboxingFlags::UNBOX_TO_INT;
        }
        case TypeFlag::LONG: {
            return ir::BoxingUnboxingFlags::UNBOX_TO_LONG;
        }
        case TypeFlag::FLOAT: {
            return ir::BoxingUnboxingFlags::UNBOX_TO_FLOAT;
        }
        case TypeFlag::DOUBLE: {
            return ir::BoxingUnboxingFlags::UNBOX_TO_DOUBLE;
        }
        default:
            UNREACHABLE();
    }
}

void ETSChecker::AddBoxingFlagToPrimitiveType(TypeRelation *relation, Type *target)
{
    auto boxingResult = PrimitiveTypeAsETSBuiltinType(target);
    if ((boxingResult != nullptr) && !relation->OnlyCheckBoxingUnboxing()) {
        relation->GetNode()->AddBoxingUnboxingFlags(GetBoxingFlag(boxingResult));
        relation->Result(true);
    }
}

void ETSChecker::AddUnboxingFlagToPrimitiveType(TypeRelation *relation, Type *source, Type *self)
{
    auto unboxingResult = UnboxingConverter(this, relation, source, self).Result();
    if ((unboxingResult != nullptr) && relation->IsTrue() && !relation->OnlyCheckBoxingUnboxing()) {
        relation->GetNode()->AddBoxingUnboxingFlags(GetUnboxingFlag(unboxingResult));
    }
}

void ETSChecker::CheckUnboxedTypeWidenable(TypeRelation *relation, Type *target, Type *self)
{
    checker::SavedTypeRelationFlagsContext savedTypeRelationFlagCtx(
        relation, TypeRelationFlag::ONLY_CHECK_WIDENING |
                      (relation->ApplyNarrowing() ? TypeRelationFlag::NARROWING : TypeRelationFlag::NONE));
    // NOTE: vpukhov. handle union type
    auto unboxedType = ETSBuiltinTypeAsPrimitiveType(target);
    if (unboxedType == nullptr) {
        return;
    }
    NarrowingWideningConverter(this, relation, unboxedType, self);
    if (!relation->IsTrue()) {
        relation->Result(relation->IsAssignableTo(self, unboxedType));
    }
}

void ETSChecker::CheckUnboxedTypesAssignable(TypeRelation *relation, Type *source, Type *target)
{
    auto *unboxedSourceType = relation->GetChecker()->AsETSChecker()->ETSBuiltinTypeAsPrimitiveType(source);
    auto *unboxedTargetType = relation->GetChecker()->AsETSChecker()->ETSBuiltinTypeAsPrimitiveType(target);
    if (unboxedSourceType == nullptr || unboxedTargetType == nullptr) {
        return;
    }
    relation->IsAssignableTo(unboxedSourceType, unboxedTargetType);
    if (relation->IsTrue()) {
        relation->GetNode()->AddBoxingUnboxingFlags(
            relation->GetChecker()->AsETSChecker()->GetUnboxingFlag(unboxedSourceType));
    }
}

void ETSChecker::CheckBoxedSourceTypeAssignable(TypeRelation *relation, Type *source, Type *target)
{
    ASSERT(relation != nullptr);
    checker::SavedTypeRelationFlagsContext savedTypeRelationFlagCtx(
        relation, (relation->ApplyWidening() ? TypeRelationFlag::WIDENING : TypeRelationFlag::NONE) |
                      (relation->ApplyNarrowing() ? TypeRelationFlag::NARROWING : TypeRelationFlag::NONE) |
                      (relation->OnlyCheckBoxingUnboxing() ? TypeRelationFlag::ONLY_CHECK_BOXING_UNBOXING
                                                           : TypeRelationFlag::NONE));
    auto *boxedSourceType = relation->GetChecker()->AsETSChecker()->PrimitiveTypeAsETSBuiltinType(source);
    if (boxedSourceType == nullptr) {
        return;
    }
    ASSERT(target != nullptr);
    // Do not box primitive in case of cast to dynamic types
    if (target->IsETSDynamicType()) {
        return;
    }
    relation->IsAssignableTo(boxedSourceType, target);
    if (relation->IsTrue()) {
        AddBoxingFlagToPrimitiveType(relation, boxedSourceType);
    } else {
        auto unboxedTargetType = ETSBuiltinTypeAsPrimitiveType(target);
        if (unboxedTargetType == nullptr) {
            return;
        }
        NarrowingWideningConverter(this, relation, unboxedTargetType, source);
        if (relation->IsTrue()) {
            AddBoxingFlagToPrimitiveType(relation, target);
        }
    }
}

void ETSChecker::CheckUnboxedSourceTypeWithWideningAssignable(TypeRelation *relation, Type *source, Type *target)
{
    auto *unboxedSourceType = relation->GetChecker()->AsETSChecker()->ETSBuiltinTypeAsPrimitiveType(source);
    if (unboxedSourceType == nullptr) {
        return;
    }
    relation->IsAssignableTo(unboxedSourceType, target);
    if (!relation->IsTrue() && relation->ApplyWidening()) {
        relation->GetChecker()->AsETSChecker()->CheckUnboxedTypeWidenable(relation, target, unboxedSourceType);
    }
    if (!relation->OnlyCheckBoxingUnboxing()) {
        relation->GetNode()->AddBoxingUnboxingFlags(
            relation->GetChecker()->AsETSChecker()->GetUnboxingFlag(unboxedSourceType));
    }
}

static ir::AstNode *DerefETSTypeReference(ir::AstNode *node)
{
    ASSERT(node->IsETSTypeReference());
    do {
        auto *name = node->AsETSTypeReference()->Part()->Name();
        ASSERT(name->IsIdentifier());
        auto *var = name->AsIdentifier()->Variable();
        ASSERT(var != nullptr);
        auto *declNode = var->Declaration()->Node();
        if (!declNode->IsTSTypeAliasDeclaration()) {
            return declNode;
        }
        node = declNode->AsTSTypeAliasDeclaration()->TypeAnnotation();
    } while (node->IsETSTypeReference());
    return node;
}

bool ETSChecker::CheckLambdaAssignable(ir::Expression *param, ir::ScriptFunction *lambda)
{
    ASSERT(param->IsETSParameterExpression());
    ir::AstNode *typeAnn = param->AsETSParameterExpression()->Ident()->TypeAnnotation();
    if (typeAnn->IsETSTypeReference()) {
        typeAnn = DerefETSTypeReference(typeAnn);
    }
    if (!typeAnn->IsETSFunctionType()) {
        if (typeAnn->IsETSUnionType()) {
            return CheckLambdaAssignableUnion(typeAnn, lambda);
        }

        return false;
    }
    ir::ETSFunctionType *calleeType = typeAnn->AsETSFunctionType();
    return lambda->Params().size() == calleeType->Params().size();
}

bool ETSChecker::TypeInference(Signature *signature, const ArenaVector<ir::Expression *> &arguments,
                               TypeRelationFlag flags)
{
    bool invocable = true;
    auto const argumentCount = arguments.size();
    auto const parameterCount = signature->Params().size();
    auto const count = std::min(parameterCount, argumentCount);

    for (size_t index = 0U; index < count; ++index) {
        auto const &argument = arguments[index];
        if (!argument->IsArrowFunctionExpression()) {
            continue;
        }

        if (index == arguments.size() - 1 && (flags & TypeRelationFlag::NO_CHECK_TRAILING_LAMBDA) != 0) {
            continue;
        }

        auto *const arrowFuncExpr = argument->AsArrowFunctionExpression();
        ir::ScriptFunction *const lambda = arrowFuncExpr->Function();
        if (!NeedTypeInference(lambda)) {
            continue;
        }

        auto const *const param = signature->Function()->Params()[index]->AsETSParameterExpression()->Ident();
        ir::AstNode *typeAnn = param->TypeAnnotation();

        if (typeAnn->IsETSTypeReference()) {
            typeAnn = DerefETSTypeReference(typeAnn);
        }

        ASSERT(typeAnn->IsETSFunctionType());
        InferTypesForLambda(lambda, typeAnn->AsETSFunctionType());
        Type *const argumentType = GetApparentType(arrowFuncExpr->Check(this));
        Type *const parameterType = GetApparentType(signature->Params()[index]->TsType());
        const Type *targetType = TryGettingFunctionTypeFromInvokeFunction(parameterType);
        const std::initializer_list<TypeErrorMessageElement> msg = {
            "Type '", argumentType, "' is not compatible with type '", targetType, "' at index ", index + 1};

        checker::InvocationContext invokationCtx(Relation(), arguments[index], argumentType, parameterType,
                                                 arrowFuncExpr->Start(), msg, flags);

        invocable &= invokationCtx.IsInvocable();
    }
    return invocable;
}

bool ETSChecker::ExtensionETSFunctionType(checker::Type *type)
{
    if (!type->IsETSFunctionType()) {
        return false;
    }

    for (auto *signature : type->AsETSFunctionType()->CallSignatures()) {
        if (signature->Function()->IsExtensionMethod()) {
            return true;
        }
    }

    return false;
}

void ETSChecker::CheckExceptionClauseType(const std::vector<checker::ETSObjectType *> &exceptions,
                                          ir::CatchClause *catchClause, checker::Type *clauseType)
{
    for (auto *exception : exceptions) {
        this->Relation()->IsIdenticalTo(clauseType, exception);
        if (this->Relation()->IsTrue()) {
            this->ThrowTypeError("Redeclaration of exception type", catchClause->Start());
        }
    }
}
}  // namespace ark::es2panda::checker
