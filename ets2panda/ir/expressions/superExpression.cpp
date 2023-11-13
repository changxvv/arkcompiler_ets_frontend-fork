/**
 * Copyright (c) 2021-2023 Huawei Device Co., Ltd.
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

#include "superExpression.h"

#include "checker/ETSchecker.h"
#include "checker/TSchecker.h"
#include "compiler/core/ETSGen.h"
#include "compiler/core/pandagen.h"
#include "util/helpers.h"
#include "ir/astDump.h"
#include "ir/srcDump.h"

namespace panda::es2panda::ir {
void SuperExpression::TransformChildren([[maybe_unused]] const NodeTransformer &cb) {}
void SuperExpression::Iterate([[maybe_unused]] const NodeTraverser &cb) const {}

void SuperExpression::Dump(ir::AstDumper *dumper) const
{
    dumper->Add({{"type", "Super"}});
}

void SuperExpression::Dump(ir::SrcDumper *dumper) const
{
    dumper->Add("super");
}

void SuperExpression::Compile(compiler::PandaGen *pg) const
{
    pg->GetAstCompiler()->Compile(this);
}

void SuperExpression::Compile(compiler::ETSGen *etsg) const
{
    etsg->GetAstCompiler()->Compile(this);
}

checker::Type *SuperExpression::Check(checker::TSChecker *checker)
{
    return checker->GetAnalyzer()->Check(this);
}

checker::Type *SuperExpression::Check([[maybe_unused]] checker::ETSChecker *checker)
{
    return checker->GetAnalyzer()->Check(this);
}

// NOLINTNEXTLINE(google-default-arguments)
SuperExpression *SuperExpression::Clone(ArenaAllocator *const allocator, AstNode *const parent)
{
    if (auto *const clone = allocator->New<SuperExpression>(); clone != nullptr) {
        if (parent != nullptr) {
            clone->SetParent(parent);
        }
        return clone;
    }
    throw Error(ErrorType::GENERIC, "", CLONE_ALLOCATION_ERROR);
}
}  // namespace panda::es2panda::ir
