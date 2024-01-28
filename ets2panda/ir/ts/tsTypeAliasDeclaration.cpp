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

#include "tsTypeAliasDeclaration.h"
#include <cstddef>

#include "macros.h"
#include "varbinder/scope.h"
#include "checker/TSchecker.h"
#include "compiler/core/ETSGen.h"
#include "compiler/core/pandagen.h"
#include "ir/astDump.h"
#include "ir/srcDump.h"
#include "ir/typeNode.h"
#include "ir/base/decorator.h"
#include "ir/expressions/identifier.h"
#include "ir/ts/tsTypeParameter.h"
#include "ir/ts/tsTypeParameterDeclaration.h"

namespace ark::es2panda::ir {
void TSTypeAliasDeclaration::TransformChildren(const NodeTransformer &cb)
{
    for (auto *&it : decorators_) {
        it = cb(it)->AsDecorator();
    }

    id_ = cb(id_)->AsIdentifier();

    if (typeParams_ != nullptr) {
        typeParams_ = cb(typeParams_)->AsTSTypeParameterDeclaration();
    }

    if (TypeAnnotation() != nullptr) {
        SetTsTypeAnnotation(static_cast<TypeNode *>(cb(TypeAnnotation())));
    }
}

void TSTypeAliasDeclaration::Iterate(const NodeTraverser &cb) const
{
    for (auto *it : decorators_) {
        cb(it);
    }

    cb(id_);

    if (typeParams_ != nullptr) {
        cb(typeParams_);
    }

    if (TypeAnnotation() != nullptr) {
        cb(TypeAnnotation());
    }
}

void TSTypeAliasDeclaration::Dump(ir::AstDumper *dumper) const
{
    dumper->Add({{"type", "TSTypeAliasDeclaration"},
                 {"decorators", AstDumper::Optional(decorators_)},
                 {"id", id_},
                 {"typeAnnotation", TypeAnnotation()},
                 {"typeParameters", AstDumper::Optional(typeParams_)},
                 {"declare", AstDumper::Optional(declare_)}});
}

void TSTypeAliasDeclaration::Dump(ir::SrcDumper *dumper) const
{
    ASSERT(id_);
    dumper->Add("type ");
    id_->Dump(dumper);
    if (typeParams_ != nullptr) {
        dumper->Add("<");
        typeParams_->Dump(dumper);
        dumper->Add(">");
    }
    dumper->Add(" = ");
    if (id_->IsAnnotatedExpression()) {
        auto type = TypeAnnotation();
        ASSERT(type);
        type->Dump(dumper);
    }
    dumper->Add(";");
    dumper->Endl();
}

void TSTypeAliasDeclaration::Compile([[maybe_unused]] compiler::PandaGen *pg) const
{
    pg->GetAstCompiler()->Compile(this);
}

void TSTypeAliasDeclaration::Compile(compiler::ETSGen *etsg) const
{
    etsg->GetAstCompiler()->Compile(this);
}

checker::Type *TSTypeAliasDeclaration::Check([[maybe_unused]] checker::TSChecker *checker)
{
    return checker->GetAnalyzer()->Check(this);
}

checker::Type *TSTypeAliasDeclaration::Check([[maybe_unused]] checker::ETSChecker *checker)
{
    return checker->GetAnalyzer()->Check(this);
}

}  // namespace ark::es2panda::ir
