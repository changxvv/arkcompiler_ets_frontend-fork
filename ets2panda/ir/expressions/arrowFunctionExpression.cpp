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

#include "arrowFunctionExpression.h"

#include "compiler/core/pandagen.h"
#include "compiler/core/ETSGen.h"
#include "checker/ETSchecker.h"
#include "checker/ets/typeRelationContext.h"
#include "checker/TSchecker.h"
#include "ir/astDump.h"
#include "ir/base/scriptFunction.h"
#include "ir/ets/etsTypeReference.h"
#include "ir/ets/etsTypeReferencePart.h"
#include "ir/expressions/identifier.h"
#include "ir/expressions/thisExpression.h"
#include "ir/statements/variableDeclarator.h"

namespace panda::es2panda::ir {
void ArrowFunctionExpression::TransformChildren(const NodeTransformer &cb)
{
    func_ = cb(func_)->AsScriptFunction();
}

void ArrowFunctionExpression::Iterate(const NodeTraverser &cb) const
{
    cb(func_);
}

void ArrowFunctionExpression::Dump(ir::AstDumper *dumper) const
{
    dumper->Add({{"type", "ArrowFunctionExpression"}, {"function", func_}});
}

void ArrowFunctionExpression::Compile(compiler::PandaGen *pg) const
{
    pg->GetAstCompiler()->Compile(this);
}

void ArrowFunctionExpression::Compile(compiler::ETSGen *etsg) const
{
    etsg->GetAstCompiler()->Compile(this);
}

checker::Type *ArrowFunctionExpression::Check(checker::TSChecker *checker)
{
    return checker->GetAnalyzer()->Check(this);
}

checker::Type *ArrowFunctionExpression::Check(checker::ETSChecker *checker)
{
    return checker->GetAnalyzer()->Check(this);
}

ArrowFunctionExpression::ArrowFunctionExpression(ArrowFunctionExpression const &other, ArenaAllocator *const allocator)
    : Expression(static_cast<Expression const &>(other)),
      captured_vars_(allocator->Adapter()),
      propagate_this_(other.propagate_this_)
{
    func_ = other.func_->Clone(allocator, this)->AsScriptFunction();

    for (auto *const variable : other.captured_vars_) {
        captured_vars_.emplace_back(variable);
    }

    if (other.resolved_lambda_ != nullptr) {
        resolved_lambda_ = other.resolved_lambda_->Clone(allocator, this)->AsClassDefinition();
    }
}

// NOLINTNEXTLINE(google-default-arguments)
ArrowFunctionExpression *ArrowFunctionExpression::Clone(ArenaAllocator *const allocator, AstNode *const parent)
{
    if (auto *const clone = allocator->New<ArrowFunctionExpression>(*this, allocator); clone != nullptr) {
        if (parent != nullptr) {
            clone->SetParent(parent);
        }
        return clone;
    }

    throw Error(ErrorType::GENERIC, "", CLONE_ALLOCATION_ERROR);
}

ir::TypeNode *ArrowFunctionExpression::CreateReturnNodeFromType(checker::ETSChecker *checker,
                                                                checker::Type *return_type)
{
    /*
    Construct a synthetic Node with the correct ts_type_.
    */
    ASSERT(return_type != nullptr);
    ir::TypeNode *return_node = nullptr;
    auto *ident = checker->Allocator()->New<ir::Identifier>(util::StringView(""), checker->Allocator());
    ir::ETSTypeReferencePart *part = checker->Allocator()->New<ir::ETSTypeReferencePart>(ident);
    return_node = checker->Allocator()->New<ir::ETSTypeReference>(part);
    part->SetParent(return_node);
    return_node->SetTsType(return_type);
    return return_node;
}

ir::TypeNode *ArrowFunctionExpression::CreateTypeAnnotation(checker::ETSChecker *checker)
{
    ir::TypeNode *return_node = nullptr;
    /*
    There are two scenarios for lambda type inference: defined or undefined return type.
    example code:
    ```
    enum Color { Red, Blue}
    // has Return Type Color
    let x  = () : Color => {return Color.Red}
    // No Return Type Color
    let y  = () => {return Color.Red}
    ```
    */
    if (Function()->ReturnTypeAnnotation() == nullptr) {
        /*
        When lambda expression does not declare a return type, we need to construct the
        declaration node of lambda according to the Function()->Signature()->ReturnType().
        */
        return_node = CreateReturnNodeFromType(checker, Function()->Signature()->ReturnType());
    } else {
        return_node = Function()->ReturnTypeAnnotation();
    }

    auto orig_params = Function()->Params();
    auto signature = ir::FunctionSignature(nullptr, std::move(orig_params), return_node);
    auto *func_type =
        checker->Allocator()->New<ir::ETSFunctionType>(std::move(signature), ir::ScriptFunctionFlags::NONE);
    return_node->SetParent(func_type);
    return func_type;
}
}  // namespace panda::es2panda::ir
