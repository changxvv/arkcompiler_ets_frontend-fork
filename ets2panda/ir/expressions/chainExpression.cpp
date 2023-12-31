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

#include "chainExpression.h"

#include "checker/TSchecker.h"
#include "compiler/core/ETSGen.h"
#include "compiler/core/pandagen.h"
#include "ir/astDump.h"
#include "ir/expressions/memberExpression.h"

namespace panda::es2panda::ir {
void ChainExpression::TransformChildren(const NodeTransformer &cb)
{
    expression_ = cb(expression_)->AsExpression();
}

void ChainExpression::Iterate(const NodeTraverser &cb) const
{
    cb(expression_);
}

void ChainExpression::Dump(ir::AstDumper *dumper) const
{
    dumper->Add({{"type", "ChainExpression"}, {"expression", expression_}});
}

void ChainExpression::Compile(compiler::PandaGen *pg) const
{
    pg->GetAstCompiler()->Compile(this);
}

void ChainExpression::CompileToReg(compiler::PandaGen *pg, compiler::VReg &obj_reg) const
{
    compiler::OptionalChain chain(pg, this);

    if (expression_->IsMemberExpression()) {
        obj_reg = pg->AllocReg();
        expression_->AsMemberExpression()->CompileToReg(pg, obj_reg);
    } else {
        obj_reg = compiler::VReg::Invalid();
        expression_->Compile(pg);
    }
}

void ChainExpression::Compile(compiler::ETSGen *etsg) const
{
    etsg->GetAstCompiler()->Compile(this);
}

checker::Type *ChainExpression::Check(checker::TSChecker *checker)
{
    return checker->GetAnalyzer()->Check(this);
}

checker::Type *ChainExpression::Check(checker::ETSChecker *checker)
{
    return checker->GetAnalyzer()->Check(this);
}

// NOLINTNEXTLINE(google-default-arguments)
ChainExpression *ChainExpression::Clone(ArenaAllocator *const allocator, AstNode *const parent)
{
    auto *const expression = expression_ != nullptr ? expression_->Clone(allocator)->AsExpression() : nullptr;

    if (auto *const clone = allocator->New<ChainExpression>(expression); clone != nullptr) {
        if (expression != nullptr) {
            expression->SetParent(clone);
        }
        if (parent != nullptr) {
            clone->SetParent(parent);
        }
        return clone;
    }

    throw Error(ErrorType::GENERIC, "", CLONE_ALLOCATION_ERROR);
}
}  // namespace panda::es2panda::ir
