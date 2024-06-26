/*
 * Copyright (c) 2021-2024 Huawei Device Co., Ltd.
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

#include "ETSBinder.h"

#include "ir/expressions/blockExpression.h"
#include "ir/expressions/identifier.h"
#include "ir/expressions/thisExpression.h"
#include "ir/expressions/typeofExpression.h"
#include "ir/expressions/memberExpression.h"
#include "ir/expressions/callExpression.h"
#include "ir/expressions/functionExpression.h"
#include "ir/base/methodDefinition.h"
#include "ir/base/scriptFunction.h"
#include "ir/base/classElement.h"
#include "ir/base/classDefinition.h"
#include "ir/base/classProperty.h"
#include "ir/base/classStaticBlock.h"
#include "ir/statements/blockStatement.h"
#include "ir/statements/classDeclaration.h"
#include "ir/statements/variableDeclarator.h"
#include "ir/statements/functionDeclaration.h"
#include "ir/statements/returnStatement.h"
#include "ir/ets/etsPrimitiveType.h"
#include "ir/ets/etsTypeReferencePart.h"
#include "ir/ets/etsNewClassInstanceExpression.h"
#include "ir/ets/etsTypeReference.h"
#include "ir/ets/etsFunctionType.h"
#include "ir/ets/etsScript.h"
#include "ir/ets/etsImportDeclaration.h"
#include "ir/ts/tsInterfaceDeclaration.h"
#include "ir/ts/tsTypeParameterDeclaration.h"
#include "ir/ts/tsTypeParameterInstantiation.h"
#include "ir/ts/tsClassImplements.h"
#include "ir/ts/tsEnumDeclaration.h"
#include "ir/ts/tsEnumMember.h"
#include "ir/ts/tsInterfaceHeritage.h"
#include "ir/ts/tsInterfaceBody.h"
#include "ir/ts/tsFunctionType.h"
#include "ir/ts/tsQualifiedName.h"
#include "ir/module/importDefaultSpecifier.h"
#include "ir/module/importNamespaceSpecifier.h"
#include "ir/module/importDeclaration.h"
#include "ir/module/importSpecifier.h"
#include "ir/expressions/literals/stringLiteral.h"
#include "parser/program/program.h"
#include "util/helpers.h"
#include "util/ustring.h"
#include "checker/types/type.h"
#include "checker/types/ets/types.h"

namespace ark::es2panda::varbinder {

void ETSBinder::IdentifierAnalysis()
{
    ASSERT(Program()->Ast());
    ASSERT(GetScope() == TopScope());
    ASSERT(VarScope() == TopScope());

    recordTable_->SetProgram(Program());
    globalRecordTable_.SetClassDefinition(Program()->GlobalClass());
    externalRecordTable_.insert({Program(), &globalRecordTable_});

    BuildProgram();

    ASSERT(globalRecordTable_.ClassDefinition() == Program()->GlobalClass());
}

void ETSBinder::LookupTypeArgumentReferences(ir::ETSTypeReference *typeRef)
{
    auto *iter = typeRef->Part();

    while (iter != nullptr) {
        if (iter->TypeParams() == nullptr) {
            iter = iter->Previous();
            continue;
        }

        ResolveReferences(iter->TypeParams());
        iter = iter->Previous();
    }
}

void ETSBinder::LookupTypeReference(ir::Identifier *ident, bool allowDynamicNamespaces)
{
    const auto &name = ident->Name();
    if (name == compiler::Signatures::UNDEFINED || name == compiler::Signatures::NULL_LITERAL) {
        return;
    }
    auto *iter = GetScope();

    while (iter != nullptr) {
        auto res = iter->Find(name, ResolveBindingOptions::DECLARATION | ResolveBindingOptions::TYPE_ALIASES);
        if (res.variable == nullptr) {
            break;
        }

        if (IsDynamicModuleVariable(res.variable)) {
            ident->SetVariable(res.variable);
            return;
        }

        if (allowDynamicNamespaces && IsDynamicNamespaceVariable(res.variable)) {
            ident->SetVariable(res.variable);
            return;
        }

        switch (res.variable->Declaration()->Node()->Type()) {
            case ir::AstNodeType::CLASS_DECLARATION:
            case ir::AstNodeType::CLASS_DEFINITION:
            case ir::AstNodeType::STRUCT_DECLARATION:
            case ir::AstNodeType::TS_ENUM_DECLARATION:
            case ir::AstNodeType::TS_INTERFACE_DECLARATION:
            case ir::AstNodeType::TS_TYPE_PARAMETER:
            case ir::AstNodeType::TS_TYPE_ALIAS_DECLARATION:
            case ir::AstNodeType::IMPORT_NAMESPACE_SPECIFIER: {
                ident->SetVariable(res.variable);
                return;
            }
            default: {
                iter = iter->Parent();
            }
        }
    }

    ThrowUnresolvableType(ident->Start(), name);
}

void ETSBinder::ResolveReferencesForScope(ir::AstNode const *const parent, Scope *const scope)
{
    parent->Iterate([this, scope](auto *node) { ResolveReferenceForScope(node, scope); });
}

void ETSBinder::ResolveReferenceForScope(ir::AstNode *const node, Scope *const scope)
{
    switch (node->Type()) {
        case ir::AstNodeType::IDENTIFIER: {
            auto *ident = node->AsIdentifier();
            if (ident->Variable() != nullptr) {
                break;
            }
            if (auto const res = scope->Find(ident->Name(), ResolveBindingOptions::ALL); res.variable != nullptr) {
                ident->SetVariable(res.variable);
            }
            break;
        }
        case ir::AstNodeType::VARIABLE_DECLARATOR: {
            auto scopeCtx = LexicalScope<Scope>::Enter(this, scope);
            BuildVarDeclarator(node->AsVariableDeclarator());
            break;
        }
        /* Maybe will be used
        case ir::AstNodeType::BLOCK_STATEMENT: {
            auto scope_ctx = LexicalScope<Scope>::Enter(this, node->AsBlockStatement()->Scope());
            ResolveReferences(node);
            break;
        }
        */
        case ir::AstNodeType::BLOCK_EXPRESSION: {
            auto scopeCtx = LexicalScope<Scope>::Enter(this, node->AsBlockExpression()->Scope());
            ResolveReferences(node);
            break;
        }
        default: {
            ResolveReferencesForScope(node, scope);
            break;
        }
    }
}

