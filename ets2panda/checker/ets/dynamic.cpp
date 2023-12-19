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

#include <utility>
#include "checker/ETSchecker.h"

#include "varbinder/scope.h"
#include "varbinder/declaration.h"
#include "varbinder/varbinder.h"
#include "varbinder/ETSBinder.h"
#include "checker/types/ets/etsDynamicFunctionType.h"
#include "ir/base/classProperty.h"
#include "ir/base/classStaticBlock.h"
#include "ir/base/methodDefinition.h"
#include "ir/base/scriptFunction.h"
#include "ir/expressions/assignmentExpression.h"
#include "ir/expressions/callExpression.h"
#include "ir/expressions/functionExpression.h"
#include "ir/expressions/identifier.h"
#include "ir/expressions/thisExpression.h"
#include "ir/expressions/memberExpression.h"
#include "ir/ets/etsPrimitiveType.h"
#include "ir/ts/tsAsExpression.h"
#include "ir/statements/blockStatement.h"
#include "ir/statements/classDeclaration.h"
#include "ir/statements/expressionStatement.h"
#include "ir/statements/returnStatement.h"
#include "ir/statements/variableDeclaration.h"
#include "ir/statements/variableDeclarator.h"
#include "parser/program/program.h"
#include "util/helpers.h"
#include "util/language.h"
#include "generated/signatures.h"
#include "ir/ets/etsParameterExpression.h"

