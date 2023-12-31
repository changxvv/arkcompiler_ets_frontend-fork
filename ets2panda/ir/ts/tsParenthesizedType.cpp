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

#include "tsParenthesizedType.h"

#include "ir/astDump.h"
#include "checker/TSchecker.h"

namespace panda::es2panda::ir {
void TSParenthesizedType::TransformChildren(const NodeTransformer &cb)
{
    type_ = static_cast<TypeNode *>(cb(type_));
}

void TSParenthesizedType::Iterate(const NodeTraverser &cb) const
{
    cb(type_);
}

void TSParenthesizedType::Dump(ir::AstDumper *dumper) const
{
    dumper->Add({{"type", "TSParenthesizedType"}, {"typeAnnotation", type_}});
}

void TSParenthesizedType::Compile([[maybe_unused]] compiler::PandaGen *pg) const {}

checker::Type *TSParenthesizedType::Check([[maybe_unused]] checker::TSChecker *checker)
{
    type_->Check(checker);
    return nullptr;
}

checker::Type *TSParenthesizedType::GetType([[maybe_unused]] checker::TSChecker *checker)
{
    if (TsType() != nullptr) {
        return TsType();
    }

    SetTsType(type_->GetType(checker));
    return TsType();
}

checker::Type *TSParenthesizedType::Check([[maybe_unused]] checker::ETSChecker *checker)
{
    return nullptr;
}
}  // namespace panda::es2panda::ir
