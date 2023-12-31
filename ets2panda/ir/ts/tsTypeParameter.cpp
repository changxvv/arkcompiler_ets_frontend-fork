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

#include "tsTypeParameter.h"

#include "ir/astDump.h"
#include "ir/typeNode.h"
#include "ir/expressions/identifier.h"

namespace panda::es2panda::ir {
void TSTypeParameter::TransformChildren(const NodeTransformer &cb)
{
    name_ = cb(name_)->AsIdentifier();

    if (constraint_ != nullptr) {
        constraint_ = static_cast<TypeNode *>(cb(constraint_));
    }

    if (default_type_ != nullptr) {
        default_type_ = static_cast<TypeNode *>(cb(default_type_));
    }
}

void TSTypeParameter::Iterate(const NodeTraverser &cb) const
{
    cb(name_);

    if (constraint_ != nullptr) {
        cb(constraint_);
    }

    if (default_type_ != nullptr) {
        cb(default_type_);
    }
}

void TSTypeParameter::Dump(ir::AstDumper *dumper) const
{
    dumper->Add({
        {"type", "TSTypeParameter"},
        {"name", name_},
        {"constraint", AstDumper::Optional(constraint_)},
        {"default", AstDumper::Optional(default_type_)},
        {"in", AstDumper::Optional(IsIn())},
        {"out", AstDumper::Optional(IsOut())},
    });
}

void TSTypeParameter::Compile([[maybe_unused]] compiler::PandaGen *pg) const {}

checker::Type *TSTypeParameter::Check([[maybe_unused]] checker::TSChecker *checker)
{
    return nullptr;
}

checker::Type *TSTypeParameter::Check([[maybe_unused]] checker::ETSChecker *checker)
{
    return nullptr;
}
}  // namespace panda::es2panda::ir
