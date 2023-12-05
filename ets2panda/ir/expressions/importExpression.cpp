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

#include "checker/TSchecker.h"
#include "compiler/core/ETSGen.h"
#include "compiler/core/pandagen.h"

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

void ImportExpression::Compile(compiler::PandaGen *pg) const
{
    pg->GetAstCompiler()->Compile(this);
}

void ImportExpression::Compile(compiler::ETSGen *etsg) const
{
    etsg->GetAstCompiler()->Compile(this);
}
checker::Type *ImportExpression::Check(checker::TSChecker *checker)
{
    return checker->GetAnalyzer()->Check(this);
}

checker::Type *ImportExpression::Check(checker::ETSChecker *checker)
{
    return checker->GetAnalyzer()->Check(this);
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
