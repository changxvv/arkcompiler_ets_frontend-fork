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

#include "tsConditionalType.h"

#include "ir/astDump.h"

namespace panda::es2panda::ir {
void TSConditionalType::TransformChildren(const NodeTransformer &cb)
{
    check_type_ = cb(check_type_)->AsExpression();
    extends_type_ = cb(extends_type_)->AsExpression();
    true_type_ = cb(true_type_)->AsExpression();
    false_type_ = cb(false_type_)->AsExpression();
}

void TSConditionalType::Iterate(const NodeTraverser &cb) const
{
    cb(check_type_);
    cb(extends_type_);
    cb(true_type_);
    cb(false_type_);
}

void TSConditionalType::Dump(ir::AstDumper *dumper) const
{
    dumper->Add({{"type", "TSConditionalType"},
                 {"checkType", check_type_},
                 {"extendsType", extends_type_},
                 {"trueType", true_type_},
                 {"falseType", false_type_}});
}

void TSConditionalType::Compile([[maybe_unused]] compiler::PandaGen *pg) const {}

checker::Type *TSConditionalType::Check([[maybe_unused]] checker::TSChecker *checker)
{
    return nullptr;
}

checker::Type *TSConditionalType::GetType([[maybe_unused]] checker::TSChecker *checker)
{
    return nullptr;
}

checker::Type *TSConditionalType::Check([[maybe_unused]] checker::ETSChecker *checker)
{
    return nullptr;
}
}  // namespace panda::es2panda::ir