void ETSBinder::LookupIdentReference(ir::Identifier *ident)
{
    const auto &name = ident->Name();
    auto res = GetScope()->Find(name, ResolveBindingOptions::ALL);
    if (res.level != 0) {
        ASSERT(res.variable != nullptr);

        auto *outerFunction = GetScope()->EnclosingVariableScope()->Node();

        if ((!outerFunction->IsScriptFunction() || !outerFunction->AsScriptFunction()->IsArrow()) &&
            !res.variable->IsGlobalVariable() && res.variable->HasFlag(VariableFlags::LOCAL) && res.level > 1) {
            ThrowInvalidCapture(ident->Start(), name);
        }
    }

    if (res.variable == nullptr) {
        return;
    }

    if (ident->IsReference() && res.variable->Declaration()->IsLetOrConstDecl() &&
        !res.variable->HasFlag(VariableFlags::INITIALIZED)) {
        ThrowTDZ(ident->Start(), name);
    }
}

void ETSBinder::BuildClassProperty(const ir::ClassProperty *prop)
{
    ResolveReferences(prop);
}

void ETSBinder::InitializeInterfaceIdent(ir::TSInterfaceDeclaration *decl)
{
    auto res = GetScope()->Find(decl->Id()->Name());

    ASSERT(res.variable && res.variable->Declaration()->IsInterfaceDecl());
    res.variable->AddFlag(VariableFlags::INITIALIZED);
    decl->Id()->SetVariable(res.variable);
}

void ETSBinder::ResolveEnumDeclaration(ir::TSEnumDeclaration *enumDecl)
{
    auto enumScopeCtx = LexicalScope<LocalScope>::Enter(this, enumDecl->Scope());

    for (auto *member : enumDecl->Members()) {
        ResolveReference(member);
    }
}

void ETSBinder::ResolveInterfaceDeclaration(ir::TSInterfaceDeclaration *decl)
{
    auto boundCtx = BoundContext(recordTable_, decl);

    for (auto *extend : decl->Extends()) {
        ResolveReference(extend);
    }

    auto scopeCtx = LexicalScope<ClassScope>::Enter(this, decl->Scope()->AsClassScope());

    for (auto *stmt : decl->Body()->Body()) {
        if (!stmt->IsClassProperty()) {
            continue;
        }

        ResolveReference(stmt);

        auto fieldVar =
            ResolvePropertyReference(stmt->AsClassProperty(), decl->Scope()->AsClassScope())
                ->FindLocal(stmt->AsClassProperty()->Id()->Name(), varbinder::ResolveBindingOptions::BINDINGS);
        fieldVar->AddFlag(VariableFlags::INITIALIZED);
    }

    for (auto *stmt : decl->Body()->Body()) {
        if (stmt->IsClassProperty()) {
            continue;
        }
        ResolveReference(stmt);
    }
}

void ETSBinder::BuildInterfaceDeclaration(ir::TSInterfaceDeclaration *decl)
{
    if (decl->TypeParams() != nullptr) {
        auto typeParamScopeCtx = LexicalScope<LocalScope>::Enter(this, decl->TypeParams()->Scope());
        ResolveReferences(decl->TypeParams());
        ResolveInterfaceDeclaration(decl);
        return;
    }

    ResolveInterfaceDeclaration(decl);
}

void ETSBinder::BuildMethodDefinition(ir::MethodDefinition *methodDef)
{
    if (methodDef->Function()->TypeParams() != nullptr) {
        auto scopeCtx = LexicalScope<LocalScope>::Enter(this, methodDef->Function()->TypeParams()->Scope());
        ResolveReferences(methodDef->Function()->TypeParams());
        ResolveMethodDefinition(methodDef);
        return;
    }

    ResolveMethodDefinition(methodDef);
}

