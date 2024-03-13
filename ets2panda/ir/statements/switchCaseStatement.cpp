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

#include "switchCaseStatement.h"

#include "checker/TSchecker.h"
#include "compiler/core/ETSGen.h"
#include "compiler/core/pandagen.h"
#include "checker/ets/typeRelationContext.h"
#include "ir/astDump.h"
#include "ir/srcDump.h"

namespace ark::es2panda::ir {
void SwitchCaseStatement::TransformChildren(const NodeTransformer &cb)
{
    if (test_ != nullptr) {
        test_ = cb(test_)->AsExpression();
    }

    for (auto *&it : consequent_) {
        it = cb(it)->AsStatement();
    }
}

void SwitchCaseStatement::Iterate(const NodeTraverser &cb) const
{
    if (test_ != nullptr) {
        cb(test_);
    }

    for (auto *it : consequent_) {
        cb(it);
    }
}

void SwitchCaseStatement::Dump(ir::AstDumper *dumper) const
{
    dumper->Add({{"type", "SwitchCase"}, {"test", AstDumper::Nullish(test_)}, {"consequent", consequent_}});
}

void SwitchCaseStatement::Dump(ir::SrcDumper *dumper) const
{
    if (test_ != nullptr) {
        dumper->Add("case ");
        test_->Dump(dumper);
        dumper->Add(":");
    } else {
        dumper->Add("default:");
    }
    if (!consequent_.empty()) {
        dumper->IncrIndent();
        dumper->Endl();
        for (auto cs : consequent_) {
            cs->Dump(dumper);
        }
        dumper->DecrIndent();
    }
}

void SwitchCaseStatement::Compile(compiler::PandaGen *pg) const
{
    pg->GetAstCompiler()->Compile(this);
}

void SwitchCaseStatement::Compile(compiler::ETSGen *etsg) const
{
    etsg->GetAstCompiler()->Compile(this);
}

checker::Type *SwitchCaseStatement::Check(checker::TSChecker *checker)
{
    return checker->GetAnalyzer()->Check(this);
}

checker::Type *SwitchCaseStatement::Check(checker::ETSChecker *checker)
{
    return checker->GetAnalyzer()->Check(this);
}

// Auxilary function extracted from the 'Check' method for 'SwitchStatement' to reduce function's size.
void SwitchCaseStatement::CheckAndTestCase(checker::ETSChecker *checker, checker::Type *comparedExprType,
                                           checker::Type *unboxedDiscType, ir::Expression *node, bool &isDefaultCase)
{
    if (test_ != nullptr) {
        auto *caseType = test_->Check(checker);
        bool validCaseType = true;

        if (caseType->HasTypeFlag(checker::TypeFlag::CHAR)) {
            validCaseType = comparedExprType->HasTypeFlag(checker::TypeFlag::ETS_INTEGRAL);
        } else if (caseType->IsETSEnumType() && comparedExprType->IsETSEnumType()) {
            validCaseType = comparedExprType->AsETSEnumType()->IsSameEnumType(caseType->AsETSEnumType());
        } else if (caseType->IsETSStringEnumType() && comparedExprType->IsETSStringEnumType()) {
            validCaseType = comparedExprType->AsETSStringEnumType()->IsSameEnumType(caseType->AsETSStringEnumType());
        } else {
            checker::AssignmentContext(
                checker->Relation(), node, caseType, unboxedDiscType, test_->Start(),
                {"Switch case type '", caseType, "' is not comparable to discriminant type '", comparedExprType, "'"},
                (comparedExprType->IsETSObjectType() ? checker::TypeRelationFlag::NO_WIDENING
                                                     : checker::TypeRelationFlag::NO_UNBOXING) |
                    checker::TypeRelationFlag::NO_BOXING);
        }

        if (!validCaseType) {
            checker->ThrowTypeError(
                {"Switch case type '", caseType, "' is not comparable to discriminant type '", comparedExprType, "'"},
                test_->Start());
        }
    } else {
        isDefaultCase = true;
    }

    for (auto *caseStmt : consequent_) {
        caseStmt->Check(checker);
    }
}
}  // namespace ark::es2panda::ir
