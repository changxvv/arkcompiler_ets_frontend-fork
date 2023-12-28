/*
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

#include "opaqueTypeNode.h"
#include "checker/TSchecker.h"
#include "compiler/core/ETSGen.h"
#include "compiler/core/pandagen.h"
#include "ir/astDump.h"
#include "ir/srcDump.h"

namespace panda::es2panda::ir {

void OpaqueTypeNode::TransformChildren([[maybe_unused]] const NodeTransformer &cb) {}
void OpaqueTypeNode::Iterate([[maybe_unused]] const NodeTraverser &cb) const {}

void OpaqueTypeNode::Dump(ir::AstDumper *dumper) const
{
    dumper->Add({{"type", "OpaqueType"}});
}

void OpaqueTypeNode::Dump(ir::SrcDumper *dumper) const
{
    dumper->Add("OpaqueTypeNode");
}

void OpaqueTypeNode::Compile([[maybe_unused]] compiler::PandaGen *pg) const
{
    pg->GetAstCompiler()->Compile(this);
}

void OpaqueTypeNode::Compile([[maybe_unused]] compiler::ETSGen *etsg) const
{
    etsg->GetAstCompiler()->Compile(this);
}

checker::Type *OpaqueTypeNode::Check([[maybe_unused]] checker::TSChecker *checker)
{
    return checker->GetAnalyzer()->Check(this);
}

checker::Type *OpaqueTypeNode::GetType([[maybe_unused]] checker::TSChecker *checker)
{
    return nullptr;
}

checker::Type *OpaqueTypeNode::GetType([[maybe_unused]] checker::ETSChecker *checker)
{
    return TsType();
}

checker::Type *OpaqueTypeNode::Check([[maybe_unused]] checker::ETSChecker *checker)
{
    return checker->GetAnalyzer()->Check(this);
}
}  // namespace panda::es2panda::ir