void ETSBinder::ResolveMethodDefinition(ir::MethodDefinition *methodDef)
{
    methodDef->ResolveReferences([this](auto *childNode) { ResolveReference(childNode); });

    auto *func = methodDef->Function();
    if (methodDef->IsStatic() || func->IsStaticBlock()) {
        return;
    }

    auto paramScopeCtx = LexicalScope<FunctionParamScope>::Enter(this, func->Scope()->ParamScope());

    auto params = func->Scope()->ParamScope()->Params();
    if (!params.empty() && params.front()->Name() == MANDATORY_PARAM_THIS) {
        return;  // Implicit this parameter is already inserted by ResolveReferences(), don't insert it twice.
    }

    auto *thisParam = AddMandatoryParam(MANDATORY_PARAM_THIS);
    thisParam->Declaration()->BindNode(thisParam_);
}

void ETSBinder::BuildMemberExpression(ir::MemberExpression *memberExpr)
{
    ResolveReference(memberExpr->Object());

    if (memberExpr->Kind() == ir::MemberExpressionKind::ELEMENT_ACCESS) {
        ResolveReference(memberExpr->Property());
    }
}

void ETSBinder::BuildClassDefinition(ir::ClassDefinition *classDef)
{
    auto boundCtx = BoundContext(recordTable_, classDef);

    if (classDef->TypeParams() != nullptr) {
        auto scopeCtx = LexicalScope<LocalScope>::Enter(this, classDef->TypeParams()->Scope());
        ResolveReferences(classDef->TypeParams());
        BuildClassDefinitionImpl(classDef);
        return;
    }

    BuildClassDefinitionImpl(classDef);
}

LocalScope *ETSBinder::ResolvePropertyReference(ir::ClassProperty *prop, ClassScope *scope)
{
    ResolveReferences(prop);

    if (prop->IsStatic()) {
        return scope->StaticFieldScope();
    }

    return scope->InstanceFieldScope();
}

void ETSBinder::BuildClassDefinitionImpl(ir::ClassDefinition *classDef)
{
    auto classCtx = LexicalScope<ClassScope>::Enter(this, classDef->Scope()->AsClassScope());

    if (classDef->Super() != nullptr) {
        ResolveReference(classDef->Super());
    }

    for (auto *impl : classDef->Implements()) {
        ResolveReference(impl);
    }

    for (auto *stmt : classDef->Body()) {
        if (!stmt->IsClassProperty()) {
            continue;
        }

        auto fieldScope = ResolvePropertyReference(stmt->AsClassProperty(), classDef->Scope()->AsClassScope());
        auto fieldName = stmt->AsClassProperty()->Id()->Name();
        auto fieldVar = fieldScope->FindLocal(fieldName, varbinder::ResolveBindingOptions::BINDINGS);
        fieldVar->AddFlag(VariableFlags::INITIALIZED);
        if (fieldVar->Declaration()->IsConstDecl() && stmt->AsClassProperty()->Value() == nullptr) {
            fieldVar->AddFlag(VariableFlags::EXPLICIT_INIT_REQUIRED);
        }
    }

    for (auto *stmt : classDef->Body()) {
        if (stmt->IsClassProperty()) {
            continue;
        }
        ResolveReference(stmt);
    }
}

void ETSBinder::AddLambdaFunctionThisParam(const ir::ScriptFunction *const func, bool isExternal)
{
    auto paramScopeCtx = LexicalScope<FunctionParamScope>::Enter(this, func->Scope()->ParamScope());
    auto *const thisParam = AddMandatoryParam(MANDATORY_PARAM_THIS);
    thisParam->Declaration()->BindNode(thisParam_);
    if (!func->IsAsyncFunc() && !isExternal) {
        Functions().push_back(func->Scope());
    }
}

void ETSBinder::AddInvokeFunctionThisParam(ir::ScriptFunction *func)
{
    auto paramScopeCtx = LexicalScope<FunctionParamScope>::Enter(this, func->Scope()->ParamScope());
    auto *thisParam = AddMandatoryParam(MANDATORY_PARAM_THIS);
    thisParam->Declaration()->BindNode(thisParam_);
}

void ETSBinder::BuildProxyMethod(ir::ScriptFunction *func, const util::StringView &containingClassName, bool isStatic,
                                 bool isExternal)
{
    ASSERT(!containingClassName.Empty());
    func->Scope()->BindName(containingClassName);

    if (!isStatic) {
        auto paramScopeCtx = LexicalScope<FunctionParamScope>::Enter(this, func->Scope()->ParamScope());
        auto *thisParam = AddMandatoryParam(MANDATORY_PARAM_THIS);
        thisParam->Declaration()->BindNode(thisParam_);
    }

    if (!func->IsAsyncFunc() && !isExternal) {
        Functions().push_back(func->Scope());
    }
}

void ETSBinder::BuildLambdaObject(ir::AstNode *refNode, ir::ClassDefinition *lambdaObject,
                                  checker::Signature *signature, bool isExternal)
{
    auto boundCtx = BoundContext(GetGlobalRecordTable(), lambdaObject);
    const auto &lambdaBody = lambdaObject->Body();

    AddLambdaFunctionThisParam(lambdaBody[lambdaBody.size() - 3U]->AsMethodDefinition()->Function(), isExternal);
    AddLambdaFunctionThisParam(lambdaBody[lambdaBody.size() - 2U]->AsMethodDefinition()->Function(), isExternal);
    AddLambdaFunctionThisParam(lambdaBody[lambdaBody.size() - 1U]->AsMethodDefinition()->Function(), isExternal);

    LambdaObjects().insert({refNode, {lambdaObject, signature}});
}

