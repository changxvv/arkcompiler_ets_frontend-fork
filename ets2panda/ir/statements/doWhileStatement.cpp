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

#include "doWhileStatement.h"

#include "varbinder/scope.h"
#include "compiler/base/condition.h"
#include "compiler/core/labelTarget.h"
#include "compiler/core/pandagen.h"
#include "compiler/core/ETSGen.h"
#include "checker/TSchecker.h"

namespace panda::es2panda::ir {
void DoWhileStatement::TransformChildren(const NodeTransformer &cb)
{
    body_ = cb(body_)->AsStatement();
    test_ = cb(test_)->AsExpression();
}

void DoWhileStatement::Iterate(const NodeTraverser &cb) const
{
    cb(body_);
    cb(test_);
}

void DoWhileStatement::Dump(ir::AstDumper *dumper) const
{
    dumper->Add({{"type", "DoWhileStatement"}, {"body", body_}, {"test", test_}});
}

void DoWhileStatement::Compile(compiler::PandaGen *pg) const
{
    pg->GetAstCompiler()->Compile(this);
}

void DoWhileStatement::Compile(compiler::ETSGen *etsg) const
{
    etsg->GetAstCompiler()->Compile(this);
}

checker::Type *DoWhileStatement::Check(checker::TSChecker *checker)
{
    return checker->GetAnalyzer()->Check(this);
}

checker::Type *DoWhileStatement::Check(checker::ETSChecker *checker)
{
    return checker->GetAnalyzer()->Check(this);
}
}  // namespace panda::es2panda::ir
