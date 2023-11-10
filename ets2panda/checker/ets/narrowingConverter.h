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

#ifndef ES2PANDA_COMPILER_CHECKER_ETS_NARROWING_CONVERTER_H
#define ES2PANDA_COMPILER_CHECKER_ETS_NARROWING_CONVERTER_H

#include "checker/ets/typeConverter.h"
#include "checker/ETSchecker.h"
#include "util/helpers.h"

namespace panda::es2panda::checker {
class NarrowingConverter : public TypeConverter {
public:
    explicit NarrowingConverter(ETSChecker *checker, TypeRelation *relation, Type *target, Type *source)
        : TypeConverter(checker, relation, target, source)
    {
        if (!relation->ApplyNarrowing()) {
            return;
        }

        ASSERT(relation->GetNode());

        switch (ETSChecker::ETSChecker::ETSType(target)) {
            case TypeFlag::BYTE: {
                ApplyNarrowing<ByteType>(TypeFlag::NARROWABLE_TO_BYTE);
                break;
            }
            case TypeFlag::CHAR: {
                ApplyNarrowing<CharType>(TypeFlag::NARROWABLE_TO_CHAR);
                break;
            }
            case TypeFlag::SHORT: {
                ApplyNarrowing<ShortType>(TypeFlag::NARROWABLE_TO_SHORT);
                break;
            }
            case TypeFlag::INT: {
                ApplyNarrowing<IntType>(TypeFlag::NARROWABLE_TO_INT);
                break;
            }
            case TypeFlag::LONG: {
                ApplyNarrowing<LongType>(TypeFlag::NARROWABLE_TO_LONG);
                break;
            }
            case TypeFlag::FLOAT: {
                ApplyNarrowing<FloatType>(TypeFlag::NARROWABLE_TO_FLOAT);
                break;
            }

            default: {
                break;
            }
        }
    }

private:
    template <typename TargetType>
    void ApplyNarrowing(TypeFlag flag)
    {
        if (!Source()->HasTypeFlag(flag)) {
            return;
        }

        switch (ETSChecker::ETSChecker::ETSType(Source())) {
            case TypeFlag::CHAR: {
                ApplyNarrowing<TargetType, CharType>();
                break;
            }
            case TypeFlag::SHORT: {
                ApplyNarrowing<TargetType, ShortType>();
                break;
            }
            case TypeFlag::INT: {
                ApplyNarrowing<TargetType, IntType>();
                break;
            }
            case TypeFlag::LONG: {
                ApplyNarrowing<TargetType, LongType>();
                break;
            }
            case TypeFlag::FLOAT: {
                ApplyNarrowing<TargetType, FloatType>();
                break;
            }
            case TypeFlag::DOUBLE: {
                ApplyNarrowing<TargetType, DoubleType>();
                break;
            }
            default: {
                break;
            }
        }
    }

    template <typename TargetType, typename SourceType>
    void ApplyNarrowing()
    {
        using SType = typename SourceType::UType;
        using TType = typename TargetType::UType;

        if (Source()->HasTypeFlag(TypeFlag::CONSTANT)) {
            SType value = reinterpret_cast<SourceType *>(Source())->GetValue();
            if (Relation()->InCastingContext() || util::Helpers::IsTargetFitInSourceRange<TType, SType>(value)) {
                Relation()->GetNode()->SetTsType(Checker()->Allocator()->New<TargetType>(static_cast<TType>(value)));
                Relation()->Result(true);
                return;
            }

            Relation()->Result(RelationResult::ERROR);
            return;
        }

        Relation()->Result(true);
    }
};
}  // namespace panda::es2panda::checker

#endif
