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

#include "binaryExpression.h"

#include "compiler/core/pandagen.h"
#include "compiler/core/ETSGen.h"
#include "checker/TSchecker.h"

namespace panda::es2panda::ir {
void BinaryExpression::TransformChildren(const NodeTransformer &cb)
{
    left_ = cb(left_)->AsExpression();
    right_ = cb(right_)->AsExpression();
}

void BinaryExpression::Iterate(const NodeTraverser &cb) const
{
    cb(left_);
    cb(right_);
}

void BinaryExpression::Dump(ir::AstDumper *dumper) const
{
    dumper->Add({{"type", IsLogical() ? "LogicalExpression" : "BinaryExpression"},
                 {"operator", operator_},
                 {"left", left_},
                 {"right", right_}});
}

void BinaryExpression::Compile(compiler::PandaGen *pg) const
{
    pg->GetAstCompiler()->Compile(this);
}

void BinaryExpression::Compile(compiler::ETSGen *etsg) const
{
    etsg->GetAstCompiler()->Compile(this);
}

checker::Type *BinaryExpression::Check(checker::TSChecker *checker)
{
    return checker->GetAnalyzer()->Check(this);
}

checker::Type *BinaryExpression::Check(checker::ETSChecker *checker)
{
    return checker->GetAnalyzer()->Check(this);
}

// NOLINTNEXTLINE(google-default-arguments)
BinaryExpression *BinaryExpression::Clone(ArenaAllocator *const allocator, AstNode *const parent)
{
    auto *const left = left_ != nullptr ? left_->Clone(allocator)->AsExpression() : nullptr;
    auto *const right = right_ != nullptr ? right_->Clone(allocator)->AsExpression() : nullptr;

    if (auto *const clone = allocator->New<BinaryExpression>(left, right, operator_); clone != nullptr) {
        if (operation_type_ != nullptr) {
            clone->SetOperationType(operation_type_);
        }
        if (right != nullptr) {
            right->SetParent(clone);
        }
        if (left != nullptr) {
            left->SetParent(clone);
        }
        if (parent != nullptr) {
            clone->SetParent(parent);
        }
        return clone;
    }

    throw Error(ErrorType::GENERIC, "", CLONE_ALLOCATION_ERROR);
}
}  // namespace panda::es2panda::ir
