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

#include "switchStatement.h"

#include "varbinder/scope.h"
#include "compiler/core/labelTarget.h"
#include "compiler/core/switchBuilder.h"
#include "compiler/core/pandagen.h"
#include "compiler/core/ETSGen.h"
#include "checker/TSchecker.h"

namespace panda::es2panda::ir {
void SwitchStatement::TransformChildren(const NodeTransformer &cb)
{
    discriminant_ = cb(discriminant_)->AsExpression();

    for (auto *&it : cases_) {
        it = cb(it)->AsSwitchCaseStatement();
    }
}

void SwitchStatement::Iterate(const NodeTraverser &cb) const
{
    cb(discriminant_);

    for (auto *it : cases_) {
        cb(it);
    }
}

void SwitchStatement::Dump(ir::AstDumper *dumper) const
{
    dumper->Add({{"type", "SwitchStatement"}, {"discriminant", discriminant_}, {"cases", cases_}});
}

void SwitchStatement::Compile(compiler::PandaGen *pg) const
{
    pg->GetAstCompiler()->Compile(this);
}

void SwitchStatement::Compile(compiler::ETSGen *etsg) const
{
    etsg->GetAstCompiler()->Compile(this);
}

checker::Type *SwitchStatement::Check(checker::TSChecker *checker)
{
    return checker->GetAnalyzer()->Check(this);
}

checker::Type *SwitchStatement::Check(checker::ETSChecker *const checker)
{
    return checker->GetAnalyzer()->Check(this);
}

void SwitchStatement::SetReturnType(checker::ETSChecker *checker, checker::Type *type)
{
    for (auto *cs : cases_) {
        if (cs != nullptr) {
            cs->SetReturnType(checker, type);
        }
    }
}
}  // namespace panda::es2panda::ir
