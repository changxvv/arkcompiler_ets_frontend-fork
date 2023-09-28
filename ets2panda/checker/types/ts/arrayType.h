/**
 * Copyright (c) 2021-2022 Huawei Device Co., Ltd.
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

#ifndef ES2PANDA_COMPILER_CHECKER_TYPES_TS_ARRAY_TYPE_H
#define ES2PANDA_COMPILER_CHECKER_TYPES_TS_ARRAY_TYPE_H

#include "plugins/ecmascript/es2panda/checker/types/type.h"

namespace panda::es2panda::checker {
class ArrayType : public Type {
public:
    explicit ArrayType(Type *element_type) : Type(TypeFlag::ARRAY), element_(element_type) {}

    Type *ElementType()
    {
        return element_;
    }

    const Type *ElementType() const
    {
        return element_;
    }

    void ToString(std::stringstream &ss) const override;
    void Identical(TypeRelation *relation, Type *other) override;
    void AssignmentTarget(TypeRelation *relation, Type *source) override;
    TypeFacts GetTypeFacts() const override;
    Type *Instantiate(ArenaAllocator *allocator, TypeRelation *relation, GlobalTypesHolder *global_types) override;

private:
    Type *element_;
};
}  // namespace panda::es2panda::checker

#endif /* TYPESCRIPT_TYPES_ARRAY_TYPE_H */