void ETSBinder::BuildFunctionType(ir::ETSFunctionType *funcType)
{
    auto boundCtx = BoundContext(GetGlobalRecordTable(), funcType->FunctionalInterface());

    auto *invokeFunc = funcType->FunctionalInterface()->Body()->Body()[0]->AsMethodDefinition()->Function();
    auto *funcScope = invokeFunc->Scope();
    funcScope->BindName(recordTable_->RecordName());
    AddInvokeFunctionThisParam(invokeFunc);

    GetGlobalRecordTable()->Signatures().push_back(funcScope);
}

void ETSBinder::AddDynamicSpecifiersToTopBindings(ir::AstNode *const specifier,
                                                  const ir::ETSImportDeclaration *const import)
{
    const auto name = [specifier]() {
        if (specifier->IsImportNamespaceSpecifier()) {
            return specifier->AsImportNamespaceSpecifier()->Local()->Name();
        }

        return specifier->AsImportSpecifier()->Local()->Name();
    }();

    auto *const decl = Allocator()->New<varbinder::LetDecl>(name, specifier);
    auto *const var = Allocator()->New<varbinder::LocalVariable>(decl, varbinder::VariableFlags::STATIC);
    var->AddFlag(VariableFlags::INITIALIZED);

    dynamicImportVars_.emplace(var, DynamicImportData {import, specifier, var});

    TopScope()->InsertDynamicBinding(name, var);
}

void ETSBinder::InsertForeignBinding(ir::AstNode *const specifier, const ir::ETSImportDeclaration *const import,
                                     const util::StringView &name, Variable *var)
{
    if (import->Language().IsDynamic()) {
        dynamicImportVars_.emplace(var, DynamicImportData {import, specifier, var});
    }

    TopScope()->InsertForeignBinding(name, var);
}

void ETSBinder::ImportAllForeignBindings(ir::AstNode *const specifier,
                                         const varbinder::Scope::VariableMap &globalBindings,
                                         const parser::Program *const importProgram,
                                         const varbinder::GlobalScope *const importGlobalScope,
                                         const ir::ETSImportDeclaration *const import)
{
    for (const auto [bindingName, var] : globalBindings) {
        if (bindingName.Is(compiler::Signatures::ETS_GLOBAL)) {
            const auto *const classDef = var->Declaration()->Node()->AsClassDeclaration()->Definition();
            ImportGlobalProperties(classDef);
            continue;
        }

        if (!importGlobalScope->IsForeignBinding(bindingName) && !var->Declaration()->Node()->IsDefaultExported()) {
            InsertForeignBinding(specifier, import, bindingName, var);
        }
    }

    for (const auto [bindingName, var] : importProgram->GlobalClassScope()->StaticMethodScope()->Bindings()) {
        if (!var->Declaration()->Node()->IsDefaultExported()) {
            InsertForeignBinding(specifier, import, bindingName, var);
        }
    }

    for (const auto [bindingName, var] : importProgram->GlobalClassScope()->StaticFieldScope()->Bindings()) {
        if (!var->Declaration()->Node()->IsDefaultExported()) {
            InsertForeignBinding(specifier, import, bindingName, var);
        }
    }
}

bool ETSBinder::AddImportNamespaceSpecifiersToTopBindings(ir::AstNode *const specifier,
                                                          const varbinder::Scope::VariableMap &globalBindings,
                                                          const parser::Program *const importProgram,
                                                          const varbinder::GlobalScope *const importGlobalScope,
                                                          const ir::ETSImportDeclaration *const import)
{
    if (!specifier->IsImportNamespaceSpecifier()) {
        return false;
    }
    const auto *const namespaceSpecifier = specifier->AsImportNamespaceSpecifier();

    if (namespaceSpecifier->Local()->Name().Empty()) {
        ImportAllForeignBindings(specifier, globalBindings, importProgram, importGlobalScope, import);
    }

    std::unordered_set<std::string> exportedNames;
    for (auto item : ReExportImports()) {
        // NOTE(rsipka): this should be refactored or eliminated
        if (auto source = import->ResolvedSource()->Str(), program = item->GetProgramPath();
            !source.Is(program.Mutf8())) {
            continue;
        }

        for (auto it : item->GetETSImportDeclarations()->Specifiers()) {
            if (it->IsImportNamespaceSpecifier() && !specifier->AsImportNamespaceSpecifier()->Local()->Name().Empty()) {
                std::cerr << "Warning: import with alias cannot be used with re-export\n";
                continue;
            }

            AddSpecifiersToTopBindings(it, item->GetETSImportDeclarations(),
                                       item->GetETSImportDeclarations()->Source());

            if (it->IsImportSpecifier() &&
                !exportedNames.insert(it->AsImportSpecifier()->Local()->Name().Mutf8()).second) {
                ThrowError(import->Start(), "Ambiguous import \"" + it->AsImportSpecifier()->Local()->Name().Mutf8() +
                                                "\" has multiple matching exports");
            }
        }
    }

    return true;
}

