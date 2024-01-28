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

#include "methodDefinition.h"

#include "varbinder/scope.h"
#include "checker/TSchecker.h"
#include "compiler/core/ETSGen.h"
#include "compiler/core/pandagen.h"
#include "ir/astDump.h"
#include "ir/srcDump.h"

namespace ark::es2panda::ir {

ScriptFunction *MethodDefinition::Function()
{
    return value_->IsFunctionExpression() ? value_->AsFunctionExpression()->Function() : nullptr;
}

const ScriptFunction *MethodDefinition::Function() const
{
    return value_->IsFunctionExpression() ? value_->AsFunctionExpression()->Function() : nullptr;
}

PrivateFieldKind MethodDefinition::ToPrivateFieldKind(bool const isStatic) const
{
    switch (kind_) {
        case MethodDefinitionKind::METHOD: {
            return isStatic ? PrivateFieldKind::STATIC_METHOD : PrivateFieldKind::METHOD;
        }
        case MethodDefinitionKind::GET: {
            return isStatic ? PrivateFieldKind::STATIC_GET : PrivateFieldKind::GET;
        }
        case MethodDefinitionKind::SET: {
            return isStatic ? PrivateFieldKind::STATIC_SET : PrivateFieldKind::SET;
        }
        default: {
            UNREACHABLE();
        }
    }
}

void MethodDefinition::Iterate(const NodeTraverser &cb) const
{
    cb(key_);
    cb(value_);

    for (auto *it : overloads_) {
        cb(it);
    }

    for (auto *it : decorators_) {
        cb(it);
    }
}

void MethodDefinition::TransformChildren(const NodeTransformer &cb)
{
    key_ = cb(key_)->AsExpression();
    value_ = cb(value_)->AsExpression();

    for (auto *&it : overloads_) {
        it = cb(it)->AsMethodDefinition();
    }

    for (auto *&it : decorators_) {
        it = cb(it)->AsDecorator();
    }
}

void MethodDefinition::Dump(ir::AstDumper *dumper) const
{
    const char *kind = nullptr;

    switch (kind_) {
        case MethodDefinitionKind::CONSTRUCTOR: {
            kind = "constructor";
            break;
        }
        case MethodDefinitionKind::METHOD: {
            kind = "method";
            break;
        }
        case MethodDefinitionKind::EXTENSION_METHOD: {
            kind = "extensionmethod";
            break;
        }
        case MethodDefinitionKind::GET: {
            kind = "get";
            break;
        }
        case MethodDefinitionKind::SET: {
            kind = "set";
            break;
        }
        default: {
            UNREACHABLE();
        }
    }

    dumper->Add({{"type", "MethodDefinition"},
                 {"key", key_},
                 {"kind", kind},
                 {"accessibility", AstDumper::Optional(AstDumper::ModifierToString(flags_))},
                 {"static", IsStatic()},
                 {"optional", IsOptionalDeclaration()},
                 {"computed", isComputed_},
                 {"value", value_},
                 {"overloads", overloads_},
                 {"decorators", decorators_}});
}

void MethodDefinition::Dump(ir::SrcDumper *dumper) const
{
    for (auto method : overloads_) {
        method->Dump(dumper);
        dumper->Endl();
    }

    if (IsPrivate()) {
        dumper->Add("private ");
    } else if (IsProtected()) {
        dumper->Add("protected ");
    } else if (IsInternal()) {
        dumper->Add("internal ");
    } else {
        dumper->Add("public ");
    }

    if (IsStatic()) {
        dumper->Add("static ");
    }

    if (IsAbstract()) {
        dumper->Add("abstract ");
    }

    if (IsFinal()) {
        dumper->Add("final ");
    }

    if (IsNative()) {
        dumper->Add("native ");
    }

    if (IsAsync()) {
        dumper->Add("async ");
    }

    if (IsOverride()) {
        dumper->Add("override ");
    }

    if (key_ != nullptr) {
        key_->Dump(dumper);
    }

    if (value_ != nullptr) {
        value_->Dump(dumper);
    }
}

void MethodDefinition::Compile(compiler::PandaGen *pg) const
{
    pg->GetAstCompiler()->Compile(this);
}

void MethodDefinition::Compile(compiler::ETSGen *etsg) const
{
    etsg->GetAstCompiler()->Compile(this);
}

checker::Type *MethodDefinition::Check(checker::TSChecker *checker)
{
    return checker->GetAnalyzer()->Check(this);
}

checker::Type *MethodDefinition::Check(checker::ETSChecker *checker)
{
    return checker->GetAnalyzer()->Check(this);
}

// NOLINTNEXTLINE(google-default-arguments)
MethodDefinition *MethodDefinition::Clone(ArenaAllocator *const allocator, AstNode *const parent)
{
    auto *const key = key_ != nullptr ? key_->Clone(allocator)->AsExpression() : nullptr;
    auto *const value = value_ != nullptr ? value_->Clone(allocator)->AsExpression() : nullptr;

    if (auto *const clone = allocator->New<MethodDefinition>(kind_, key, value, flags_, allocator, isComputed_);
        clone != nullptr) {
        if (parent != nullptr) {
            clone->SetParent(parent);
        }

        if (key != nullptr) {
            key->SetParent(clone);
        }

        if (value != nullptr) {
            value->SetParent(clone);
        }

        for (auto *const decorator : decorators_) {
            clone->AddDecorator(decorator->Clone(allocator, clone));
        }

        for (auto *const overloads : overloads_) {
            clone->AddOverload(overloads->Clone(allocator, clone));
        }

        return clone;
    }

    throw Error(ErrorType::GENERIC, "", CLONE_ALLOCATION_ERROR);
}
}  // namespace ark::es2panda::ir
