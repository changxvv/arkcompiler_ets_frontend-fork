/**
 * Copyright (c) 2021-2024 Huawei Device Co., Ltd.
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

#include "unaryExpression.h"

#include "compiler/core/pandagen.h"
#include "compiler/core/ETSGen.h"
#include "checker/TSchecker.h"
#include "checker/ETSchecker.h"
#include "ir/astDump.h"

namespace ark::es2panda::ir {
void UnaryExpression::TransformChildren(const NodeTransformer &cb)
{
    argument_ = cb(argument_)->AsExpression();
}

void UnaryExpression::Iterate(const NodeTraverser &cb) const
{
    cb(argument_);
}

void UnaryExpression::Dump(ir::AstDumper *dumper) const
{
    dumper->Add({{"type", "UnaryExpression"}, {"operator", operator_}, {"prefix", true}, {"argument", argument_}});
}

void UnaryExpression::Dump(ir::SrcDumper *dumper) const
{
    dumper->Add(TokenToString(operator_));
    argument_->Dump(dumper);
}

void UnaryExpression::Compile(compiler::PandaGen *pg) const
{
    pg->GetAstCompiler()->Compile(this);
}

void UnaryExpression::Compile([[maybe_unused]] compiler::ETSGen *etsg) const
{
    etsg->GetAstCompiler()->Compile(this);
}

checker::Type *UnaryExpression::Check([[maybe_unused]] checker::TSChecker *checker)
{
    return checker->GetAnalyzer()->Check(this);
}

checker::Type *UnaryExpression::Check(checker::ETSChecker *checker)
{
    return checker->GetAnalyzer()->Check(this);
}

UnaryExpression *UnaryExpression::Clone(ArenaAllocator *const allocator, AstNode *const parent)
{
    auto *const argument = argument_ != nullptr ? argument_->Clone(allocator, nullptr)->AsExpression() : nullptr;

    if (auto *const clone = allocator->New<UnaryExpression>(argument, operator_); clone != nullptr) {
        if (argument != nullptr) {
            argument->SetParent(clone);
        }

        if (parent != nullptr) {
            clone->SetParent(parent);
        }

        clone->SetRange(Range());
        return clone;
    }

    throw Error(ErrorType::GENERIC, "", CLONE_ALLOCATION_ERROR);
}

void UnaryExpression::CheckSmartCastCondition() noexcept
{
    if (argument_ != nullptr && operator_ == lexer::TokenType::PUNCTUATOR_EXCLAMATION_MARK) {
        if (argument_->IsIdentifier()) {
            smartCastCondition_ = argument_->AsIdentifier()->GetSmartCastCondition();
        } else if (argument_->IsBinaryExpression()) {
            auto const &smartCastCondition = argument_->AsBinaryExpression()->GetSmartCastCondition();
            if (smartCastCondition.has_value() &&
                (smartCastCondition->testedType->DefinitelyETSNullish() ||
                 argument_->AsBinaryExpression()->OperatorType() == lexer::TokenType::KEYW_INSTANCEOF)) {
                smartCastCondition_ = smartCastCondition;
            }
        } else if (argument_->IsUnaryExpression()) {
            smartCastCondition_ = argument_->AsUnaryExpression()->GetSmartCastCondition();
        }

        if (smartCastCondition_.has_value()) {
            smartCastCondition_->negate = !smartCastCondition_->negate;
        }
    }
}

}  // namespace ark::es2panda::ir
