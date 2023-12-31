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

#include "tsEnumDeclaration.h"

#include "checker/TSchecker.h"
#include "compiler/core/ETSGen.h"
#include "compiler/core/pandagen.h"
#include "varbinder/scope.h"
#include "util/helpers.h"

namespace panda::es2panda::ir {
void TSEnumDeclaration::TransformChildren(const NodeTransformer &cb)
{
    for (auto *&it : decorators_) {
        it = cb(it)->AsDecorator();
    }

    key_ = cb(key_)->AsIdentifier();

    for (auto *&it : members_) {
        it = cb(it);
    }
}

void TSEnumDeclaration::Iterate(const NodeTraverser &cb) const
{
    for (auto *it : decorators_) {
        cb(it);
    }

    cb(key_);

    for (auto *it : members_) {
        cb(it);
    }
}

void TSEnumDeclaration::Dump(ir::AstDumper *dumper) const
{
    dumper->Add({{"type", "TSEnumDeclaration"},
                 {"decorators", AstDumper::Optional(decorators_)},
                 {"id", key_},
                 {"members", members_},
                 {"const", is_const_}});
}

// NOTE (csabahurton): this method has not been moved to TSAnalyizer.cpp, because it is not used.
varbinder::EnumMemberResult EvaluateMemberExpression(checker::TSChecker *checker,
                                                     [[maybe_unused]] varbinder::EnumVariable *enum_var,
                                                     ir::MemberExpression *expr)
{
    if (checker::TSChecker::IsConstantMemberAccess(expr->AsExpression())) {
        if (expr->Check(checker)->TypeFlags() == checker::TypeFlag::ENUM) {
            util::StringView name;
            if (!expr->IsComputed()) {
                name = expr->Property()->AsIdentifier()->Name();
            } else {
                ASSERT(checker::TSChecker::IsStringLike(expr->Property()));
                name = reinterpret_cast<const ir::StringLiteral *>(expr->Property())->Str();
            }

            // NOTE: aszilagyi.
        }
    }

    return false;
}

void TSEnumDeclaration::Compile(compiler::PandaGen *pg) const
{
    pg->GetAstCompiler()->Compile(this);
}

void TSEnumDeclaration::Compile(compiler::ETSGen *etsg) const
{
    etsg->GetAstCompiler()->Compile(this);
}

checker::Type *TSEnumDeclaration::Check(checker::TSChecker *checker)
{
    return checker->GetAnalyzer()->Check(this);
}

checker::Type *TSEnumDeclaration::Check(checker::ETSChecker *const checker)
{
    return checker->GetAnalyzer()->Check(this);
}
}  // namespace panda::es2panda::ir
