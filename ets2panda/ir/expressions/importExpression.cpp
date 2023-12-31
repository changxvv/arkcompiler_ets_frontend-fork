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

#include "importExpression.h"

#include "compiler/core/pandagen.h"
#include "ir/astDump.h"

namespace panda::es2panda::ir {
void ImportExpression::TransformChildren(const NodeTransformer &cb)
{
    source_ = cb(source_)->AsExpression();
}

void ImportExpression::Iterate(const NodeTraverser &cb) const
{
    cb(source_);
}

void ImportExpression::Dump(ir::AstDumper *dumper) const
{
    dumper->Add({{"type", "ImportExpression"}, {"source", source_}});
}

void ImportExpression::Compile([[maybe_unused]] compiler::PandaGen *pg) const
{
    pg->Unimplemented();
}

checker::Type *ImportExpression::Check([[maybe_unused]] checker::TSChecker *checker)
{
    return nullptr;
}

checker::Type *ImportExpression::Check([[maybe_unused]] checker::ETSChecker *checker)
{
    return nullptr;
}

// NOLINTNEXTLINE(google-default-arguments)
ImportExpression *ImportExpression::Clone(ArenaAllocator *const allocator, AstNode *const parent)
{
    auto *const source = source_ != nullptr ? source_->Clone(allocator)->AsExpression() : nullptr;

    if (auto *const clone = allocator->New<ImportExpression>(source); clone != nullptr) {
        if (source != nullptr) {
            source->SetParent(clone);
        }
        if (parent != nullptr) {
            clone->SetParent(parent);
        }
        return clone;
    }

    throw Error(ErrorType::GENERIC, "", CLONE_ALLOCATION_ERROR);
}
}  // namespace panda::es2panda::ir
