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

#ifndef ES2PANDA_COMPILER_CHECKER_TYPES_ETS_FLOAT_TYPE_H
#define ES2PANDA_COMPILER_CHECKER_TYPES_ETS_FLOAT_TYPE_H

#include "checker/types/type.h"

namespace panda::es2panda::checker {
class FloatType : public Type {
public:
    using UType = float;

    FloatType() : Type(TypeFlag::FLOAT) {}
    explicit FloatType(UType value) : Type(TypeFlag::FLOAT | TypeFlag::CONSTANT), value_(value) {}

    UType GetValue() const
    {
        return value_;
    }

    void Identical(TypeRelation *relation, Type *other) override;
    void AssignmentTarget(TypeRelation *relation, Type *source) override;
    bool AssignmentSource([[maybe_unused]] TypeRelation *relation, [[maybe_unused]] Type *target) override;
    void Cast(TypeRelation *relation, Type *target) override;
    Type *Instantiate(ArenaAllocator *allocator, TypeRelation *relation, GlobalTypesHolder *global_types) override;

    void ToString(std::stringstream &ss) const override
    {
        ss << "float";
    }

    void ToAssemblerType([[maybe_unused]] std::stringstream &ss) const override
    {
        ss << compiler::Signatures::PRIMITIVE_FLOAT;
    }

    void ToDebugInfoType(std::stringstream &ss) const override
    {
        ss << compiler::Signatures::TYPE_DESCRIPTOR_FLOAT;
    }

    std::tuple<bool, bool> ResolveConditionExpr() const override
    {
        return {IsConstantType(), value_ != 0};
    }

private:
    UType value_ {0.0};
};
}  // namespace panda::es2panda::checker

#endif
