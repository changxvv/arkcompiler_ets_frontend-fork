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

#include "charLiteral.h"

#include "compiler/core/pandagen.h"
#include "compiler/core/ETSGen.h"
#include "checker/ETSchecker.h"
#include "ir/astDump.h"

#include <utility>

namespace panda::es2panda::ir {
void CharLiteral::TransformChildren([[maybe_unused]] const NodeTransformer &cb) {}
void CharLiteral::Iterate([[maybe_unused]] const NodeTraverser &cb) const {}

void CharLiteral::Dump(ir::AstDumper *dumper) const
{
    dumper->Add({{"type", "CharLiteral"}, {"value", char_}});
}

void CharLiteral::Compile([[maybe_unused]] compiler::PandaGen *pg) const {}

void CharLiteral::Compile([[maybe_unused]] compiler::ETSGen *etsg) const
{
    etsg->LoadAccumulatorChar(this, char_);
}

checker::Type *CharLiteral::Check([[maybe_unused]] checker::TSChecker *checker)
{
    return nullptr;
}

checker::Type *CharLiteral::Check([[maybe_unused]] checker::ETSChecker *checker)
{
    if (TsType() == nullptr) {
        SetTsType(checker->Allocator()->New<checker::CharType>(char_));
    }
    return TsType();
}

// NOLINTNEXTLINE(google-default-arguments)
CharLiteral *CharLiteral::Clone(ArenaAllocator *const allocator, AstNode *const parent)
{
    if (auto *const clone = allocator->New<CharLiteral>(char_); clone != nullptr) {
        if (parent != nullptr) {
            clone->SetParent(parent);
        }
        return clone;
    }

    throw Error(ErrorType::GENERIC, "", CLONE_ALLOCATION_ERROR);
}
}  // namespace panda::es2panda::ir