namespace ark::es2panda::checker {

ir::ETSParameterExpression *ETSChecker::AddParam(varbinder::FunctionParamScope *paramScope, util::StringView name,
                                                 checker::Type *type)
{
    auto paramCtx = varbinder::LexicalScope<varbinder::FunctionParamScope>::Enter(VarBinder(), paramScope, false);
    auto *paramIdent = AllocNode<ir::Identifier>(name, Allocator());
    auto *param = AllocNode<ir::ETSParameterExpression>(paramIdent, nullptr);
    auto *paramVar = std::get<1>(VarBinder()->AddParamDecl(param));
    paramVar->SetTsType(type);
    param->Ident()->SetVariable(paramVar);
    param->Ident()->SetTsType(type);
    return param;
}

static bool IsByValueCall(varbinder::ETSBinder *varbinder, ir::Expression *callee)
{
    if (callee->IsMemberExpression()) {
        return !callee->AsMemberExpression()->ObjType()->IsETSDynamicType();
    }

    if (callee->IsETSTypeReference()) {
        return false;
    }

    auto *var = callee->AsIdentifier()->Variable();
    auto *data = varbinder->DynamicImportDataForVar(var);
    if (data != nullptr) {
        auto *specifier = data->specifier;
        if (specifier->IsImportSpecifier()) {
            return false;
        }
    }

    return true;
}

template <typename T>
ir::ScriptFunction *ETSChecker::CreateDynamicCallIntrinsic(ir::Expression *callee, const ArenaVector<T *> &arguments,
                                                           Language lang)
{
    auto *paramScope = Allocator()->New<varbinder::FunctionParamScope>(Allocator(), nullptr);
    auto *scope = Allocator()->New<varbinder::FunctionScope>(Allocator(), paramScope);

    ArenaVector<ir::Expression *> params(Allocator()->Adapter());

    auto *info = CreateSignatureInfo();
    info->minArgCount = arguments.size() + 2U;

    auto dynamicType = GlobalBuiltinDynamicType(lang);

    auto *objParam = AddParam(paramScope, "obj", dynamicType);
    params.push_back(objParam);
    info->params.push_back(objParam->Ident()->Variable()->AsLocalVariable());

    ir::ETSParameterExpression *param2;
    if (!IsByValueCall(VarBinder()->AsETSBinder(), callee)) {
        param2 = AddParam(paramScope, "qname_start", GlobalIntType());
        params.push_back(param2);
        info->params.push_back(param2->Ident()->Variable()->AsLocalVariable());
        param2 = AddParam(paramScope, "qname_len", GlobalIntType());
    } else {
        param2 = AddParam(paramScope, "this", dynamicType);
    }

    params.push_back(param2);
    info->params.push_back(param2->Ident()->Variable()->AsLocalVariable());

    for (size_t i = 0; i < arguments.size(); i++) {
        util::UString paramName("p" + std::to_string(i), Allocator());
        Type *paramType =
            arguments[i]->TsType()->IsLambdaObject() ? GlobalBuiltinJSValueType() : arguments[i]->TsType();
        ir::ETSParameterExpression *param = AddParam(paramScope, paramName.View(), paramType);
        params.push_back(param);
        info->params.push_back(param->Ident()->Variable()->AsLocalVariable());
    }

    auto *func = AllocNode<ir::ScriptFunction>(
        ir::FunctionSignature(nullptr, std::move(params), nullptr), nullptr,
        ir::ScriptFunction::ScriptFunctionData {ir::ScriptFunctionFlags::METHOD, ir::ModifierFlags::NONE});
    func->SetScope(scope);

    scope->BindNode(func);
    paramScope->BindNode(func);
    scope->BindParamScope(paramScope);
    paramScope->BindFunctionScope(scope);

    auto *name = AllocNode<ir::Identifier>("invoke", Allocator());
    func->SetIdent(name);

    auto *signature = CreateSignature(info, dynamicType, func);
    signature->AddSignatureFlag(SignatureFlags::STATIC);

    func->SetSignature(signature);
    signature->SetOwner(Context().ContainingClass());

    return func;
}

static void ToString(ETSChecker *checker, const ArenaVector<ir::Expression *> &arguments, std::stringstream &ss)
{
    for (auto *arg : arguments) {
        auto *type = arg->Check(checker);
        ss << "-";
        type->ToString(ss);
    }
}

static void ToString([[maybe_unused]] ETSChecker *checker, const ArenaVector<varbinder::LocalVariable *> &arguments,
                     std::stringstream &ss)
{
    for (auto *arg : arguments) {
        auto *type = arg->TsType();
        ss << "-";
        type->ToString(ss);
    }
}

void ETSChecker::CreateDynamicCallQualifiedName(ir::Expression *callee, bool isConstruct)
{
    ir::Expression *obj = callee;
    ArenaVector<util::StringView> parts(Allocator()->Adapter());

    if (isConstruct) {
        auto *name = obj->AsETSTypeReference()->Part()->Name();
        while (name->IsTSQualifiedName()) {
            auto *qname = name->AsTSQualifiedName();
            name = qname->Left();
            parts.push_back(qname->Right()->AsIdentifier()->Name());
        }
        obj = name;
    } else {
        while (obj->IsMemberExpression() && obj->AsMemberExpression()->ObjType()->IsETSDynamicType()) {
            auto *memExpr = obj->AsMemberExpression();
            obj = memExpr->Object();
            parts.push_back(memExpr->Property()->AsIdentifier()->Name());
        }
    }
    if (obj->IsIdentifier()) {
        auto *var = obj->AsIdentifier()->Variable();
        auto *data = VarBinder()->AsETSBinder()->DynamicImportDataForVar(var);
        if (data != nullptr) {
            ASSERT(data->import->Language().IsDynamic());
            auto *specifier = data->specifier;
            if (specifier->IsImportSpecifier()) {
                parts.push_back(specifier->AsImportSpecifier()->Imported()->Name());
            }
        }
    }

    if (parts.empty()) {
        return;
    }
    std::reverse(parts.begin(), parts.end());
    DynamicCallNames(isConstruct)->try_emplace(parts, 0);
}

template <typename T>
Signature *ETSChecker::ResolveDynamicCallExpression(ir::Expression *callee, const ArenaVector<T *> &arguments,
                                                    Language lang, bool isConstruct)
{
    auto &dynamicIntrinsics = *DynamicCallIntrinsics(isConstruct);

    auto mapIt = dynamicIntrinsics.find(lang);
    if (mapIt == dynamicIntrinsics.cend()) {
        std::tie(mapIt, std::ignore) = dynamicIntrinsics.emplace(lang, Allocator()->Adapter());
    }

    auto &map = mapIt->second;

    std::stringstream ss;
    ss << "dyncall";
    if (IsByValueCall(VarBinder()->AsETSBinder(), callee)) {
        ss << "-byvalue";
    } else {
        CreateDynamicCallQualifiedName(callee, isConstruct);
    }

    ToString(this, arguments, ss);

    auto key = ss.str();
    auto it = map.find(util::StringView(key));
    if (it == map.end()) {
        auto *func = CreateDynamicCallIntrinsic(callee, arguments, lang);
        map.emplace(util::UString(key, Allocator()).View(), func);
        return func->Signature();
    }

    return it->second->Signature();
}

template Signature *ETSChecker::ResolveDynamicCallExpression<ir::Expression>(
    ir::Expression *callee, const ArenaVector<ir::Expression *> &arguments, Language lang, bool isConstruct);

template Signature *ETSChecker::ResolveDynamicCallExpression<varbinder::LocalVariable>(
    ir::Expression *callee, const ArenaVector<varbinder::LocalVariable *> &arguments, Language lang, bool isConstruct);

template <bool IS_STATIC>
std::pair<ir::ScriptFunction *, ir::Identifier *> ETSChecker::CreateScriptFunction(
    varbinder::FunctionScope *scope, ClassInitializerBuilder const &builder)
{
    ArenaVector<ir::Statement *> statements(Allocator()->Adapter());
    ArenaVector<ir::Expression *> params(Allocator()->Adapter());

    ir::ScriptFunction *func;
    ir::Identifier *id;

    if constexpr (IS_STATIC) {
        builder(scope, &statements, nullptr);
        auto *body = AllocNode<ir::BlockStatement>(Allocator(), std::move(statements));
        body->SetScope(scope);
        id = AllocNode<ir::Identifier>(compiler::Signatures::CCTOR, Allocator());
        func = AllocNode<ir::ScriptFunction>(
            ir::FunctionSignature(nullptr, std::move(params), nullptr), body,
            ir::ScriptFunction::ScriptFunctionData {ir::ScriptFunctionFlags::STATIC_BLOCK |
                                                        ir::ScriptFunctionFlags::EXPRESSION,
                                                    ir::ModifierFlags::STATIC});
    } else {
        builder(scope, &statements, &params);
        auto *body = AllocNode<ir::BlockStatement>(Allocator(), std::move(statements));
        body->SetScope(scope);
        id = AllocNode<ir::Identifier>(compiler::Signatures::CTOR, Allocator());
        func = AllocNode<ir::ScriptFunction>(
            ir::FunctionSignature(nullptr, std::move(params), nullptr), body,
            ir::ScriptFunction::ScriptFunctionData {
                ir::ScriptFunctionFlags::CONSTRUCTOR | ir::ScriptFunctionFlags::EXPRESSION, ir::ModifierFlags::PUBLIC});
    }

    func->SetScope(scope);
    scope->BindNode(func);
    func->SetIdent(id);

    return std::make_pair(func, id);
}

template <bool IS_STATIC>
std::conditional_t<IS_STATIC, ir::ClassStaticBlock *, ir::MethodDefinition *> ETSChecker::CreateClassInitializer(
    varbinder::ClassScope *classScope, const ClassInitializerBuilder &builder, ETSObjectType *type)
{
    varbinder::LocalScope *methodScope = nullptr;
    if constexpr (IS_STATIC) {
        methodScope = classScope->StaticMethodScope();
    } else {
        methodScope = classScope->InstanceMethodScope();
    }
    auto classCtx = varbinder::LexicalScope<varbinder::LocalScope>::Enter(VarBinder(), methodScope);

    auto *paramScope = Allocator()->New<varbinder::FunctionParamScope>(Allocator(), classScope);
    auto *scope = Allocator()->New<varbinder::FunctionScope>(Allocator(), paramScope);

    auto [func, id] = CreateScriptFunction<IS_STATIC>(scope, builder);

    paramScope->BindNode(func);
    scope->BindParamScope(paramScope);
    paramScope->BindFunctionScope(scope);

    auto *signatureInfo = CreateSignatureInfo();
    signatureInfo->restVar = nullptr;
    auto *signature = CreateSignature(signatureInfo, GlobalVoidType(), func);
    func->SetSignature(signature);

    auto *funcExpr = AllocNode<ir::FunctionExpression>(func);

    VarBinder()->AsETSBinder()->BuildInternalName(func);
    VarBinder()->AsETSBinder()->BuildFunctionName(func);
    VarBinder()->Functions().push_back(func->Scope());

    if constexpr (IS_STATIC) {
        auto *staticBlock = AllocNode<ir::ClassStaticBlock>(funcExpr, Allocator());
        staticBlock->AddModifier(ir::ModifierFlags::STATIC);
        return staticBlock;
    } else {
        type->AddConstructSignature(signature);

        auto *ctor =
            AllocNode<ir::MethodDefinition>(ir::MethodDefinitionKind::CONSTRUCTOR, id->Clone(Allocator(), nullptr),
                                            funcExpr, ir::ModifierFlags::NONE, Allocator(), false);
        auto *funcType = CreateETSFunctionType(signature, id->Name());
        ctor->SetTsType(funcType);
        return ctor;
    }
}

ir::ClassStaticBlock *ETSChecker::CreateDynamicCallClassInitializer(varbinder::ClassScope *classScope, Language lang,
                                                                    bool isConstruct)
{
    return CreateClassInitializer<true>(
        classScope, [this, lang, isConstruct](varbinder::FunctionScope *scope, ArenaVector<ir::Statement *> *statements,
                                              [[maybe_unused]] ArenaVector<ir::Expression *> *params) {
            auto [builtin_class_name, builtin_method_name] =
                util::Helpers::SplitSignature(isConstruct ? compiler::Signatures::Dynamic::InitNewClassBuiltin(lang)
                                                          : compiler::Signatures::Dynamic::InitCallClassBuiltin(lang));
            auto *classId = AllocNode<ir::Identifier>(builtin_class_name, Allocator());
            auto *methodId = AllocNode<ir::Identifier>(builtin_method_name, Allocator());
            auto *callee = AllocNode<ir::MemberExpression>(classId, methodId, ir::MemberExpressionKind::PROPERTY_ACCESS,
                                                           false, false);

            ArenaVector<ir::Expression *> callParams(Allocator()->Adapter());

            std::stringstream ss;
            auto name = isConstruct ? compiler::Signatures::Dynamic::NewClass(lang)
                                    : compiler::Signatures::Dynamic::CallClass(lang);
            auto package = VarBinder()->Program()->GetPackageName();

            ss << compiler::Signatures::CLASS_REF_BEGIN;
            if (!package.Empty()) {
                std::string packageStr(package);
                std::replace(packageStr.begin(), packageStr.end(), *compiler::Signatures::METHOD_SEPARATOR.begin(),
                             *compiler::Signatures::NAMESPACE_SEPARATOR.begin());
                ss << packageStr << compiler::Signatures::NAMESPACE_SEPARATOR;
            }
            ss << name << compiler::Signatures::MANGLE_SEPARATOR;

            auto *className = AllocNode<ir::StringLiteral>(util::UString(ss.str(), Allocator()).View());
            callParams.push_back(className);

            auto *initCall = AllocNode<ir::CallExpression>(callee, std::move(callParams), nullptr, false);

            {
                ScopeContext ctx(this, scope);
                initCall->Check(this);
            }

            statements->push_back(AllocNode<ir::ExpressionStatement>(initCall));
        });
}

void ETSChecker::BuildClass(util::StringView name, const ClassBuilder &builder)
{
    auto *classId = AllocNode<ir::Identifier>(name, Allocator());
    auto [decl, var] = VarBinder()->NewVarDecl<varbinder::ClassDecl>(classId->Start(), classId->Name());
    classId->SetVariable(var);

    auto classCtx = varbinder::LexicalScope<varbinder::ClassScope>(VarBinder());

    auto *classDef = AllocNode<ir::ClassDefinition>(Allocator(), classId, ir::ClassDefinitionModifiers::DECLARATION,
                                                    ir::ModifierFlags::NONE, Language(Language::Id::ETS));
    classDef->SetScope(classCtx.GetScope());

    auto *classDefType =
        Allocator()->New<checker::ETSObjectType>(Allocator(), classDef->Ident()->Name(), classDef->Ident()->Name(),
                                                 classDef, checker::ETSObjectFlags::CLASS, Relation());
    classDef->SetTsType(classDefType);

    auto *classDecl = AllocNode<ir::ClassDeclaration>(classDef, Allocator());
    classDecl->SetParent(VarBinder()->TopScope()->Node());
    classDef->Scope()->BindNode(classDecl);
    decl->BindNode(classDef);

    VarBinder()->Program()->Ast()->Statements().push_back(classDecl);

    varbinder::BoundContext boundCtx(VarBinder()->AsETSBinder()->GetGlobalRecordTable(), classDef);

    ArenaVector<ir::AstNode *> classBody(Allocator()->Adapter());

    builder(classCtx.GetScope(), &classBody);

    classDef->AddProperties(std::move(classBody));
}

ir::ClassProperty *ETSChecker::CreateStaticReadonlyField(varbinder::ClassScope *scope, const char *name)
{
    auto *fieldIdent = AllocNode<ir::Identifier>(name, Allocator());
    // NOTE: remove const when readonly is properly supported
    auto flags =
        ir::ModifierFlags::STATIC | ir::ModifierFlags::PRIVATE | ir::ModifierFlags::READONLY | ir::ModifierFlags::CONST;
    auto *field = AllocNode<ir::ClassProperty>(fieldIdent, nullptr, nullptr, flags, Allocator(), false);
    field->SetTsType(GlobalIntType());

    auto *decl = Allocator()->New<varbinder::LetDecl>(fieldIdent->Name());
    decl->BindNode(field);

    auto *var = scope->AddDecl(Allocator(), decl, VarBinder()->Extension());
    var->AddFlag(varbinder::VariableFlags::PROPERTY);
    fieldIdent->SetVariable(var);
    var->SetTsType(GlobalIntType());

    auto *classType = scope->Node()->AsClassDeclaration()->Definition()->TsType()->AsETSObjectType();
    classType->AddProperty<PropertyType::STATIC_FIELD>(var->AsLocalVariable());

    return field;
}

void ETSChecker::BuildDynamicCallClass(bool isConstruct)
{
    auto &dynamicIntrinsics = *DynamicCallIntrinsics(isConstruct);

    if (dynamicIntrinsics.empty()) {
        return;
    }

    for (auto &entry : dynamicIntrinsics) {
        auto lang = entry.first;
        auto &intrinsics = entry.second;
        auto className = isConstruct ? compiler::Signatures::Dynamic::NewClass(lang)
                                     : compiler::Signatures::Dynamic::CallClass(lang);
        BuildClass(className, [this, lang, &intrinsics, isConstruct](varbinder::ClassScope *scope,
                                                                     ArenaVector<ir::AstNode *> *classBody) {
            for (auto &[_, func] : intrinsics) {
                (void)_;

                func->Scope()->ParamScope()->SetParent(scope);

                auto *funcExpr = AllocNode<ir::FunctionExpression>(func);

                auto *method = AllocNode<ir::MethodDefinition>(
                    ir::MethodDefinitionKind::METHOD, func->Id()->Clone(Allocator(), nullptr), funcExpr,
                    ir::ModifierFlags::PUBLIC | ir::ModifierFlags::NATIVE | ir::ModifierFlags::STATIC, Allocator(),
                    false);

                VarBinder()->AsETSBinder()->BuildInternalName(func);
                VarBinder()->AsETSBinder()->BuildFunctionName(func);

                classBody->push_back(method);
            }

            classBody->push_back(CreateStaticReadonlyField(scope, "qname_start_from"));

            classBody->push_back(CreateDynamicCallClassInitializer(scope, lang, isConstruct));
        });
    }
}

void ETSChecker::ClassInitializerFromImport(ir::ETSImportDeclaration *import, varbinder::FunctionScope *scope,
                                            ArenaVector<ir::Statement *> *statements)
{
    auto builtin = compiler::Signatures::Dynamic::LoadModuleBuiltin(import->Language());
    auto [builtin_class_name, builtin_method_name] = util::Helpers::SplitSignature(builtin);

    auto *classId = AllocNode<ir::Identifier>(builtin_class_name, Allocator());
    auto *methodId = AllocNode<ir::Identifier>(builtin_method_name, Allocator());
    auto *callee =
        AllocNode<ir::MemberExpression>(classId, methodId, ir::MemberExpressionKind::PROPERTY_ACCESS, false, false);

    // Note(rsipka): this check could be avoided with appropriate language extensions
    ArenaVector<ir::Expression *> callParams(Allocator()->Adapter());
    if (ark::os::file::File::IsRegularFile(import->ResolvedSource()->Str().Mutf8())) {
        callParams.push_back(AllocNode<ir::StringLiteral>(
            util::UString(ark::os::RemoveExtension(import->ResolvedSource()->Str().Mutf8()), Allocator()).View()));
    } else {
        callParams.push_back(import->ResolvedSource());
    }

    auto *loadCall = AllocNode<ir::CallExpression>(callee, std::move(callParams), nullptr, false);

    auto *moduleClassId = AllocNode<ir::Identifier>(compiler::Signatures::DYNAMIC_MODULE_CLASS, Allocator());
    auto *fieldId = AllocNode<ir::Identifier>(import->AssemblerName(), Allocator());
    auto *property = AllocNode<ir::MemberExpression>(moduleClassId, fieldId, ir::MemberExpressionKind::PROPERTY_ACCESS,
                                                     false, false);

    auto *initializer =
        AllocNode<ir::AssignmentExpression>(property, loadCall, lexer::TokenType::PUNCTUATOR_SUBSTITUTION);

    {
        ScopeContext ctx(this, scope);
        initializer->Check(this);
    }
    statements->push_back(AllocNode<ir::ExpressionStatement>(initializer));
}

ir::ClassStaticBlock *ETSChecker::CreateDynamicModuleClassInitializer(
    varbinder::ClassScope *classScope, const std::vector<ir::ETSImportDeclaration *> &imports)
{
    return CreateClassInitializer<true>(
        classScope, [this, imports](varbinder::FunctionScope *scope, ArenaVector<ir::Statement *> *statements,
                                    [[maybe_unused]] ArenaVector<ir::Expression *> *params) {
            for (auto *import : imports) {
                ClassInitializerFromImport(import, scope, statements);
            }
        });
}

template <bool IS_STATIC>
static void AddMethodToClass(varbinder::ClassScope *classScope, varbinder::Variable *methodVar, Signature *signature)
{
    auto *classType = classScope->Node()->AsClassDeclaration()->Definition()->TsType()->AsETSObjectType();
    if constexpr (IS_STATIC) {
        classType->AddProperty<PropertyType::STATIC_METHOD>(methodVar->AsLocalVariable());
    } else {
        classType->AddProperty<PropertyType::INSTANCE_METHOD>(methodVar->AsLocalVariable());
    }
    signature->SetOwner(classType);
}

template <bool IS_STATIC>
ir::MethodDefinition *ETSChecker::CreateClassMethod(varbinder::ClassScope *classScope, const std::string_view name,
                                                    ir::ModifierFlags modifierFlags, const MethodBuilder &builder)
{
    auto classCtx = varbinder::LexicalScope<varbinder::LocalScope>::Enter(VarBinder(), classScope->StaticMethodScope());
    ArenaVector<ir::Expression *> params(Allocator()->Adapter());
    auto *paramScope = Allocator()->New<varbinder::FunctionParamScope>(Allocator(), classScope);
    auto *scope = Allocator()->New<varbinder::FunctionScope>(Allocator(), paramScope);
    auto *id = AllocNode<ir::Identifier>(name, Allocator());

    ArenaVector<ir::Statement *> statements(Allocator()->Adapter());
    Type *returnType = nullptr;

    builder(scope, &statements, &params, &returnType);

    auto *body = AllocNode<ir::BlockStatement>(Allocator(), std::move(statements));
    body->SetScope(scope);

    auto *func = AllocNode<ir::ScriptFunction>(
        ir::FunctionSignature(nullptr, std::move(params), nullptr), body,
        ir::ScriptFunction::ScriptFunctionData {ir::ScriptFunctionFlags::METHOD, modifierFlags});

    func->SetScope(scope);
    scope->BindNode(func);
    func->SetIdent(id);
    paramScope->BindNode(func);
    scope->BindParamScope(paramScope);
    paramScope->BindFunctionScope(scope);

    auto *signatureInfo = CreateSignatureInfo();
    signatureInfo->restVar = nullptr;
    auto *signature = CreateSignature(signatureInfo, returnType, func);
    if constexpr (IS_STATIC) {
        signature->AddSignatureFlag(SignatureFlags::STATIC);
    }
    func->SetSignature(signature);

    auto *funcExpr = AllocNode<ir::FunctionExpression>(func);
    auto *method =
        AllocNode<ir::MethodDefinition>(ir::MethodDefinitionKind::METHOD, func->Id()->Clone(Allocator(), nullptr),
                                        funcExpr, modifierFlags, Allocator(), false);

    VarBinder()->AsETSBinder()->BuildInternalName(func);
    VarBinder()->AsETSBinder()->BuildFunctionName(func);
    VarBinder()->Functions().push_back(func->Scope());

    auto *decl = Allocator()->New<varbinder::LetDecl>(id->Name());
    decl->BindNode(method);

    auto *funcType = CreateETSFunctionType(signature, id->Name());
    auto *var = scope->AddDecl(Allocator(), decl, VarBinder()->Extension());
    var->SetTsType(funcType);
    method->SetTsType(funcType);
    var->AddFlag(varbinder::VariableFlags::PROPERTY);
    func->Id()->SetVariable(var);
    method->Id()->SetVariable(var);

    AddMethodToClass<IS_STATIC>(classScope, var, signature);

    return method;
}

ir::MethodDefinition *ETSChecker::CreateDynamicModuleClassInitMethod(varbinder::ClassScope *classScope)
{
    return CreateClassMethod<true>(classScope, compiler::Signatures::DYNAMIC_MODULE_CLASS_INIT,
                                   ir::ModifierFlags::PUBLIC | ir::ModifierFlags::STATIC,
                                   [this]([[maybe_unused]] varbinder::FunctionScope *scope,
                                          [[maybe_unused]] ArenaVector<ir::Statement *> *statements,
                                          [[maybe_unused]] ArenaVector<ir::Expression *> *params,
                                          Type **returnType) { *returnType = GlobalVoidType(); });
}

ir::MethodDefinition *ETSChecker::CreateLambdaObjectClassInvokeMethod(varbinder::ClassScope *classScope,
                                                                      Signature *invokeSignature,
                                                                      ir::TypeNode *retTypeAnnotation)
{
    return CreateClassMethod<true>(
        classScope, compiler::Signatures::LAMBDA_OBJECT_INVOKE, ir::ModifierFlags::PUBLIC,
        [this, classScope, invokeSignature,
         retTypeAnnotation](varbinder::FunctionScope *scope, ArenaVector<ir::Statement *> *statements,
                            ArenaVector<ir::Expression *> *params, Type **returnType) {
            util::UString thisParamName(std::string("this"), Allocator());
            ir::ETSParameterExpression *thisParam =
                AddParam(scope->Parent()->AsFunctionParamScope(), thisParamName.View(),
                         classScope->Node()->AsClassDeclaration()->Definition()->TsType()->AsETSObjectType());
            params->push_back(thisParam);

            ArenaVector<ir::Expression *> callParams(Allocator()->Adapter());
            uint32_t idx = 0;
            for (auto *invokeParam : invokeSignature->Params()) {
                ir::ETSParameterExpression *param = AddParam(
                    scope->Parent()->AsFunctionParamScope(),
                    util::UString(std::string("p") + std::to_string(idx), Allocator()).View(), invokeParam->TsType());
                params->push_back(param);
                callParams.push_back(param->Clone(Allocator(), nullptr));
                ++idx;
            }

            auto *properyId = AllocNode<ir::Identifier>("jsvalue_lambda", Allocator());
            auto *callee = AllocNode<ir::MemberExpression>(thisParam->Clone(Allocator(), nullptr), properyId,
                                                           ir::MemberExpressionKind::PROPERTY_ACCESS, false, false);
            auto *callLambda = AllocNode<ir::CallExpression>(callee, std::move(callParams), nullptr, false);

            {
                ScopeContext ctx(this, scope);
                callLambda->Check(this);
            }

            auto *castToRetTypeExpr =
                AllocNode<ir::TSAsExpression>(callLambda, retTypeAnnotation->Clone(Allocator(), nullptr), false);
            castToRetTypeExpr->SetTsType(invokeSignature->ReturnType());
            auto *retStatement = AllocNode<ir::ReturnStatement>(castToRetTypeExpr);
            statements->push_back(retStatement);

            *returnType = invokeSignature->ReturnType();
        });
}

void ETSChecker::EmitDynamicModuleClassInitCall()
{
    auto *globalClass = VarBinder()->Program()->GlobalClass();
    auto &body = globalClass->Body();
    auto it = std::find_if(body.begin(), body.end(), [](ir::AstNode *node) { return node->IsClassStaticBlock(); });

    ASSERT(it != body.end());

    auto *staticBlock = (*it)->AsClassStaticBlock();
    auto *cctorBody = staticBlock->Function()->Body()->AsBlockStatement();

    auto *classId = AllocNode<ir::Identifier>(compiler::Signatures::DYNAMIC_MODULE_CLASS, Allocator());
    auto *methodId = AllocNode<ir::Identifier>(compiler::Signatures::DYNAMIC_MODULE_CLASS_INIT, Allocator());
    auto *callee =
        AllocNode<ir::MemberExpression>(classId, methodId, ir::MemberExpressionKind::PROPERTY_ACCESS, false, false);

    ArenaVector<ir::Expression *> callParams(Allocator()->Adapter());
    auto *initCall = AllocNode<ir::CallExpression>(callee, std::move(callParams), nullptr, false);

    {
        ScopeContext ctx(this, cctorBody->Scope());
        initCall->Check(this);
    }

    auto *const node = AllocNode<ir::ExpressionStatement>(initCall);
    node->SetParent(cctorBody);
    cctorBody->Statements().push_back(node);
}

void ETSChecker::BuildDynamicImportClass()
{
    auto dynamicImports = VarBinder()->AsETSBinder()->DynamicImports();
    if (dynamicImports.empty()) {
        return;
    }

    // clang-format off
    BuildClass(compiler::Signatures::DYNAMIC_MODULE_CLASS,
                [this, dynamicImports](varbinder::ClassScope *scope, ArenaVector<ir::AstNode *> *classBody) {
                    std::unordered_set<util::StringView> fields;
                    std::vector<ir::ETSImportDeclaration *> imports;

                    auto *classType = scope->Node()->AsClassDeclaration()->Definition()->TsType()->AsETSObjectType();

                    for (auto *import : dynamicImports) {
                        auto source = import->Source()->Str();
                        if (fields.find(source) != fields.cend()) {
                            continue;
                        }

                        auto assemblyName = std::string(source);
                        std::replace_if(
                            assemblyName.begin(), assemblyName.end(), [](char c) { return std::isalnum(c) == 0; }, '_');
                        assemblyName.append(std::to_string(fields.size()));

                        import->AssemblerName() = util::UString(assemblyName, Allocator()).View();
                        fields.insert(import->AssemblerName());
                        imports.push_back(import);

                        auto *fieldIdent = AllocNode<ir::Identifier>(import->AssemblerName(), Allocator());
                        // NOTE: remove const when readonly is properly supported
                        auto flags = ir::ModifierFlags::STATIC | ir::ModifierFlags::PUBLIC |
                                ir::ModifierFlags::READONLY  | ir::ModifierFlags::CONST;
                        auto *field = AllocNode<ir::ClassProperty>(fieldIdent, nullptr, nullptr, flags, Allocator(),
                            false);
                        field->SetTsType(GlobalBuiltinDynamicType(import->Language()));

                        auto *decl = Allocator()->New<varbinder::LetDecl>(fieldIdent->Name());
                        decl->BindNode(field);

                        auto *var = scope->AddDecl(Allocator(), decl, VarBinder()->Extension());
                        var->AddFlag(varbinder::VariableFlags::PROPERTY);
                        fieldIdent->SetVariable(var);
                        var->SetTsType(GlobalBuiltinDynamicType(import->Language()));

                        classType->AddProperty<PropertyType::STATIC_FIELD>(var->AsLocalVariable());

                        classBody->push_back(field);
                    }

                    classBody->push_back(CreateDynamicModuleClassInitializer(scope, imports));
                    classBody->push_back(CreateDynamicModuleClassInitMethod(scope));
                });
    // clang-format on

    EmitDynamicModuleClassInitCall();
}

ir::MethodDefinition *ETSChecker::CreateLambdaObjectClassInitializer(varbinder::ClassScope *classScope,
                                                                     ETSObjectType *functionalInterface)
{
    return CreateClassInitializer<false>(
        classScope,
        [this, classScope](varbinder::FunctionScope *scope, ArenaVector<ir::Statement *> *statements,
                           ArenaVector<ir::Expression *> *params) {
            util::UString thisParamName(std::string("this"), Allocator());
            ir::ETSParameterExpression *thisParam =
                AddParam(scope->Parent()->AsFunctionParamScope(), thisParamName.View(),
                         classScope->Node()->AsClassDeclaration()->Definition()->TsType()->AsETSObjectType());
            params->push_back(thisParam);

            util::UString jsvalueParamName(std::string("jsvalue_param"), Allocator());
            ir::ETSParameterExpression *jsvalueParam =
                AddParam(scope->Parent()->AsFunctionParamScope(), jsvalueParamName.View(), GlobalBuiltinJSValueType());
            params->push_back(jsvalueParam);

            auto *moduleClassId = AllocNode<ir::Identifier>("this", Allocator());
            auto *fieldId = AllocNode<ir::Identifier>("jsvalue_lambda", Allocator());
            auto *property = AllocNode<ir::MemberExpression>(moduleClassId, fieldId,
                                                             ir::MemberExpressionKind::PROPERTY_ACCESS, false, false);
            auto *initializer = AllocNode<ir::AssignmentExpression>(property, jsvalueParam->Clone(Allocator(), nullptr),
                                                                    lexer::TokenType::PUNCTUATOR_SUBSTITUTION);
            {
                ScopeContext ctx(this, scope);
                initializer->Check(this);
            }

            statements->push_back(AllocNode<ir::ExpressionStatement>(initializer));
        },
        functionalInterface);
}

void ETSChecker::BuildLambdaObjectClass(ETSObjectType *functionalInterface, ir::TypeNode *retTypeAnnotation)
{
    auto *invokeMethod = functionalInterface->GetOwnProperty<checker::PropertyType::INSTANCE_METHOD>("invoke");
    auto *invokeSignature = invokeMethod->TsType()->AsETSFunctionType()->CallSignatures()[0];

    std::stringstream ss;
    ss << compiler::Signatures::LAMBDA_OBJECT;
    for (auto *arg : invokeSignature->Params()) {
        ss << "-";
        arg->TsType()->ToString(ss);
    }
    static std::string syntheticLambdaObjName {ss.str()};

    if (dynamicLambdaSignatureCache_.find(syntheticLambdaObjName) != dynamicLambdaSignatureCache_.end()) {
        functionalInterface->AddConstructSignature(dynamicLambdaSignatureCache_[syntheticLambdaObjName]);
        return;
    }

    BuildClass(util::StringView(syntheticLambdaObjName),
               [this, invokeSignature, retTypeAnnotation, functionalInterface](varbinder::ClassScope *scope,
                                                                               ArenaVector<ir::AstNode *> *classBody) {
                   auto *classType = scope->Node()->AsClassDeclaration()->Definition()->TsType()->AsETSObjectType();
                   classType->AddInterface(functionalInterface);

                   auto assemblyName = "jsvalue_lambda";
                   auto *fieldIdent = AllocNode<ir::Identifier>(assemblyName, Allocator());
                   auto *field = AllocNode<ir::ClassProperty>(fieldIdent, nullptr, nullptr, ir::ModifierFlags::PRIVATE,
                                                              Allocator(), false);
                   field->SetTsType(GlobalBuiltinJSValueType());

                   auto *decl = Allocator()->New<varbinder::LetDecl>(fieldIdent->Name());
                   decl->BindNode(field);

                   auto *var = scope->AddDecl(Allocator(), decl, VarBinder()->Extension());
                   var->AddFlag(varbinder::VariableFlags::PROPERTY);
                   var->SetTsType(GlobalBuiltinJSValueType());
                   fieldIdent->SetVariable(var);

                   classType->AddProperty<PropertyType::INSTANCE_FIELD>(var->AsLocalVariable());

                   classBody->push_back(field);

                   classBody->push_back(CreateLambdaObjectClassInitializer(scope, functionalInterface));

                   classBody->push_back(CreateLambdaObjectClassInvokeMethod(scope, invokeSignature, retTypeAnnotation));
               });

    dynamicLambdaSignatureCache_[syntheticLambdaObjName] = functionalInterface->ConstructSignatures()[0];
}

}  // namespace ark::es2panda::checker
