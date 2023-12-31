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

#include "tsPropertySignature.h"

#include "checker/TSchecker.h"
#include "compiler/core/ETSGen.h"
#include "compiler/core/pandagen.h"

namespace panda::es2panda::ir {
void TSPropertySignature::TransformChildren(const NodeTransformer &cb)
{
    key_ = cb(key_)->AsExpression();

    if (TypeAnnotation() != nullptr) {
        SetTsTypeAnnotation(static_cast<TypeNode *>(cb(TypeAnnotation())));
    }
}

void TSPropertySignature::Iterate(const NodeTraverser &cb) const
{
    cb(key_);

    if (TypeAnnotation() != nullptr) {
        cb(TypeAnnotation());
    }
}

void TSPropertySignature::Dump(ir::AstDumper *dumper) const
{
    dumper->Add({{"type", "TSPropertySignature"},
                 {"computed", computed_},
                 {"optional", optional_},
                 {"readonly", readonly_},
                 {"key", key_},
                 {"typeAnnotation", AstDumper::Optional(TypeAnnotation())}});
}

void TSPropertySignature::Compile(compiler::PandaGen *pg) const
{
    pg->GetAstCompiler()->Compile(this);
}

void TSPropertySignature::Compile(compiler::ETSGen *etsg) const
{
    etsg->GetAstCompiler()->Compile(this);
}

checker::Type *TSPropertySignature::Check(checker::TSChecker *checker)
{
    return checker->GetAnalyzer()->Check(this);
}

checker::Type *TSPropertySignature::Check(checker::ETSChecker *checker)
{
    return checker->GetAnalyzer()->Check(this);
}

// NOLINTNEXTLINE(google-default-arguments)
TSPropertySignature *TSPropertySignature::Clone(ArenaAllocator *const allocator, AstNode *const parent)
{
    auto *const key = key_ != nullptr ? key_->Clone(allocator)->AsExpression() : nullptr;
    auto *const type_annotation = TypeAnnotation()->Clone(allocator);

    if (auto *const clone = allocator->New<TSPropertySignature>(key, type_annotation, computed_, optional_, readonly_);
        clone != nullptr) {
        if (parent != nullptr) {
            clone->SetParent(parent);
        }
        if (key != nullptr) {
            key->SetParent(clone);
        }
        type_annotation->SetParent(clone);
        return clone;
    }

    throw Error(ErrorType::GENERIC, "", CLONE_ALLOCATION_ERROR);
}
}  // namespace panda::es2panda::ir