Variable *ETSBinder::FindImportSpecifiersVariable(const util::StringView &imported,
                                                  const varbinder::Scope::VariableMap &globalBindings,
                                                  const ArenaVector<parser::Program *> &recordRes)
{
    auto foundVar = globalBindings.find(imported);
    if (foundVar == globalBindings.end()) {
        const auto &staticMethodBindings = recordRes.front()->GlobalClassScope()->StaticMethodScope()->Bindings();
        foundVar = staticMethodBindings.find(imported);
        if (foundVar != staticMethodBindings.end()) {
            return foundVar->second;
        }
        bool found = false;
        for (auto res : recordRes) {
            const auto &staticFieldBindings = res->GlobalClassScope()->StaticFieldScope()->Bindings();
            foundVar = staticFieldBindings.find(imported);
            if (foundVar != staticFieldBindings.end()) {
                found = true;
                foundVar->second->AsLocalVariable()->AddFlag(VariableFlags::INITIALIZED);
                break;
            }
        }
        if (!found) {
            return nullptr;
        }
    }

    return foundVar->second;
}

ir::ETSImportDeclaration *ETSBinder::FindImportDeclInReExports(const ir::ETSImportDeclaration *const import,
                                                               std::vector<ir::ETSImportDeclaration *> &viewedReExport,
                                                               const util::StringView &imported,
                                                               const ir::StringLiteral *const importPath)
{
    ir::ETSImportDeclaration *implDecl = nullptr;
    for (auto item : ReExportImports()) {
        if (auto source = import->ResolvedSource()->Str(), program = item->GetProgramPath();
            !source.Is(program.Mutf8())) {
            continue;
        }

        viewedReExport.push_back(item->GetETSImportDeclarations());

        auto specifiers = item->GetETSImportDeclarations()->Specifiers();
        if (specifiers[0]->IsImportSpecifier()) {
            if (!std::any_of(specifiers.begin(), specifiers.end(), [&imported](auto it) {
                    return it->AsImportSpecifier()->Local()->Name().Is(imported.Mutf8());
                })) {
                continue;
            }
        } else {
            ArenaVector<parser::Program *> record =
                GetExternalProgram(item->GetETSImportDeclarations()->ResolvedSource()->Str(), importPath);
            if (FindImportSpecifiersVariable(imported, record.front()->GlobalScope()->Bindings(), record) == nullptr) {
                continue;
            }
        }

        // NOTE: ttamas - Duplication check created error
        implDecl = item->GetETSImportDeclarations();
    }
    return implDecl;
}

bool ETSBinder::AddImportSpecifiersToTopBindings(ir::AstNode *const specifier,
                                                 const varbinder::Scope::VariableMap &globalBindings,
                                                 const ir::ETSImportDeclaration *const import,
                                                 const ArenaVector<parser::Program *> &recordRes,
                                                 std::vector<ir::ETSImportDeclaration *> viewedReExport)
{
    if (!specifier->IsImportSpecifier()) {
        return false;
    }
    const ir::StringLiteral *const importPath = import->Source();
    auto insertForeignBinding = [this, specifier, import](const util::StringView &name, Variable *var) {
        if (import->Language().IsDynamic()) {
            dynamicImportVars_.emplace(var, DynamicImportData {import, specifier, var});
        }

        TopScope()->InsertForeignBinding(name, var);
    };

    const auto *const importSpecifier = specifier->AsImportSpecifier();

    if (!importSpecifier->Imported()->IsIdentifier()) {
        return true;
    }

    const auto &imported = importSpecifier->Imported()->AsIdentifier()->Name();

    auto *const var = FindImportSpecifiersVariable(imported, globalBindings, recordRes);

    const auto &localName = [this, importSpecifier, &imported, &importPath]() {
        if (importSpecifier->Local() != nullptr) {
            auto fnc = [&importPath, &imported](const auto &savedSpecifier) {
                return importPath->Str() != savedSpecifier.first && imported == savedSpecifier.second;
            };
            if (!std::any_of(importSpecifiers_.begin(), importSpecifiers_.end(), fnc)) {
                TopScope()->EraseBinding(imported);
            }

            importSpecifiers_.push_back(std::make_pair(importPath->Str(), imported));

            return importSpecifier->Local()->Name();
        }

        return imported;
    }();

    if (var == nullptr) {
        ir::ETSImportDeclaration *implDecl = FindImportDeclInReExports(import, viewedReExport, imported, importPath);
        if (implDecl != nullptr) {
            AddSpecifiersToTopBindings(specifier, implDecl, implDecl->Source(), viewedReExport);
            return true;
        }

        ThrowError(importPath->Start(), "Cannot find imported element " + imported.Mutf8());
    }

    if (var->Declaration()->Node()->IsDefaultExported()) {
        ThrowError(importPath->Start(), "Use the default import syntax to import a default exported element");
    }

    if (!var->Declaration()->Node()->IsExported()) {
        ThrowError(importPath->Start(), "Imported element not exported '" + var->Declaration()->Name().Mutf8() + "'");
    }

    insertForeignBinding(localName, var);
    return true;
}

