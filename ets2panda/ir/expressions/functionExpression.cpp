/**
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

#include "functionExpression.h"

#include "checker/TSchecker.h"
#include "compiler/core/ETSGen.h"
#include "compiler/core/pandagen.h"
#include "ir/astDump.h"

namespace panda::es2panda::ir {
void FunctionExpression::TransformChildren(const NodeTransformer &cb)
{
    func_ = cb(func_)->AsScriptFunction();
}

void FunctionExpression::Iterate(const NodeTraverser &cb) const
{
    cb(func_);
}

void FunctionExpression::Dump(ir::AstDumper *dumper) const
{
    dumper->Add({{"type", "FunctionExpression"}, {"function", func_}});
}

void FunctionExpression::Compile(compiler::PandaGen *pg) const
{
    pg->GetAstCompiler()->Compile(this);
}

void FunctionExpression::Compile(compiler::ETSGen *etsg) const
{
    etsg->GetAstCompiler()->Compile(this);
}

checker::Type *FunctionExpression::Check(checker::TSChecker *checker)
{
    return checker->GetAnalyzer()->Check(this);
}

checker::Type *FunctionExpression::Check([[maybe_unused]] checker::ETSChecker *checker)
{
    return checker->GetAnalyzer()->Check(this);
}

// NOLINTNEXTLINE(google-default-arguments)
FunctionExpression *FunctionExpression::Clone(ArenaAllocator *const allocator, AstNode *const parent)
{
    auto *const func = func_->Clone(allocator)->AsScriptFunction();

    if (auto *const clone = allocator->New<FunctionExpression>(func); clone != nullptr) {
        func->SetParent(clone);
        if (parent != nullptr) {
            clone->SetParent(parent);
        }
        return clone;
    }

    throw Error(ErrorType::GENERIC, "", CLONE_ALLOCATION_ERROR);
}
}  // namespace panda::es2panda::ir
