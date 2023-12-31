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

#include "tsParameterProperty.h"

#include "ir/astDump.h"
#include "ir/expression.h"

namespace panda::es2panda::ir {
void TSParameterProperty::TransformChildren(const NodeTransformer &cb)
{
    parameter_ = cb(parameter_)->AsExpression();
}

void TSParameterProperty::Iterate(const NodeTraverser &cb) const
{
    cb(parameter_);
}

void TSParameterProperty::Dump(ir::AstDumper *dumper) const
{
    dumper->Add({{"type", "TSParameterProperty"},
                 {"accessibility", accessibility_ == AccessibilityOption::PUBLIC      ? "public"
                                   : accessibility_ == AccessibilityOption::PRIVATE   ? "private"
                                   : accessibility_ == AccessibilityOption::PROTECTED ? "protected"
                                                                                      : "undefined"},
                 {"readonly", readonly_},
                 {"static", static_},
                 {"export", export_},
                 {"parameter", parameter_}});
}

void TSParameterProperty::Compile([[maybe_unused]] compiler::PandaGen *pg) const {}

checker::Type *TSParameterProperty::Check([[maybe_unused]] checker::TSChecker *checker)
{
    return nullptr;
}

checker::Type *TSParameterProperty::Check([[maybe_unused]] checker::ETSChecker *checker)
{
    return nullptr;
}
}  // namespace panda::es2panda::ir
