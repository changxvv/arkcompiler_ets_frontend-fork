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

#include "memberExpression.h"

#include "checker/TSchecker.h"
#include "checker/ets/castingContext.h"
#include "compiler/core/ETSGen.h"
#include "compiler/core/pandagen.h"
#include "ir/astDump.h"
#include "ir/srcDump.h"

namespace ark::es2panda::ir {
MemberExpression::MemberExpression([[maybe_unused]] Tag const tag, MemberExpression const &other,
                                   ArenaAllocator *allocator)
    : MemberExpression(other)
{
    object_ = other.object_ != nullptr ? other.object_->Clone(allocator, this)->AsExpression() : nullptr;
    property_ = other.property_ != nullptr ? other.property_->Clone(allocator, this)->AsExpression() : nullptr;
}

bool MemberExpression::IsPrivateReference() const noexcept
{
    return property_->IsIdentifier() && property_->AsIdentifier()->IsPrivateIdent();
}

void MemberExpression::TransformChildren(const NodeTransformer &cb)
{
    object_ = cb(object_)->AsExpression();
    property_ = cb(property_)->AsExpression();
}

void MemberExpression::Iterate(const NodeTraverser &cb) const
{
    cb(object_);
    cb(property_);
}

void MemberExpression::Dump(ir::AstDumper *dumper) const
{
    dumper->Add({{"type", "MemberExpression"},
                 {"object", object_},
                 {"property", property_},
                 {"computed", computed_},
                 {"optional", IsOptional()}});
}

void MemberExpression::Dump(ir::SrcDumper *dumper) const
{
    ASSERT(object_ != nullptr);
    ASSERT(property_ != nullptr);

    object_->Dump(dumper);
    if (IsOptional()) {
        dumper->Add("?");
    }
    if ((MemberExpressionKind::ELEMENT_ACCESS & kind_) != 0U) {
        dumper->Add("[");
        property_->Dump(dumper);
        dumper->Add("]");
    } else {
        dumper->Add(".");
        property_->Dump(dumper);
    }
    if ((parent_ != nullptr) && (parent_->IsBlockStatement() || parent_->IsBlockExpression())) {
        dumper->Add(";");
        dumper->Endl();
    }
}

void MemberExpression::LoadRhs(compiler::PandaGen *pg) const
{
    compiler::RegScope rs(pg);
    bool isSuper = object_->IsSuperExpression();
    compiler::Operand prop = pg->ToPropertyKey(property_, computed_, isSuper);

    if (isSuper) {
        pg->LoadSuperProperty(this, prop);
    } else if (IsPrivateReference()) {
        const auto &name = property_->AsIdentifier()->Name();
        compiler::VReg objReg = pg->AllocReg();
        pg->StoreAccumulator(this, objReg);
        compiler::VReg ctor = pg->AllocReg();
        compiler::Function::LoadClassContexts(this, pg, ctor, name);
        pg->ClassPrivateFieldGet(this, ctor, objReg, name);
    } else {
        pg->LoadObjProperty(this, prop);
    }
}

void MemberExpression::CompileToRegs(compiler::PandaGen *pg, compiler::VReg object, compiler::VReg property) const
{
    object_->Compile(pg);
    pg->StoreAccumulator(this, object);

    pg->OptionalChainCheck(IsOptional(), object);

    if (!computed_) {
        pg->LoadAccumulatorString(this, property_->AsIdentifier()->Name());
    } else {
        property_->Compile(pg);
    }

    pg->StoreAccumulator(this, property);
}

void MemberExpression::Compile(compiler::PandaGen *pg) const
{
    pg->GetAstCompiler()->Compile(this);
}

void MemberExpression::CompileToReg(compiler::PandaGen *pg, compiler::VReg objReg) const
{
    object_->Compile(pg);
    pg->StoreAccumulator(this, objReg);
    pg->OptionalChainCheck(IsOptional(), objReg);
    LoadRhs(pg);
}

void MemberExpression::Compile(compiler::ETSGen *etsg) const
{
    etsg->GetAstCompiler()->Compile(this);
}

checker::Type *MemberExpression::Check(checker::TSChecker *checker)
{
    return checker->GetAnalyzer()->Check(this);
}

std::pair<checker::Type *, varbinder::LocalVariable *> MemberExpression::ResolveEnumMember(checker::ETSChecker *checker,
                                                                                           checker::Type *type) const
{
    auto const *const enumInterface = [type]() -> checker::ETSEnumInterface const * {
        if (type->IsETSEnumType()) {
            return type->AsETSEnumType();
        }
        return type->AsETSStringEnumType();
    }();

    if (parent_->Type() == ir::AstNodeType::CALL_EXPRESSION && parent_->AsCallExpression()->Callee() == this) {
        return {enumInterface->LookupMethod(checker, object_, property_->AsIdentifier()), nullptr};
    }

    auto *const literalType = enumInterface->LookupConstant(checker, object_, property_->AsIdentifier());
    return {literalType, literalType->GetMemberVar()};
}

std::pair<checker::Type *, varbinder::LocalVariable *> MemberExpression::ResolveObjectMember(
    checker::ETSChecker *checker) const
{
    auto resolveRes = checker->ResolveMemberReference(this, objType_);
    switch (resolveRes.size()) {
        case 1U: {
            if (resolveRes[0]->Kind() == checker::ResolvedKind::PROPERTY) {
                auto var = resolveRes[0]->Variable()->AsLocalVariable();
                checker->ValidatePropertyAccess(var, objType_, property_->Start());
                return {checker->GetTypeOfVariable(var), var};
            }
            return {checker->GetTypeOfVariable(resolveRes[0]->Variable()), nullptr};
        }
        case 2U: {
            auto classMethodType = checker->GetTypeOfVariable(resolveRes[1]->Variable());
            auto extensionMethodType = checker->GetTypeOfVariable(resolveRes[0]->Variable());
            auto *resolvedType = extensionMethodType;
            if (classMethodType->IsETSFunctionType()) {
                ASSERT(extensionMethodType->IsETSFunctionType());
                resolvedType = checker->CreateETSExtensionFuncHelperType(classMethodType->AsETSFunctionType(),
                                                                         extensionMethodType->AsETSFunctionType());
            }
            return {resolvedType, nullptr};
        }
        default: {
            UNREACHABLE();
        }
    }
}

checker::Type *MemberExpression::TraverseUnionMember(checker::ETSChecker *checker, checker::ETSUnionType *unionType,
                                                     checker::Type *commonPropType)

{
    auto const addPropType = [this, checker, &commonPropType](checker::Type *memberType) {
        if (commonPropType != nullptr && commonPropType != memberType) {
            checker->ThrowTypeError("Member type must be the same for all union objects.", Start());
        }
        commonPropType = memberType;
    };
    for (auto *const type : unionType->ConstituentTypes()) {
        auto *const apparent = checker->GetApparentType(type);
        if (apparent->IsETSObjectType()) {
            SetObjectType(apparent->AsETSObjectType());
            addPropType(ResolveObjectMember(checker).first);
        } else if (apparent->IsETSEnumType() || apparent->IsETSStringEnumType()) {
            addPropType(ResolveEnumMember(checker, apparent).first);
        } else {
            checker->ThrowTypeError({"Type ", unionType, " is illegal in union member expression."}, Start());
        }
    }
    return commonPropType;
}

checker::Type *MemberExpression::CheckUnionMember(checker::ETSChecker *checker, checker::Type *baseType)
{
    auto *const unionType = baseType->AsETSUnionType();
    auto *const commonPropType = TraverseUnionMember(checker, unionType, nullptr);
    SetObjectType(checker->GlobalETSObjectType());
    return commonPropType;
}

checker::Type *MemberExpression::AdjustType(checker::ETSChecker *checker, checker::Type *type)
{
    auto *const objType = checker->GetApparentType(Object()->TsType());
    if (PropVar() != nullptr) {  // access erased property type
        uncheckedType_ = checker->GuaranteedTypeForUncheckedPropertyAccess(PropVar());
    } else if (IsComputed() && objType->IsETSArrayType()) {  // access erased array or tuple type
        uncheckedType_ = checker->GuaranteedTypeForUncheckedCast(objType->AsETSArrayType()->ElementType(), type);
    }
    SetTsType(type);
    return TsType();
}

void MemberExpression::CheckArrayIndexValue(checker::ETSChecker *checker) const
{
    std::size_t index;

    auto const &number = property_->AsNumberLiteral()->Number();

    if (number.IsInteger()) {
        auto const value = number.GetLong();
        if (value < 0) {
            checker->ThrowTypeError("Index value cannot be less than zero.", property_->Start());
        }
        index = static_cast<std::size_t>(value);
    } else if (number.IsReal()) {
        double value = number.GetDouble();
        double fraction = std::modf(value, &value);
        if (value < 0.0 || fraction >= std::numeric_limits<double>::epsilon()) {
            checker->ThrowTypeError("Index value cannot be less than zero or fractional.", property_->Start());
        }
        index = static_cast<std::size_t>(value);
    } else {
        UNREACHABLE();
    }

    if (object_->IsArrayExpression() && object_->AsArrayExpression()->Elements().size() <= index) {
        checker->ThrowTypeError("Index value cannot be greater than or equal to the array size.", property_->Start());
    }
}

checker::Type *MemberExpression::CheckIndexAccessMethod(checker::ETSChecker *checker)
{
    checker::PropertySearchFlags searchFlag =
        checker::PropertySearchFlags::SEARCH_METHOD | checker::PropertySearchFlags::IS_FUNCTIONAL;
    searchFlag |= checker::PropertySearchFlags::SEARCH_IN_BASE | checker::PropertySearchFlags::SEARCH_IN_INTERFACES;
    // NOTE(DZ) maybe we need to exclude static methods: search_flag &= ~(checker::PropertySearchFlags::SEARCH_STATIC);

    if (objType_->HasTypeFlag(checker::TypeFlag::GENERIC)) {
        searchFlag |= checker::PropertySearchFlags::SEARCH_ALL;
    }

    bool const isSetter = Parent()->IsAssignmentExpression() && Parent()->AsAssignmentExpression()->Left() == this;
    std::string_view const methodName =
        isSetter ? compiler::Signatures::SET_INDEX_METHOD : compiler::Signatures::GET_INDEX_METHOD;

    auto *const method = objType_->GetProperty(methodName, searchFlag);
    if (method == nullptr || !method->HasFlag(varbinder::VariableFlags::METHOD)) {
        checker->ThrowTypeError("Object type doesn't have proper index access method.", Start());
    }

    ArenaVector<Expression *> arguments {checker->Allocator()->Adapter()};
    arguments.emplace_back(property_);
    if (isSetter) {
        //  Temporary change the parent of right assignment node to check if correct "$_set" function presents.
        //  later on in lowering the entire assignment expression will be replace top the call to that method.
        auto *value = Parent()->AsAssignmentExpression()->Right();
        value->SetParent(this);
        arguments.emplace_back(value);
    }

    auto &signatures = checker->GetTypeOfVariable(method)->AsETSFunctionType()->CallSignatures();

    checker::Signature *signature = checker->ValidateSignatures(signatures, nullptr, arguments, Start(), "indexing",
                                                                checker::TypeRelationFlag::NO_THROW);
    if (signature == nullptr) {
        checker->ThrowTypeError("Cannot find index access method with the required signature.", Property()->Start());
    }
    checker->ValidateSignatureAccessibility(objType_, nullptr, signature, Start(),
                                            "Index access method is not visible here.");

    ASSERT(signature->Function() != nullptr);

    if (signature->Function()->IsThrowing() || signature->Function()->IsRethrowing()) {
        checker->CheckThrowingStatements(this);
    }

    if (isSetter) {
        //  Restore the right assignment node's parent to keep AST invariant valid.
        Parent()->AsAssignmentExpression()->Right()->SetParent(Parent());
        return signature->Params()[1]->TsType();
    }

    return signature->ReturnType();
}

checker::Type *MemberExpression::CheckTupleAccessMethod(checker::ETSChecker *checker, checker::Type *baseType)
{
    ASSERT(baseType->IsETSTupleType());

    auto *const tupleTypeAtIdx =
        baseType->AsETSTupleType()->GetTypeAtIndex(checker->GetTupleElementAccessValue(Property()->TsType()));

    if ((!Parent()->IsAssignmentExpression() || Parent()->AsAssignmentExpression()->Left() != this) &&
        (!Parent()->IsUpdateExpression())) {
        // Error never should be thrown by this call, because LUB of types can be converted to any type which
        // LUB was calculated by casting
        const checker::CastingContext cast(checker->Relation(), this, baseType->AsETSArrayType()->ElementType(),
                                           tupleTypeAtIdx, Start(), {"Tuple type couldn't be converted "});
    }

    return tupleTypeAtIdx;
}

checker::Type *MemberExpression::CheckComputed(checker::ETSChecker *checker, checker::Type *baseType)
{
    if (baseType->IsETSArrayType() || baseType->IsETSDynamicType()) {
        checker->ValidateArrayIndex(property_);

        if (baseType->IsETSTupleType()) {
            checker->ValidateTupleIndex(baseType->AsETSTupleType(), this);
        } else if (baseType->IsETSArrayType() && property_->IsNumberLiteral()) {
            // Check if the index value is inside array bounds if it is defined explicitly
            CheckArrayIndexValue(checker);
        }

        // NOTE: apply capture conversion on this type
        if (baseType->IsETSArrayType()) {
            if (baseType->IsETSTupleType()) {
                return CheckTupleAccessMethod(checker, baseType);
            }

            if (object_->IsArrayExpression() && property_->IsNumberLiteral()) {
                auto const number = property_->AsNumberLiteral()->Number().GetLong();
                return object_->AsArrayExpression()->Elements()[number]->Check(checker);
            }

            return baseType->AsETSArrayType()->ElementType();
        }

        // Dynamic
        return checker->GlobalBuiltinDynamicType(baseType->AsETSDynamicType()->Language());
    }

    if (baseType->IsETSObjectType()) {
        SetObjectType(baseType->AsETSObjectType());
        return CheckIndexAccessMethod(checker);
    }

    checker->ThrowTypeError("Indexed access is not supported for such expression type.", Object()->Start());
}

checker::Type *MemberExpression::Check(checker::ETSChecker *checker)
{
    return checker->GetAnalyzer()->Check(this);
}

MemberExpression *MemberExpression::Clone(ArenaAllocator *const allocator, AstNode *const parent)
{
    if (auto *const clone = allocator->New<MemberExpression>(Tag {}, *this, allocator); clone != nullptr) {
        if (parent != nullptr) {
            clone->SetParent(parent);
        }

        clone->SetRange(Range());
        return clone;
    }

    throw Error(ErrorType::GENERIC, "", CLONE_ALLOCATION_ERROR);
}

}  // namespace ark::es2panda::ir