varbinder::Variable *ETSBinder::FindStaticBinding(const ArenaVector<parser::Program *> &recordRes,
                                                  const ir::StringLiteral *const importPath)
{
    auto predicateFunc = [](const auto &item) { return item.second->Declaration()->Node()->IsDefaultExported(); };
    const auto &staticMethodBindings = recordRes.front()->GlobalClassScope()->StaticMethodScope()->Bindings();
    auto result = std::find_if(staticMethodBindings.begin(), staticMethodBindings.end(), predicateFunc);
    if (result == staticMethodBindings.end()) {
        const auto &staticFieldBindings = recordRes.front()->GlobalClassScope()->StaticFieldScope()->Bindings();
        result = std::find_if(staticFieldBindings.begin(), staticFieldBindings.end(), predicateFunc);
        if (result == staticFieldBindings.end()) {
            ThrowError(importPath->Start(), "Cannot find default imported element in the target");
        }
    }
    return result->second;
}

ArenaVector<parser::Program *> ETSBinder::GetExternalProgram(const util::StringView &sourceName,
                                                             const ir::StringLiteral *importPath)
{
    const auto &extRecords = globalRecordTable_.Program()->ExternalSources();

    auto [name, _] = GetModuleInfo(sourceName);
    auto res = extRecords.find(name);
    if (res == extRecords.end()) {
        ThrowError(importPath->Start(), "Cannot find import: " + importPath->Str().Mutf8());
    }

    return res->second;
}

void ETSBinder::AddSpecifiersToTopBindings(ir::AstNode *const specifier, const ir::ETSImportDeclaration *const import,
                                           ir::StringLiteral *path,
                                           std::vector<ir::ETSImportDeclaration *> viewedReExport)
{
    const ir::StringLiteral *const importPath = path;

    if (import->IsPureDynamic()) {
        AddDynamicSpecifiersToTopBindings(specifier, import);
        return;
    }

    const util::StringView sourceName = import->ResolvedSource()->Str();

    auto record = GetExternalProgram(sourceName, importPath);
    const auto *const importProgram = record.front();
    const auto *const importGlobalScope = importProgram->GlobalScope();
    const auto &globalBindings = importGlobalScope->Bindings();

    auto insertForeignBinding = [this, specifier, import](const util::StringView &name, Variable *var) {
        if (import->Language().IsDynamic()) {
            dynamicImportVars_.emplace(var, DynamicImportData {import, specifier, var});
        }

        TopScope()->InsertForeignBinding(name, var);
    };

    if (AddImportNamespaceSpecifiersToTopBindings(specifier, globalBindings, importProgram, importGlobalScope,
                                                  import)) {
        return;
    }

    if (AddImportSpecifiersToTopBindings(specifier, globalBindings, import, record, std::move(viewedReExport))) {
        return;
    }

    ASSERT(specifier->IsImportDefaultSpecifier());
    auto predicateFunc = [](const auto &item) { return item.second->Declaration()->Node()->IsDefaultExported(); };

    auto item = std::find_if(globalBindings.begin(), globalBindings.end(), predicateFunc);
    if (item == globalBindings.end()) {
        insertForeignBinding(specifier->AsImportDefaultSpecifier()->Local()->Name(),
                             FindStaticBinding(record, importPath));
        return;
    }

    insertForeignBinding(specifier->AsImportDefaultSpecifier()->Local()->Name(), item->second);
}

void ETSBinder::HandleCustomNodes(ir::AstNode *childNode)
{
    switch (childNode->Type()) {
        case ir::AstNodeType::ETS_TYPE_REFERENCE: {
            auto *typeRef = childNode->AsETSTypeReference();
            auto *baseName = typeRef->BaseName();
            ASSERT(baseName->IsReference());
            // We allow to resolve following types in pure dynamic mode:
            // import * as I from "@dynamic"
            // let x : I.X.Y
            bool allowDynamicNamespaces = typeRef->Part()->Name() != baseName;
            LookupTypeReference(baseName, allowDynamicNamespaces);
            LookupTypeArgumentReferences(typeRef);
            break;
        }
        case ir::AstNodeType::TS_INTERFACE_DECLARATION: {
            BuildInterfaceDeclaration(childNode->AsTSInterfaceDeclaration());
            break;
        }
        case ir::AstNodeType::TS_ENUM_DECLARATION: {
            ResolveEnumDeclaration(childNode->AsTSEnumDeclaration());
            break;
        }
        case ir::AstNodeType::EXPORT_NAMED_DECLARATION: {
            break;
        }
        case ir::AstNodeType::ETS_IMPORT_DECLARATION: {
            BuildImportDeclaration(childNode->AsETSImportDeclaration());
            break;
        }
        case ir::AstNodeType::MEMBER_EXPRESSION: {
            BuildMemberExpression(childNode->AsMemberExpression());
            break;
        }
        case ir::AstNodeType::METHOD_DEFINITION: {
            BuildMethodDefinition(childNode->AsMethodDefinition());
            break;
        }
        case ir::AstNodeType::ETS_NEW_CLASS_INSTANCE_EXPRESSION: {
            BuildETSNewClassInstanceExpression(childNode->AsETSNewClassInstanceExpression());
            break;
        }
        case ir::AstNodeType::ETS_FUNCTION_TYPE: {
            BuildSignatureDeclarationBaseParams(childNode);
            break;
        }
        default: {
            ResolveReferences(childNode);
            break;
        }
    }
}

