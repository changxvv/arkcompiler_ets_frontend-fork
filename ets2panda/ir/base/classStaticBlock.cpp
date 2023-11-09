/*
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

#include "classStaticBlock.h"

#include "varbinder/scope.h"
#include "checker/ETSchecker.h"
#include "checker/TSchecker.h"
#include "compiler/core/ETSGen.h"
#include "compiler/core/pandagen.h"
#include "ir/astDump.h"
#include "ir/base/decorator.h"
#include "ir/base/scriptFunction.h"
#include "ir/expression.h"
#include "ir/expressions/identifier.h"
#include "ir/expressions/functionExpression.h"

#include <cstdint>
#include <string>

namespace panda::es2panda::ir {
void ClassStaticBlock::TransformChildren(const NodeTransformer &cb)
{
    value_ = cb(value_)->AsExpression();
}

void ClassStaticBlock::Iterate(const NodeTraverser &cb) const
{
    cb(value_);
}

void ClassStaticBlock::Dump(ir::AstDumper *dumper) const
{
    dumper->Add({{"type", "ClassStaticBlock"}, {"value", value_}});
}

void ClassStaticBlock::Compile(compiler::PandaGen *pg) const
{
    pg->GetAstCompiler()->Compile(this);
}

void ClassStaticBlock::Compile(compiler::ETSGen *etsg) const
{
    etsg->GetAstCompiler()->Compile(this);
}

checker::Type *ClassStaticBlock::Check(checker::TSChecker *checker)
{
    return checker->GetAnalyzer()->Check(this);
}

checker::Type *ClassStaticBlock::Check(checker::ETSChecker *checker)
{
    return checker->GetAnalyzer()->Check(this);
}

ir::ScriptFunction *ClassStaticBlock::Function()
{
    return value_->AsFunctionExpression()->Function();
}

const ir::ScriptFunction *ClassStaticBlock::Function() const
{
    return value_->AsFunctionExpression()->Function();
}

const util::StringView &ClassStaticBlock::Name() const
{
    return Function()->Id()->Name();
}

}  // namespace panda::es2panda::ir
