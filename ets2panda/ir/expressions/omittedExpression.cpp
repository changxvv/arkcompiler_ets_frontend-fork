/**
 * Copyright (c) 2021-2023 Huawei Device Co., Ltd.
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

#include "omittedExpression.h"

#include "ir/astDump.h"
#include "checker/TSchecker.h"

namespace panda::es2panda::ir {
void OmittedExpression::TransformChildren([[maybe_unused]] const NodeTransformer &cb) {}
void OmittedExpression::Iterate([[maybe_unused]] const NodeTraverser &cb) const {}

void OmittedExpression::Dump(ir::AstDumper *dumper) const
{
    dumper->Add({{"type", "OmittedExpression"}});
}

void OmittedExpression::Compile([[maybe_unused]] compiler::PandaGen *pg) const {}

checker::Type *OmittedExpression::Check([[maybe_unused]] checker::TSChecker *checker)
{
    return checker->GlobalUndefinedType();
}

checker::Type *OmittedExpression::Check([[maybe_unused]] checker::ETSChecker *checker)
{
    return nullptr;
}

// NOLINTNEXTLINE(google-default-arguments)
OmittedExpression *OmittedExpression::Clone(ArenaAllocator *const allocator, AstNode *const parent)
{
    if (auto *const clone = allocator->New<OmittedExpression>(); clone != nullptr) {
        if (parent != nullptr) {
            clone->SetParent(parent);
        }
        return clone;
    }
    throw Error(ErrorType::GENERIC, "", CLONE_ALLOCATION_ERROR);
}
}  // namespace panda::es2panda::ir
