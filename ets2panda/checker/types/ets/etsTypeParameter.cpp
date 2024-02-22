/*
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

#include "etsTypeParameter.h"
#include "ir/expressions/identifier.h"
#include "ir/ts/tsTypeParameter.h"
#include "checker/ETSchecker.h"
#include "checker/ets/conversion.h"

namespace ark::es2panda::checker {

void ETSTypeParameter::ToString(std::stringstream &ss, bool precise) const
{
    // Need source file name to avoid clashes
    if (precise) {
        ss << declNode_->Range().start.index << "." << declNode_->Range().start.line << ".";
    }
    ss << declNode_->Name()->Name();
}

void ETSTypeParameter::Identical([[maybe_unused]] TypeRelation *relation, [[maybe_unused]] Type *other)
{
    relation->Result(false);
    if (other->IsETSTypeParameter() && other->AsETSTypeParameter()->GetOriginal() == GetOriginal()) {
        relation->Result(true);
    }
}

bool ETSTypeParameter::AssignmentSource([[maybe_unused]] TypeRelation *relation, [[maybe_unused]] Type *target)
{
    return relation->Result(false);
}

void ETSTypeParameter::AssignmentTarget([[maybe_unused]] TypeRelation *relation, [[maybe_unused]] Type *source)
{
    if (source->IsETSTypeParameter() && source->AsETSTypeParameter()->GetOriginal() == GetOriginal()) {
        relation->Result(true);
        return;
    }

    relation->IsSupertypeOf(this, source);
}

void ETSTypeParameter::Cast(TypeRelation *relation, Type *target)
{
    if (relation->IsSupertypeOf(target, this)) {
        relation->RemoveFlags(TypeRelationFlag::UNCHECKED_CAST);
        return;
    }

    // NOTE(vpukhov): adjust UNCHECKED_CAST flags
    if (target->IsETSObjectType()) {
        relation->RemoveFlags(TypeRelationFlag::UNCHECKED_CAST);
    }
    relation->Result(relation->InCastingContext());
}

void ETSTypeParameter::CastTarget(TypeRelation *relation, Type *source)
{
    if (relation->IsSupertypeOf(this, source)) {
        relation->RemoveFlags(TypeRelationFlag::UNCHECKED_CAST);
        return;
    }

    relation->Result(relation->InCastingContext());
}

void ETSTypeParameter::IsSupertypeOf([[maybe_unused]] TypeRelation *relation, [[maybe_unused]] Type *source)
{
    relation->Result(false);
}

void ETSTypeParameter::IsSubtypeOf([[maybe_unused]] TypeRelation *relation, [[maybe_unused]] Type *target)
{
    if (relation->IsSupertypeOf(target, GetConstraintType())) {
        return;
    }

    relation->Result(false);
}

Type *ETSTypeParameter::Instantiate([[maybe_unused]] ArenaAllocator *allocator, [[maybe_unused]] TypeRelation *relation,
                                    [[maybe_unused]] GlobalTypesHolder *globalTypes)
{
    auto *const checker = relation->GetChecker()->AsETSChecker();

    auto *const copiedType = checker->CreateTypeParameter();
    copiedType->AddTypeFlag(TypeFlag::GENERIC);
    copiedType->SetDeclNode(GetDeclNode());
    copiedType->SetDefaultType(GetDefaultType());
    copiedType->SetConstraintType(GetConstraintType());
    copiedType->SetVariable(Variable());
    return copiedType;
}

Type *ETSTypeParameter::Substitute([[maybe_unused]] TypeRelation *relation, const Substitution *substitution)
{
    if (substitution == nullptr || substitution->empty()) {
        return this;
    }
    if (auto repl = substitution->find(GetOriginal()); repl != substitution->end()) {
        return repl->second;
    }
    return this;
}

void ETSTypeParameter::ToAssemblerType(std::stringstream &ss) const
{
    GetConstraintType()->ToAssemblerTypeWithRank(ss);
}

void ETSTypeParameter::ToDebugInfoType(std::stringstream &ss) const
{
    GetConstraintType()->ToDebugInfoType(ss);
}

ETSTypeParameter *ETSTypeParameter::GetOriginal() const
{
    return GetDeclNode()->Name()->Variable()->TsType()->AsETSTypeParameter();
}

}  // namespace ark::es2panda::checker
