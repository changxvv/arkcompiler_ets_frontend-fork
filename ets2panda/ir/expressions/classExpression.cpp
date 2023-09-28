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

#include "classExpression.h"

#include "plugins/ecmascript/es2panda/ir/astDump.h"
#include "plugins/ecmascript/es2panda/ir/base/classDefinition.h"

namespace panda::es2panda::ir {
void ClassExpression::Iterate(const NodeTraverser &cb) const
{
    cb(def_);
}

void ClassExpression::Dump(ir::AstDumper *dumper) const
{
    dumper->Add({{"type", "ClassExpression"}, {"definition", def_}});
}

void ClassExpression::Compile([[maybe_unused]] compiler::PandaGen *pg) const
{
    def_->Compile(pg);
}

checker::Type *ClassExpression::Check([[maybe_unused]] checker::TSChecker *checker)
{
    return nullptr;
}

checker::Type *ClassExpression::Check([[maybe_unused]] checker::ETSChecker *checker)
{
    return nullptr;
}
}  // namespace panda::es2panda::ir
