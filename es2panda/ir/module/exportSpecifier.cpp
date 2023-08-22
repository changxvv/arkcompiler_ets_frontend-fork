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

#include "exportSpecifier.h"

#include <ir/astDump.h>
#include <ir/expressions/identifier.h>

namespace panda::es2panda::ir {

void ExportSpecifier::Iterate(const NodeTraverser &cb) const
{
    cb(local_);
    cb(exported_);
}

void ExportSpecifier::Dump(ir::AstDumper *dumper) const
{
    dumper->Add({{"type", "ExportSpecifier"}, {"local", local_}, {"exported", exported_}, {"isType", isType_}});
}

void ExportSpecifier::Compile([[maybe_unused]] compiler::PandaGen *pg) const {}

checker::Type *ExportSpecifier::Check([[maybe_unused]] checker::Checker *checker) const
{
    return nullptr;
}

void ExportSpecifier::UpdateSelf(const NodeUpdater &cb, [[maybe_unused]] binder::Binder *binder)
{
    local_ = std::get<ir::AstNode *>(cb(local_))->AsIdentifier();
    exported_ = std::get<ir::AstNode *>(cb(exported_))->AsIdentifier();
}

}  // namespace panda::es2panda::ir
