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

#include "labelledStatement.h"

#include "checker/TSchecker.h"
#include "compiler/core/ETSGen.h"
#include "compiler/core/pandagen.h"

namespace panda::es2panda::ir {
void LabelledStatement::TransformChildren(const NodeTransformer &cb)
{
    ident_ = cb(ident_)->AsIdentifier();
    body_ = cb(body_)->AsStatement();
}

void LabelledStatement::Iterate(const NodeTraverser &cb) const
{
    cb(ident_);
    cb(body_);
}

void LabelledStatement::Dump(ir::AstDumper *dumper) const
{
    dumper->Add({{"type", "LabelledStatement"}, {"label", ident_}, {"body", body_}});
}

const ir::AstNode *LabelledStatement::GetReferencedStatement() const
{
    const auto *iter = body_;
    while (iter->IsLabelledStatement()) {
        iter = iter->AsLabelledStatement()->Body();
    }

    switch (iter->Type()) {
        case ir::AstNodeType::DO_WHILE_STATEMENT:
        case ir::AstNodeType::SWITCH_STATEMENT:
        case ir::AstNodeType::FOR_UPDATE_STATEMENT:
        case ir::AstNodeType::FOR_IN_STATEMENT:
        case ir::AstNodeType::WHILE_STATEMENT: {
            return iter;
        }
        default: {
            return this;
        }
    }
}

void LabelledStatement::Compile(compiler::PandaGen *pg) const
{
    pg->GetAstCompiler()->Compile(this);
}

void LabelledStatement::Compile(compiler::ETSGen *etsg) const
{
    etsg->GetAstCompiler()->Compile(this);
}

checker::Type *LabelledStatement::Check(checker::TSChecker *checker)
{
    return checker->GetAnalyzer()->Check(this);
}

checker::Type *LabelledStatement::Check(checker::ETSChecker *checker)
{
    return checker->GetAnalyzer()->Check(this);
}
}  // namespace panda::es2panda::ir
