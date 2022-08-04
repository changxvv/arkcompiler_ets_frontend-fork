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

#include "scriptFunction.h"

#include <binder/scope.h>
#include <ir/astDump.h>
#include <ir/expression.h>
#include <ir/expressions/identifier.h>
#include <ir/statements/blockStatement.h>
#include <ir/ts/tsTypeParameter.h>
#include <ir/ts/tsTypeParameterDeclaration.h>

namespace panda::es2panda::ir {

size_t ScriptFunction::FormalParamsLength() const
{
    size_t length = 0;

    for (const auto *param : params_) {
        if (param->IsRestElement() || param->IsAssignmentPattern()) {
            break;
        }

        length++;
    }

    return length;
}

util::StringView ScriptFunction::GetName() const
{
    if (id_) {
        return id_->Name();
    }

    if (exportDefault_) {
        return parser::SourceTextModuleRecord::DEFAULT_LOCAL_NAME;
    }

    return "";
}

void ScriptFunction::Iterate(const NodeTraverser &cb) const
{
    if (id_) {
        cb(id_);
    }

    if (typeParams_) {
        cb(typeParams_);
    }

    for (auto *it : params_) {
        cb(it);
    }

    if (body_) {
        cb(body_);
    }
}

void ScriptFunction::Dump(ir::AstDumper *dumper) const
{
    dumper->Add({{"type", "ScriptFunction"},
                 {"id", AstDumper::Nullable(id_)},
                 {"generator", IsGenerator()},
                 {"async", IsAsync()},
                 {"expression", ((flags_ & ir::ScriptFunctionFlags::EXPRESSION) != 0)},
                 {"params", params_},
                 {"returnType", AstDumper::Optional(returnTypeAnnotation_)},
                 {"typeParameters", AstDumper::Optional(typeParams_)},
                 {"declare", AstDumper::Optional(declare_)},
                 {"body", AstDumper::Optional(body_)}});
}

void ScriptFunction::Compile([[maybe_unused]] compiler::PandaGen *pg) const {}

checker::Type *ScriptFunction::Check([[maybe_unused]] checker::Checker *checker) const
{
    return nullptr;
}

}  // namespace panda::es2panda::ir
