/**
 * Copyright (c) 2023-2024 Huawei Device Co., Ltd.
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

#include "interfacePropertyDeclarations.h"

#include "checker/ETSchecker.h"
#include "checker/types/type.h"
#include "compiler/core/ASTVerifier.h"
#include "compiler/core/compilerContext.h"
#include "compiler/lowering/util.h"
#include "ir/astNode.h"
#include "ir/expression.h"
#include "ir/expressions/identifier.h"
#include "ir/opaqueTypeNode.h"
#include "ir/statements/blockStatement.h"
#include "ir/ts/tsInterfaceBody.h"
#include "ir/base/classProperty.h"

namespace panda::es2panda::compiler {
static ir::MethodDefinition *GenerateGetterOrSetter(checker::ETSChecker *const checker, ir::ClassProperty *const field,
                                                    bool isSetter)
{
    auto classScope = NearestScope(field);
    auto *paramScope = checker->Allocator()->New<varbinder::FunctionParamScope>(checker->Allocator(), classScope);
    auto *functionScope = checker->Allocator()->New<varbinder::FunctionScope>(checker->Allocator(), paramScope);

    functionScope->BindParamScope(paramScope);
    paramScope->BindFunctionScope(functionScope);

    auto flags = ir::ModifierFlags::PUBLIC;
    flags |= ir::ModifierFlags::ABSTRACT;

    ArenaVector<ir::Expression *> params(checker->Allocator()->Adapter());

    if (isSetter) {
        auto paramIdent = field->Key()->AsIdentifier()->Clone(checker->Allocator());
        paramIdent->SetTsTypeAnnotation(field->TypeAnnotation()->Clone(checker->Allocator()));
        paramIdent->TypeAnnotation()->SetParent(paramIdent);

        auto *const paramExpression = checker->AllocNode<ir::ETSParameterExpression>(paramIdent, nullptr);
        paramExpression->SetRange(paramIdent->Range());
        auto *const paramVar = std::get<2>(paramScope->AddParamDecl(checker->Allocator(), paramExpression));

        paramIdent->SetVariable(paramVar);
        paramExpression->SetVariable(paramVar);

        params.push_back(paramExpression);
    }

    auto signature = ir::FunctionSignature(nullptr, std::move(params), isSetter ? nullptr : field->TypeAnnotation());

    auto *func =
        isSetter
            ? checker->AllocNode<ir::ScriptFunction>(std::move(signature), nullptr, ir::ScriptFunctionFlags::SETTER,
                                                     flags, true, Language(Language::Id::ETS))
            : checker->AllocNode<ir::ScriptFunction>(std::move(signature), nullptr, ir::ScriptFunctionFlags::GETTER,
                                                     flags, true, Language(Language::Id::ETS));
    func->SetRange(field->Range());

    func->SetScope(functionScope);

    auto methodIdent = field->Key()->AsIdentifier()->Clone(checker->Allocator());
    auto *decl = checker->Allocator()->New<varbinder::VarDecl>(field->Key()->AsIdentifier()->Name());
    auto var = functionScope->AddDecl(checker->Allocator(), decl, ScriptExtension::ETS);

    methodIdent->SetVariable(var);

    auto *funcExpr = checker->AllocNode<ir::FunctionExpression>(func);
    funcExpr->SetRange(func->Range());
    func->AddFlag(ir::ScriptFunctionFlags::METHOD);

    auto *method = checker->AllocNode<ir::MethodDefinition>(ir::MethodDefinitionKind::METHOD, methodIdent, funcExpr,
                                                            flags, checker->Allocator(), false);

    method->Id()->SetMutator();
    method->SetRange(field->Range());
    method->Function()->SetIdent(method->Id());
    method->Function()->AddModifier(method->Modifiers());
    paramScope->BindNode(func);
    functionScope->BindNode(func);

    return method;
}

static ir::Expression *UpdateInterfacePropertys(checker::ETSChecker *const checker,
                                                ir::TSInterfaceBody *const interface)
{
    if (interface->Body().empty()) {
        return interface;
    }

    auto propertyList = interface->Body();
    ArenaVector<ir::AstNode *> newPropertyList(checker->Allocator()->Adapter());

    auto scope = NearestScope(interface);
    ASSERT(scope->IsClassScope());

    for (const auto &prop : propertyList) {
        if (!prop->IsClassProperty()) {
            newPropertyList.emplace_back(prop);
            continue;
        }
        auto getter = GenerateGetterOrSetter(checker, prop->AsClassProperty(), false);
        newPropertyList.emplace_back(getter);

        auto methodScope = scope->AsClassScope()->InstanceMethodScope();
        auto name = getter->Key()->AsIdentifier()->Name();

        auto *decl = checker->Allocator()->New<varbinder::FunctionDecl>(checker->Allocator(), name, getter);
        auto var = methodScope->AddDecl(checker->Allocator(), decl, ScriptExtension::ETS);
        var->AddFlag(varbinder::VariableFlags::METHOD);

        if (var == nullptr) {
            auto prevDecl = methodScope->FindDecl(name);
            ASSERT(prevDecl->IsFunctionDecl());
            prevDecl->Node()->AsMethodDefinition()->AddOverload(getter);

            if (!prop->AsClassProperty()->IsReadonly()) {
                auto setter = GenerateGetterOrSetter(checker, prop->AsClassProperty(), true);
                newPropertyList.emplace_back(setter);
                prevDecl->Node()->AsMethodDefinition()->AddOverload(setter);
            }

            getter->Function()->Id()->SetVariable(
                methodScope->FindLocal(name, varbinder::ResolveBindingOptions::BINDINGS));
            continue;
        }

        if (!prop->AsClassProperty()->IsReadonly()) {
            auto setter = GenerateGetterOrSetter(checker, prop->AsClassProperty(), true);
            newPropertyList.emplace_back(setter);
            getter->AddOverload(setter);
        }
        getter->Function()->Id()->SetVariable(var);
        scope->AsClassScope()->InstanceFieldScope()->EraseBinding(name);
    }

    auto newInterface = checker->AllocNode<ir::TSInterfaceBody>(std::move(newPropertyList));
    newInterface->SetRange(interface->Range());
    newInterface->SetParent(interface->Parent());

    return newInterface;
}

bool InterfacePropertyDeclarationsPhase::Perform(public_lib::Context *ctx, parser::Program *program)
{
    for (const auto &[_, ext_programs] : program->ExternalSources()) {
        (void)_;
        for (auto *const extProg : ext_programs) {
            Perform(ctx, extProg);
        }
    }

    checker::ETSChecker *const checker = ctx->checker->AsETSChecker();

    program->Ast()->TransformChildrenRecursively([checker](ir::AstNode *const ast) -> ir::AstNode * {
        return ast->IsTSInterfaceBody() ? UpdateInterfacePropertys(checker, ast->AsTSInterfaceBody()) : ast;
    });

    return true;
}

}  // namespace panda::es2panda::compiler