bool ETSBinder::BuildInternalName(ir::ScriptFunction *scriptFunc)
{
    const bool isExternal = recordTable_->IsExternal();
    if (isExternal) {
        scriptFunc->AddFlag(ir::ScriptFunctionFlags::EXTERNAL);
    }

    if (scriptFunc->IsArrow()) {
        return true;
    }

    auto *funcScope = scriptFunc->Scope();
    funcScope->BindName(recordTable_->RecordName());

    bool compilable = scriptFunc->Body() != nullptr && !isExternal;
    if (!compilable) {
        recordTable_->Signatures().push_back(funcScope);
    }

    return compilable;
}

void ETSBinder::AddCompilableFunction(ir::ScriptFunction *func)
{
    if (func->IsArrow() || func->IsAsyncFunc()) {
        return;
    }

    AddCompilableFunctionScope(func->Scope());
}

void ETSBinder::BuildFunctionName(const ir::ScriptFunction *func) const
{
    auto *funcScope = func->Scope();

    std::stringstream ss;
    ASSERT(func->IsArrow() || !funcScope->Name().Empty());
    ss << funcScope->Name() << compiler::Signatures::METHOD_SEPARATOR;

    const auto *signature = func->Signature();

    if (func->IsStaticBlock()) {
        ss << compiler::Signatures::CCTOR;
    } else if (func->IsConstructor()) {
        ss << compiler::Signatures::CTOR;
    } else {
        if (func->IsGetter()) {
            ss << compiler::Signatures::GETTER_BEGIN;
        } else if (func->IsSetter()) {
            ss << compiler::Signatures::SETTER_BEGIN;
        }
        ss << util::Helpers::FunctionName(Allocator(), func);
    }

    signature->ToAssemblerType(GetCompilerContext(), ss);

    util::UString internalName(ss.str(), Allocator());
    funcScope->BindInternalName(internalName.View());
}

void ETSBinder::FormLambdaName(util::UString &name, const util::StringView &signature)
{
    name.Append(compiler::Signatures::LAMBDA_SEPARATOR);
    auto replaced = std::string(signature.Utf8());
    std::replace(replaced.begin(), replaced.end(), '.', '-');
    std::replace(replaced.begin(), replaced.end(), ':', '-');
    std::replace(replaced.begin(), replaced.end(), ';', '-');
    replaced.append(std::to_string(0));
    name.Append(replaced);
}

void ETSBinder::BuildLambdaObjectName(const ir::AstNode *refNode)
{
    auto found = lambdaObjects_.find(refNode);
    ASSERT(found != lambdaObjects_.end());
    auto *lambdaClass = found->second.first;
    auto *signatureRef = found->second.second;

    util::UString lambdaObjectName(lambdaClass->Ident()->Name(), Allocator());
    FormLambdaName(lambdaObjectName, signatureRef->InternalName());
    lambdaClass->Ident()->SetName(lambdaObjectName.View());
    lambdaClass->SetInternalName(lambdaClass->Ident()->Name());

    util::StringView assemblerName(lambdaClass->Ident()->Name());
    auto *program = static_cast<const ir::ETSScript *>(refNode->GetTopStatement())->Program();
    util::StringView prefix = program->GetPackageName();
    if (!prefix.Empty()) {
        util::UString fullPath(prefix, Allocator());
        fullPath.Append('.');
        fullPath.Append(assemblerName);
        assemblerName = fullPath.View();
    }

    checker::ETSObjectType *lambdaObject = lambdaClass->TsType()->AsETSObjectType();
    lambdaObject->SetName(lambdaClass->Ident()->Name());
    lambdaObject->SetAssemblerName(lambdaClass->Ident()->Name());

    const auto &lambdaBody = lambdaClass->Body();
    auto *ctorFunc = lambdaBody[lambdaBody.size() - 3U]->AsMethodDefinition()->Function();
    auto *ctorFuncScope = ctorFunc->Scope();
    ctorFuncScope->BindName(lambdaClass->Ident()->Name());

    auto *invoke0Func = lambdaBody[lambdaBody.size() - 2U]->AsMethodDefinition()->Function();
    auto *invoke0FuncScope = invoke0Func->Scope();
    invoke0FuncScope->BindName(lambdaClass->Ident()->Name());

    auto *invokeFunc = lambdaBody[lambdaBody.size() - 1U]->AsMethodDefinition()->Function();
    auto *invokeFuncScope = invokeFunc->Scope();
    invokeFuncScope->BindName(lambdaClass->Ident()->Name());
}

void ETSBinder::InitImplicitThisParam()
{
    thisParam_ = Allocator()->New<ir::Identifier>("this", Allocator());
}

void ETSBinder::BuildProgram()
{
    for (auto &[_, extPrograms] : Program()->ExternalSources()) {
        (void)_;
        for (auto *extProg : extPrograms) {
            BuildExternalProgram(extProg);
        }
    }

    for (auto *defaultImport : defaultImports_) {
        BuildImportDeclaration(defaultImport);
    }

    auto &stmts = Program()->Ast()->Statements();
    const auto etsGlobal = std::find_if(stmts.begin(), stmts.end(), [](const ir::Statement *stmt) {
        return stmt->IsClassDeclaration() &&
               stmt->AsClassDeclaration()->Definition()->Ident()->Name().Is(compiler::Signatures::ETS_GLOBAL);
    });
    if (etsGlobal != stmts.end()) {
        const auto begin = std::find_if(stmts.rbegin(), stmts.rend(), [](const ir::Statement *stmt) {
                               return stmt->IsETSImportDeclaration() || stmt->IsETSPackageDeclaration();
                           }).base();

        const auto index = std::distance(begin, etsGlobal);
        std::rotate(begin, begin + index, begin + index + 1);
    }

    for (auto *stmt : stmts) {
        ResolveReference(stmt);
    }
}

