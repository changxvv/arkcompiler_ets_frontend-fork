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

#include "stringLiteral.h"

#include "compiler/core/pandagen.h"
#include "compiler/core/ETSGen.h"
#include "checker/TSchecker.h"
#include "checker/ETSchecker.h"
#include "ir/astDump.h"

namespace panda::es2panda::ir {
void StringLiteral::TransformChildren([[maybe_unused]] const NodeTransformer &cb) {}
void StringLiteral::Iterate([[maybe_unused]] const NodeTraverser &cb) const {}

void StringLiteral::Dump(ir::AstDumper *dumper) const
{
    dumper->Add({{"type", "StringLiteral"}, {"value", str_}});
}

void StringLiteral::Compile(compiler::PandaGen *pg) const
{
    pg->LoadAccumulatorString(this, str_);
}

void StringLiteral::Compile(compiler::ETSGen *etsg) const
{
    etsg->LoadAccumulatorString(this, str_);
}

checker::Type *StringLiteral::Check(checker::TSChecker *checker)
{
    auto search = checker->StringLiteralMap().find(str_);
    if (search != checker->StringLiteralMap().end()) {
        return search->second;
    }

    auto *new_str_literal_type = checker->Allocator()->New<checker::StringLiteralType>(str_);
    checker->StringLiteralMap().insert({str_, new_str_literal_type});

    return new_str_literal_type;
}

checker::Type *StringLiteral::Check([[maybe_unused]] checker::ETSChecker *checker)
{
    if (TsType() == nullptr) {
        SetTsType(checker->CreateETSStringLiteralType(str_));
    }
    return TsType();
}

// NOLINTNEXTLINE(google-default-arguments)
Expression *StringLiteral::Clone(ArenaAllocator *const allocator, AstNode *const parent)
{
    if (auto *const clone = allocator->New<StringLiteral>(str_); clone != nullptr) {
        if (parent != nullptr) {
            clone->SetParent(parent);
        }
        return clone;
    }

    throw Error(ErrorType::GENERIC, "", CLONE_ALLOCATION_ERROR);
}
}  // namespace panda::es2panda::ir
