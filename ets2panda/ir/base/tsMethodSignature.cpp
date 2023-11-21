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

#include "tsMethodSignature.h"

#include "varbinder/scope.h"
#include "checker/TSchecker.h"
#include "compiler/core/ETSGen.h"
#include "compiler/core/pandagen.h"

namespace panda::es2panda::ir {
void TSMethodSignature::TransformChildren(const NodeTransformer &cb)
{
    key_ = cb(key_)->AsExpression();
    signature_.TransformChildren(cb);
}

void TSMethodSignature::Iterate(const NodeTraverser &cb) const
{
    cb(key_);
    signature_.Iterate(cb);
}

void TSMethodSignature::Dump(ir::AstDumper *dumper) const
{
    dumper->Add({{"type", "TSMethodSignature"},
                 {"computed", computed_},
                 {"optional", optional_},
                 {"key", key_},
                 {"params", Params()},
                 {"typeParameters", AstDumper::Optional(TypeParams())},
                 {"typeAnnotation", AstDumper::Optional(ReturnTypeAnnotation())}});
}

void TSMethodSignature::Compile(compiler::PandaGen *pg) const
{
    pg->GetAstCompiler()->Compile(this);
}

void TSMethodSignature::Compile(compiler::ETSGen *etsg) const
{
    etsg->GetAstCompiler()->Compile(this);
}

checker::Type *TSMethodSignature::Check(checker::TSChecker *checker)
{
    return checker->GetAnalyzer()->Check(this);
}

checker::Type *TSMethodSignature::Check(checker::ETSChecker *checker)
{
    return checker->GetAnalyzer()->Check(this);
}
}  // namespace panda::es2panda::ir