void ETSBinder::BuildExternalProgram(parser::Program *extProgram)
{
    auto *savedProgram = Program();
    auto *savedRecordTable = recordTable_;
    auto *savedTopScope = TopScope();

    auto flags = Program()->VarBinder()->IsGenStdLib() ? RecordTableFlags::NONE : RecordTableFlags::EXTERNAL;
    auto *extRecordTable = Allocator()->New<RecordTable>(Allocator(), extProgram, flags);
    externalRecordTable_.insert({extProgram, extRecordTable});

    ResetTopScope(extProgram->GlobalScope());
    recordTable_ = extRecordTable;
    SetProgram(extProgram);

    BuildProgram();

    SetProgram(savedProgram);
    recordTable_ = savedRecordTable;
    ResetTopScope(savedTopScope);
}

void ETSBinder::BuildETSNewClassInstanceExpression(ir::ETSNewClassInstanceExpression *classInstance)
{
    BoundContext boundCtx(recordTable_, classInstance->ClassDefinition());
    ResolveReference(classInstance->GetTypeRef());

    for (auto *arg : classInstance->GetArguments()) {
        ResolveReference(arg);
    }

    if (classInstance->ClassDefinition() == nullptr) {
        return;
    }

    ResolveReference(classInstance->ClassDefinition());
}

void ETSBinder::BuildImportDeclaration(ir::ETSImportDeclaration *decl)
{
    if (decl->Source()->Str() == Program()->SourceFile().GetAbsolutePath()) {
        return;
    }

    const auto &specifiers = decl->Specifiers();

    for (auto specifier : specifiers) {
        AddSpecifiersToTopBindings(specifier, decl, decl->Source());
    }
}

bool ETSBinder::ImportGlobalPropertiesForNotDefaultedExports(varbinder::Variable *var, const util::StringView &name,
                                                             const ir::ClassElement *classElement)
{
    if (var->Declaration()->Node()->IsDefaultExported()) {
        return false;
    }

    const auto insRes = TopScope()->InsertForeignBinding(name, var);
    if (!(!insRes.second && insRes.first != TopScope()->Bindings().end()) || !(insRes.first->second != var)) {
        return true;
    }
    if (insRes.first->second->Declaration()->IsFunctionDecl() && var->Declaration()->IsFunctionDecl()) {
        auto *const currentNode = insRes.first->second->Declaration()->Node();
        auto *const method = var->Declaration()->Node()->AsMethodDefinition();
        if (!currentNode->AsMethodDefinition()->HasOverload(method)) {
            currentNode->AsMethodDefinition()->AddOverload(method);
            method->Function()->Id()->SetVariable(insRes.first->second);
            method->Function()->AddFlag(ir::ScriptFunctionFlags::OVERLOAD);
        }
        return true;
    }

    auto str = util::Helpers::AppendAll("Variable '", name.Utf8(), "'");
    if (insRes.first->second->Declaration()->Type() == var->Declaration()->Type()) {
        str += " is already defined.";
    } else {
        str += " is already defined with different type.";
    }
    ThrowError(classElement->Id()->Start(), str);
}

void ETSBinder::ImportGlobalProperties(const ir::ClassDefinition *const classDef)
{
    const auto scopeCtx = LexicalScope<ClassScope>::Enter(this, classDef->Scope()->AsClassScope());

    for (const auto *const prop : classDef->Body()) {
        const auto *const classElement = prop->AsClassElement();

        if (classElement->IsClassStaticBlock()) {
            continue;
        }

        ASSERT(classElement->IsStatic());
        const auto &name = classElement->Id()->Name();
        auto *const var = scopeCtx.GetScope()->FindLocal(name, ResolveBindingOptions::ALL);
        ASSERT(var != nullptr);

        if (ImportGlobalPropertiesForNotDefaultedExports(var, name, classElement)) {
            return;
        }
    }
}

const DynamicImportData *ETSBinder::DynamicImportDataForVar(const Variable *var) const
{
    auto it = dynamicImportVars_.find(var);
    if (it == dynamicImportVars_.cend()) {
        return nullptr;
    }

    return &it->second;
}

bool ETSBinder::IsDynamicModuleVariable(const Variable *var) const
{
    auto *data = DynamicImportDataForVar(var);
    if (data == nullptr) {
        return false;
    }

    return data->specifier->IsImportSpecifier();
}

bool ETSBinder::IsDynamicNamespaceVariable(const Variable *var) const
{
    auto *data = DynamicImportDataForVar(var);
    if (data == nullptr) {
        return false;
    }

    return data->specifier->IsImportNamespaceSpecifier();
}

}  // namespace ark::es2panda::varbinder
