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

#include "tsQualifiedName.h"

#include <typescript/checker.h>
#include <ir/astDump.h>
#include <ir/expressions/identifier.h>

namespace panda::es2panda::ir {

void TSQualifiedName::Iterate(const NodeTraverser &cb) const
{
    cb(left_);
    cb(right_);
}

void TSQualifiedName::Dump(ir::AstDumper *dumper) const
{
    dumper->Add({{"type", "TSQualifiedName"}, {"left", left_}, {"right", right_}});
}

void TSQualifiedName::Compile([[maybe_unused]] compiler::PandaGen *pg) const {}

checker::Type *TSQualifiedName::Check(checker::Checker *checker) const
{
    checker::Type *baseType = checker->CheckNonNullType(left_->Check(checker), left_->Start());
    binder::Variable *prop = checker->GetPropertyOfType(baseType, right_->Name());

    if (prop) {
        return checker->GetTypeOfVariable(prop);
    }

    if (baseType->IsObjectType()) {
        checker::ObjectType *objType = baseType->AsObjectType();

        if (objType->StringIndexInfo()) {
            return objType->StringIndexInfo()->GetType();
        }
    }

    checker->ThrowTypeError({"Property ", right_->Name(), " does not exist on this type."}, right_->Start());
    return nullptr;
}

void TSQualifiedName::UpdateSelf(const NodeUpdater &cb, [[maybe_unused]] binder::Binder *binder)
{
    left_ = std::get<ir::AstNode *>(cb(left_))->AsExpression();
    right_ = std::get<ir::AstNode *>(cb(right_))->AsIdentifier();
}

}  // namespace panda::es2panda::ir
