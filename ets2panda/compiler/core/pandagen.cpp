/*
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

#include "pandagen.h"

#include "varbinder/varbinder.h"
#include "checker/checker.h"
#include "checker/types/globalTypesHolder.h"
#include "util/helpers.h"
#include "varbinder/scope.h"
#include "varbinder/variable.h"
#include "compiler/base/catchTable.h"
#include "compiler/base/lexenv.h"
#include "compiler/base/literals.h"
#include "compiler/core/compilerContext.h"
#include "compiler/core/labelTarget.h"
#include "compiler/core/regAllocator.h"
#include "compiler/function/asyncFunctionBuilder.h"
#include "compiler/function/asyncGeneratorFunctionBuilder.h"
#include "compiler/function/functionBuilder.h"
#include "compiler/function/generatorFunctionBuilder.h"
#include "es2panda.h"
#include "generated/isa.h"
#include "ir/base/scriptFunction.h"
#include "ir/base/spreadElement.h"
#include "ir/statement.h"
#include "ir/expressions/identifier.h"
#include "ir/expressions/literals/numberLiteral.h"
#include "ir/expressions/literals/stringLiteral.h"

namespace panda::es2panda::compiler {

#ifndef PANDA_WITH_ECMASCRIPT
class EcmaDisabled : public IRNode {
public:
    template <typename... Args>
    explicit EcmaDisabled(const ir::AstNode *node, [[maybe_unused]] Args &&...args) : IRNode(node)
    {
        UNREACHABLE();
    }

    Formats GetFormats() const override
    {
        UNREACHABLE();
    }

    size_t Registers([[maybe_unused]] std::array<VReg *, MAX_REG_OPERAND> *regs) override
    {
        UNREACHABLE();
    }

    size_t Registers([[maybe_unused]] std::array<const VReg *, MAX_REG_OPERAND> *regs) const override
    {
        UNREACHABLE();
    }

    size_t OutRegisters([[maybe_unused]] std::array<OutVReg, MAX_REG_OPERAND> *regs) const override
    {
        UNREACHABLE();
    }

    void Transform([[maybe_unused]] pandasm::Ins *ins, [[maybe_unused]] ProgramElement *program_element,
                   [[maybe_unused]] uint32_t total_regs) const override
    {
        UNREACHABLE();
    }
};

using EcmaLdhole = EcmaDisabled;
using EcmaLdnan = EcmaDisabled;
using EcmaLdinfinity = EcmaDisabled;
using EcmaLdglobal = EcmaDisabled;
using EcmaLdundefined = EcmaDisabled;
using EcmaLdsymbol = EcmaDisabled;
using EcmaLdnull = EcmaDisabled;
using EcmaLdtrue = EcmaDisabled;
using EcmaLdfalse = EcmaDisabled;
using EcmaTryldglobalbyname = EcmaDisabled;
using EcmaTrystglobalbyname = EcmaDisabled;
using EcmaLdobjbyname = EcmaDisabled;
using EcmaStobjbyname = EcmaDisabled;
using EcmaLdobjbyindex = EcmaDisabled;
using EcmaLdobjbyvalue = EcmaDisabled;
using EcmaStobjbyvalue = EcmaDisabled;
using EcmaStobjbyindex = EcmaDisabled;
using EcmaStownbyname = EcmaDisabled;
using EcmaStownbyvalue = EcmaDisabled;
using EcmaStownbyindex = EcmaDisabled;
using EcmaDelobjprop = EcmaDisabled;
using EcmaLdglobalvar = EcmaDisabled;
using EcmaStglobalvar = EcmaDisabled;
using EcmaLdbigint = EcmaDisabled;
using EcmaEqdyn = EcmaDisabled;
using EcmaNoteqdyn = EcmaDisabled;
using EcmaStricteqdyn = EcmaDisabled;
using EcmaStrictnoteqdyn = EcmaDisabled;
using EcmaLessdyn = EcmaDisabled;
using EcmaLesseqdyn = EcmaDisabled;
using EcmaGreaterdyn = EcmaDisabled;
using EcmaGreatereqdyn = EcmaDisabled;
using EcmaTonumber = EcmaDisabled;
using EcmaNegdyn = EcmaDisabled;
using EcmaNotdyn = EcmaDisabled;
using EcmaNegate = EcmaDisabled;
using EcmaIncdyn = EcmaDisabled;
using EcmaDecdyn = EcmaDisabled;
using EcmaEqdyn = EcmaDisabled;
using EcmaNoteqdyn = EcmaDisabled;
using EcmaStricteqdyn = EcmaDisabled;
using EcmaStrictnoteqdyn = EcmaDisabled;
using EcmaLessdyn = EcmaDisabled;
using EcmaLesseqdyn = EcmaDisabled;
using EcmaGreaterdyn = EcmaDisabled;
using EcmaGreatereqdyn = EcmaDisabled;
using EcmaAdd2dyn = EcmaDisabled;
using EcmaSub2dyn = EcmaDisabled;
using EcmaMul2dyn = EcmaDisabled;
using EcmaDiv2dyn = EcmaDisabled;
using EcmaMod2dyn = EcmaDisabled;
using EcmaExpdyn = EcmaDisabled;
using EcmaShl2dyn = EcmaDisabled;
using EcmaShr2dyn = EcmaDisabled;
using EcmaAshr2dyn = EcmaDisabled;
using EcmaAnd2dyn = EcmaDisabled;
using EcmaOr2dyn = EcmaDisabled;
using EcmaXor2dyn = EcmaDisabled;
using EcmaIsindyn = EcmaDisabled;
using EcmaInstanceofdyn = EcmaDisabled;
using EcmaIsundefined = EcmaDisabled;
using EcmaIsundefined = EcmaDisabled;
using EcmaJtrue = EcmaDisabled;
using EcmaIstrue = EcmaDisabled;
using EcmaJfalse = EcmaDisabled;
using EcmaIscoercible = EcmaDisabled;
using EcmaThrowdyn = EcmaDisabled;
using EcmaRethrowdyn = EcmaDisabled;
using EcmaReturnDyn = EcmaDisabled;
using EcmaReturnundefined = EcmaDisabled;
using EcmaCall0thisdyn = EcmaDisabled;
using EcmaCall1thisdyn = EcmaDisabled;
using EcmaCall0dyn = EcmaDisabled;
using EcmaCall1thisdyn = EcmaDisabled;
using EcmaCall1dyn = EcmaDisabled;
using EcmaCall2thisdyn = EcmaDisabled;
using EcmaCall2dyn = EcmaDisabled;
using EcmaCall3thisdyn = EcmaDisabled;
using EcmaCall3dyn = EcmaDisabled;
using EcmaCallithisrangedyn = EcmaDisabled;
using EcmaCallirangedyn = EcmaDisabled;
using EcmaCall1thisdyn = EcmaDisabled;
using EcmaCall1dyn = EcmaDisabled;
using EcmaCall2thisdyn = EcmaDisabled;
using EcmaCall2dyn = EcmaDisabled;
using EcmaCall3thisdyn = EcmaDisabled;
using EcmaCall3dyn = EcmaDisabled;
using EcmaCallithisrangedyn = EcmaDisabled;
using EcmaCallirangedyn = EcmaDisabled;
using EcmaSupercall = EcmaDisabled;
using EcmaSupercallspread = EcmaDisabled;
using EcmaNewobjdynrange = EcmaDisabled;
using EcmaLdhomeobject = EcmaDisabled;
using EcmaDefinemethod = EcmaDisabled;
using EcmaDefineasyncgeneratorfunc = EcmaDisabled;
using EcmaDefineasyncfunc = EcmaDisabled;
using EcmaDefinegeneratorfunc = EcmaDisabled;
using EcmaDefinencfuncdyn = EcmaDisabled;
using EcmaDefinefuncdyn = EcmaDisabled;
using EcmaTypeofdyn = EcmaDisabled;
using EcmaCallspreaddyn = EcmaDisabled;
using EcmaNewobjspreaddyn = EcmaDisabled;
using EcmaGetunmappedargs = EcmaDisabled;
using EcmaNegate = EcmaDisabled;
using EcmaToboolean = EcmaDisabled;
using EcmaTonumber = EcmaDisabled;
using EcmaGetmethod = EcmaDisabled;
using EcmaCreategeneratorobj = EcmaDisabled;
using EcmaCreateasyncgeneratorobj = EcmaDisabled;
using EcmaCreateiterresultobj = EcmaDisabled;
using EcmaSuspendgenerator = EcmaDisabled;
using EcmaSuspendasyncgenerator = EcmaDisabled;
using EcmaSetgeneratorstate = EcmaDisabled;
using EcmaSetgeneratorstate = EcmaDisabled;
using EcmaResumegenerator = EcmaDisabled;
using EcmaGetresumemode = EcmaDisabled;
using EcmaAsyncfunctionenter = EcmaDisabled;
using EcmaAsyncfunctionawait = EcmaDisabled;
using EcmaAsyncfunctionresolve = EcmaDisabled;
using EcmaAsyncfunctionreject = EcmaDisabled;
using EcmaAsyncgeneratorresolve = EcmaDisabled;
using EcmaAsyncgeneratorreject = EcmaDisabled;
using EcmaGettemplateobject = EcmaDisabled;
using EcmaCopyrestargs = EcmaDisabled;
using EcmaGetpropiterator = EcmaDisabled;
using EcmaGetnextpropname = EcmaDisabled;
using EcmaCreateemptyobject = EcmaDisabled;
using EcmaCreateobjectwithbuffer = EcmaDisabled;
using EcmaCreateobjecthavingmethod = EcmaDisabled;
using EcmaSetobjectwithproto = EcmaDisabled;
using EcmaCopydataproperties = EcmaDisabled;
using EcmaDefinegettersetterbyvalue = EcmaDisabled;
using EcmaCreateemptyarray = EcmaDisabled;
using EcmaCreatearraywithbuffer = EcmaDisabled;
using EcmaStarrayspread = EcmaDisabled;
using EcmaCreateregexpwithliteral = EcmaDisabled;
using EcmaThrowifnotobject = EcmaDisabled;
using EcmaThrowthrownotexists = EcmaDisabled;
using EcmaGetiterator = EcmaDisabled;
using EcmaGetasynciterator = EcmaDisabled;
using EcmaCreateobjectwithexcludedkeys = EcmaDisabled;
using EcmaThrowpatternnoncoercible = EcmaDisabled;
using EcmaCloseiterator = EcmaDisabled;
using EcmaImportmodule = EcmaDisabled;
using EcmaSetclasscomputedfields = EcmaDisabled;
using EcmaDefineclasswithbuffer = EcmaDisabled;
using EcmaLoadclasscomputedinstancefields = EcmaDisabled;
using EcmaDefineclassprivatefields = EcmaDisabled;
using EcmaClassfieldadd = EcmaDisabled;
using EcmaClassprivatefieldadd = EcmaDisabled;
using EcmaClassprivatemethodoraccessoradd = EcmaDisabled;
using EcmaClassprivatefieldget = EcmaDisabled;
using EcmaClassprivatefieldset = EcmaDisabled;
using EcmaClassprivatefieldin = EcmaDisabled;
using EcmaLdmodvarbyname = EcmaDisabled;
using EcmaStmodulevar = EcmaDisabled;
using EcmaCopymodule = EcmaDisabled;
using EcmaStsuperbyname = EcmaDisabled;
using EcmaLdsuperbyname = EcmaDisabled;
using EcmaStsuperbyvalue = EcmaDisabled;
using EcmaLdsuperbyvalue = EcmaDisabled;
using EcmaLdlexvardyn = EcmaDisabled;
using EcmaLdlexdyn = EcmaDisabled;
using EcmaStlexvardyn = EcmaDisabled;
using EcmaStlexdyn = EcmaDisabled;
using EcmaThrowifsupernotcorrectcall = EcmaDisabled;
using EcmaThrowtdz = EcmaDisabled;
using EcmaThrowconstassignment = EcmaDisabled;
using EcmaPoplexenvdyn = EcmaDisabled;
using EcmaCopylexenvdyn = EcmaDisabled;
using EcmaNewlexenvdyn = EcmaDisabled;
using EcmaLdlexenvdyn = EcmaDisabled;
using EcmaLdevalvar = EcmaDisabled;
using EcmaStevalvar = EcmaDisabled;
using EcmaLdevalbindings = EcmaDisabled;
using EcmaDirecteval = EcmaDisabled;
#endif

PandaGen::PandaGen(ArenaAllocator *const allocator, RegSpiller *const spiller, CompilerContext *const context,
                   varbinder::FunctionScope *const scope, ProgramElement *const program_element,
                   AstCompiler *astcompiler)
    : CodeGen(allocator, spiller, context, scope, program_element, astcompiler)
{
    Function::Compile(this);
}

FunctionBuilder *PandaGen::FuncBuilder() const noexcept
{
    return builder_;
}

EnvScope *PandaGen::GetEnvScope() const noexcept
{
    return env_scope_;
}

void PandaGen::OptionalChainCheck(const bool optional, const VReg obj) const
{
    if (optional && optional_chain_ != nullptr) {
        optional_chain_->Check(obj);
    }
}

void PandaGen::FunctionInit(CatchTable *catch_table)
{
    if (RootNode()->IsProgram()) {
        builder_ = Allocator()->New<FunctionBuilder>(this, catch_table);
        return;
    }

    const ir::ScriptFunction *func = RootNode()->AsScriptFunction();

    if (func->IsAsyncFunc()) {
        if (func->IsGenerator()) {
            builder_ = Allocator()->New<AsyncGeneratorFunctionBuilder>(this, catch_table);
            return;
        }

        builder_ = Allocator()->New<AsyncFunctionBuilder>(this, catch_table);
        return;
    }

    if (func->IsGenerator()) {
        builder_ = Allocator()->New<GeneratorFunctionBuilder>(this, catch_table);
        return;
    }

    builder_ = Allocator()->New<FunctionBuilder>(this, catch_table);
}

bool PandaGen::FunctionHasFinalizer() const
{
    if (RootNode()->IsProgram()) {
        return false;
    }

    const ir::ScriptFunction *func = RootNode()->AsScriptFunction();

    return func->IsAsyncFunc() || func->IsGenerator();
}

void PandaGen::FunctionEnter()
{
    builder_->Prepare(RootNode()->AsScriptFunction());
}

void PandaGen::FunctionExit()
{
    builder_->CleanUp(RootNode()->AsScriptFunction());
}

void PandaGen::StoreAccumulator(const ir::AstNode *node, VReg vreg)
{
    Ra().Emit<StaDyn>(node, vreg);
}

void PandaGen::LoadAccumulator(const ir::AstNode *node, VReg reg)
{
    Ra().Emit<LdaDyn>(node, reg);
}

IRNode *PandaGen::AllocMov(const ir::AstNode *node, const VReg vd, const VReg vs)
{
    return Allocator()->New<MovDyn>(node, vd, vs);
}

IRNode *PandaGen::AllocMov(const ir::AstNode *node, OutVReg vd, const VReg vs)
{
    ASSERT(vd.type == OperandType::ANY);
    return Allocator()->New<MovDyn>(node, *vd.reg, vs);
}

void PandaGen::MoveVreg(const ir::AstNode *node, VReg vd, VReg vs)
{
    Ra().Emit<MovDyn>(node, vd, vs);
}

void PandaGen::LoadAccumulatorDouble(const ir::AstNode *node, double num)
{
    Sa().Emit<FldaiDyn>(node, num);
}

void PandaGen::LoadAccumulatorInt(const ir::AstNode *node, size_t num)
{
    Sa().Emit<LdaiDyn>(node, static_cast<int64_t>(num));
}

void PandaGen::StoreConst(const ir::AstNode *node, VReg reg, Constant id)
{
    LoadConst(node, id);
    StoreAccumulator(node, reg);
}

void PandaGen::LoadConst(const ir::AstNode *node, Constant id)
{
    switch (id) {
        case Constant::JS_HOLE: {
            Sa().Emit<EcmaLdhole>(node);
            break;
        }
        case Constant::JS_NAN: {
            Sa().Emit<EcmaLdnan>(node);
            break;
        }
        case Constant::JS_INFINITY: {
            Sa().Emit<EcmaLdinfinity>(node);
            break;
        }
        case Constant::JS_GLOBAL: {
            Sa().Emit<EcmaLdglobal>(node);
            break;
        }
        case Constant::JS_UNDEFINED: {
            Sa().Emit<EcmaLdundefined>(node);
            break;
        }
        case Constant::JS_SYMBOL: {
            Sa().Emit<EcmaLdsymbol>(node);
            break;
        }
        case Constant::JS_NULL: {
            Sa().Emit<EcmaLdnull>(node);
            break;
        }
        case Constant::JS_TRUE: {
            Sa().Emit<EcmaLdtrue>(node);
            break;
        }
        case Constant::JS_FALSE: {
            Sa().Emit<EcmaLdfalse>(node);
            break;
        }
        default: {
            UNREACHABLE();
        }
    }
}

void PandaGen::GetFunctionObject(const ir::AstNode *node)
{
    LoadAccFromLexEnv(node, Scope()->Find(varbinder::VarBinder::MANDATORY_PARAM_FUNC));
}

void PandaGen::GetNewTarget(const ir::AstNode *node)
{
    LoadAccFromLexEnv(node, Scope()->Find(varbinder::VarBinder::MANDATORY_PARAM_NEW_TARGET));
}

void PandaGen::GetThis(const ir::AstNode *node)
{
    LoadAccFromLexEnv(node, Scope()->Find(varbinder::VarBinder::MANDATORY_PARAM_THIS));
}

void PandaGen::SetThis(const ir::AstNode *node)
{
    StoreAccToLexEnv(node, Scope()->Find(varbinder::VarBinder::MANDATORY_PARAM_THIS), true);
}

void PandaGen::LoadVar(const ir::Identifier *node, const varbinder::ConstScopeFindResult &result)
{
    auto *var = result.variable;

    if (var == nullptr) {
        TryLoadGlobalByName(node, result.name);
        return;
    }

    if (var->IsGlobalVariable()) {
        LoadGlobalVar(node, var->Name());
        return;
    }

    if (var->IsModuleVariable()) {
        LoadModuleVariable(node, var->AsModuleVariable()->ModuleReg(), var->AsModuleVariable()->ExoticName());
        return;
    }

    ASSERT(var->IsLocalVariable());
    LoadAccFromLexEnv(node, result);
}

void PandaGen::StoreVar(const ir::AstNode *node, const varbinder::ConstScopeFindResult &result, bool is_declaration)
{
    varbinder::Variable *var = result.variable;

    if (var == nullptr) {
        if (IsDirectEval()) {
            StoreEvalVariable(node, result.name);
        } else {
            TryStoreGlobalByName(node, result.name);
        }
        return;
    }

    if (var->IsGlobalVariable()) {
        StoreGlobalVar(node, var->Name());
        return;
    }

    if (var->IsModuleVariable()) {
        ThrowConstAssignment(node, var->Name());
        return;
    }

    ASSERT(var->IsLocalVariable());
    StoreAccToLexEnv(node, result, is_declaration);
}

void PandaGen::LoadAccFromArgs(const ir::AstNode *node)
{
    if (!Scope()->HasFlag(varbinder::ScopeFlags::USE_ARGS)) {
        return;
    }

    auto res = Scope()->Find(varbinder::VarBinder::FUNCTION_ARGUMENTS);
    ASSERT(res.scope);

    GetUnmappedArgs(node);
    StoreAccToLexEnv(node, res, true);
}

void PandaGen::LoadObjProperty(const ir::AstNode *node, const Operand &prop)
{
    if (std::holds_alternative<VReg>(prop)) {
        LoadObjByValue(node, std::get<VReg>(prop));
        return;
    }

    if (std::holds_alternative<int64_t>(prop)) {
        LoadObjByIndex(node, std::get<int64_t>(prop));
        return;
    }

    ASSERT(std::holds_alternative<util::StringView>(prop));
    LoadObjByName(node, std::get<util::StringView>(prop));
}

void PandaGen::StoreObjProperty(const ir::AstNode *node, VReg obj, const Operand &prop)
{
    if (std::holds_alternative<VReg>(prop)) {
        StoreObjByValue(node, obj, std::get<VReg>(prop));
        return;
    }

    if (std::holds_alternative<int64_t>(prop)) {
        StoreObjByIndex(node, obj, std::get<int64_t>(prop));
        return;
    }

    ASSERT(std::holds_alternative<util::StringView>(prop));
    StoreObjByName(node, obj, std::get<util::StringView>(prop));
}

void PandaGen::StoreOwnProperty(const ir::AstNode *node, VReg obj, const Operand &prop)
{
    if (std::holds_alternative<VReg>(prop)) {
        StOwnByValue(node, obj, std::get<VReg>(prop));
        return;
    }

    if (std::holds_alternative<int64_t>(prop)) {
        StOwnByIndex(node, obj, std::get<int64_t>(prop));
        return;
    }

    ASSERT(std::holds_alternative<util::StringView>(prop));
    StOwnByName(node, obj, std::get<util::StringView>(prop));
}

void PandaGen::TryLoadGlobalByName(const ir::AstNode *node, const util::StringView &name)
{
    Sa().Emit<EcmaTryldglobalbyname>(node, name);
}

void PandaGen::TryStoreGlobalByName(const ir::AstNode *node, const util::StringView &name)
{
    Sa().Emit<EcmaTrystglobalbyname>(node, name);
}

void PandaGen::LoadObjByName(const ir::AstNode *node, const util::StringView &prop)
{
    Ra().Emit<EcmaLdobjbyname>(node, prop);
}

void PandaGen::StoreObjByName(const ir::AstNode *node, VReg obj, const util::StringView &prop)
{
    Ra().Emit<EcmaStobjbyname>(node, prop, obj);
}

void PandaGen::LoadObjByIndex(const ir::AstNode *node, int64_t index)
{
    Ra().Emit<EcmaLdobjbyindex>(node, index);
}

void PandaGen::LoadObjByValue(const ir::AstNode *node, VReg obj)
{
    Ra().Emit<EcmaLdobjbyvalue>(node, obj);
}

void PandaGen::StoreObjByValue(const ir::AstNode *node, VReg obj, VReg prop)
{
    Ra().Emit<EcmaStobjbyvalue>(node, obj, prop);
}

void PandaGen::StoreObjByIndex(const ir::AstNode *node, VReg obj, int64_t index)
{
    Ra().Emit<EcmaStobjbyindex>(node, index, obj);
}

void PandaGen::StOwnByName(const ir::AstNode *node, VReg obj, const util::StringView &prop)
{
    Ra().Emit<EcmaStownbyname>(node, prop, obj);
}

void PandaGen::StOwnByValue(const ir::AstNode *node, VReg obj, VReg prop)
{
    Ra().Emit<EcmaStownbyvalue>(node, obj, prop);
}

void PandaGen::StOwnByIndex(const ir::AstNode *node, VReg obj, int64_t index)
{
    Ra().Emit<EcmaStownbyindex>(node, index, obj);
}

void PandaGen::DeleteObjProperty(const ir::AstNode *node, VReg obj, VReg prop)
{
    Ra().Emit<EcmaDelobjprop>(node, obj, prop);
}

void PandaGen::LoadGlobalVar(const ir::AstNode *node, const util::StringView &name)
{
    Sa().Emit<EcmaLdglobalvar>(node, name);
}

void PandaGen::StoreGlobalVar(const ir::AstNode *node, const util::StringView &name)
{
    Sa().Emit<EcmaStglobalvar>(node, name);
}

VReg PandaGen::LexEnv() const noexcept
{
    return env_scope_->LexEnv();
}

void PandaGen::LoadAccFromLexEnv(const ir::AstNode *node, const varbinder::ConstScopeFindResult &result)
{
    VirtualLoadVar::Expand(this, node, result);
}

void PandaGen::StoreAccToLexEnv(const ir::AstNode *node, const varbinder::ConstScopeFindResult &result,
                                bool is_declaration)
{
    VirtualStoreVar::Expand(this, node, result, is_declaration);
}

void PandaGen::LoadAccumulatorBigInt(const ir::AstNode *node, const util::StringView &big_int)
{
    Sa().Emit<EcmaLdbigint>(node, big_int);
}

void PandaGen::Condition(const ir::AstNode *node, lexer::TokenType op, VReg lhs, Label *if_false)
{
    switch (op) {
        case lexer::TokenType::PUNCTUATOR_EQUAL: {
            Ra().Emit<EcmaEqdyn>(node, lhs);
            break;
        }
        case lexer::TokenType::PUNCTUATOR_NOT_EQUAL: {
            Ra().Emit<EcmaNoteqdyn>(node, lhs);
            break;
        }
        case lexer::TokenType::PUNCTUATOR_STRICT_EQUAL: {
            Ra().Emit<EcmaStricteqdyn>(node, lhs);
            break;
        }
        case lexer::TokenType::PUNCTUATOR_NOT_STRICT_EQUAL: {
            Ra().Emit<EcmaStrictnoteqdyn>(node, lhs);
            break;
        }
        case lexer::TokenType::PUNCTUATOR_LESS_THAN: {
            Ra().Emit<EcmaLessdyn>(node, lhs);
            break;
        }
        case lexer::TokenType::PUNCTUATOR_LESS_THAN_EQUAL: {
            Ra().Emit<EcmaLesseqdyn>(node, lhs);
            break;
        }
        case lexer::TokenType::PUNCTUATOR_GREATER_THAN: {
            Ra().Emit<EcmaGreaterdyn>(node, lhs);
            break;
        }
        case lexer::TokenType::PUNCTUATOR_GREATER_THAN_EQUAL: {
            Ra().Emit<EcmaGreatereqdyn>(node, lhs);
            break;
        }
        default: {
            UNREACHABLE();
        }
    }

    BranchIfFalse(node, if_false);
}

void PandaGen::Unary(const ir::AstNode *node, lexer::TokenType op, VReg operand)
{
    switch (op) {
        case lexer::TokenType::PUNCTUATOR_PLUS: {
            Ra().Emit<EcmaTonumber>(node, operand);
            break;
        }
        case lexer::TokenType::PUNCTUATOR_MINUS: {
            Ra().Emit<EcmaNegdyn>(node, operand);
            break;
        }
        case lexer::TokenType::PUNCTUATOR_TILDE: {
            Ra().Emit<EcmaNotdyn>(node, operand);
            break;
        }
        case lexer::TokenType::PUNCTUATOR_EXCLAMATION_MARK: {
            Sa().Emit<EcmaNegate>(node);
            break;
        }
        case lexer::TokenType::PUNCTUATOR_PLUS_PLUS: {
            Ra().Emit<EcmaIncdyn>(node, operand);
            break;
        }
        case lexer::TokenType::PUNCTUATOR_MINUS_MINUS: {
            Ra().Emit<EcmaDecdyn>(node, operand);
            break;
        }
        case lexer::TokenType::KEYW_VOID:
        case lexer::TokenType::KEYW_DELETE: {
            LoadConst(node, Constant::JS_UNDEFINED);
            break;
        }
        default: {
            UNREACHABLE();
        }
    }
}

void PandaGen::Binary(const ir::AstNode *node, lexer::TokenType op, VReg lhs)
{
    switch (op) {
        case lexer::TokenType::PUNCTUATOR_EQUAL: {
            Ra().Emit<EcmaEqdyn>(node, lhs);
            break;
        }
        case lexer::TokenType::PUNCTUATOR_NOT_EQUAL: {
            Ra().Emit<EcmaNoteqdyn>(node, lhs);
            break;
        }
        case lexer::TokenType::PUNCTUATOR_STRICT_EQUAL: {
            Ra().Emit<EcmaStricteqdyn>(node, lhs);
            break;
        }
        case lexer::TokenType::PUNCTUATOR_NOT_STRICT_EQUAL: {
            Ra().Emit<EcmaStrictnoteqdyn>(node, lhs);
            break;
        }
        case lexer::TokenType::PUNCTUATOR_LESS_THAN: {
            Ra().Emit<EcmaLessdyn>(node, lhs);
            break;
        }
        case lexer::TokenType::PUNCTUATOR_LESS_THAN_EQUAL: {
            Ra().Emit<EcmaLesseqdyn>(node, lhs);
            break;
        }
        case lexer::TokenType::PUNCTUATOR_GREATER_THAN: {
            Ra().Emit<EcmaGreaterdyn>(node, lhs);
            break;
        }
        case lexer::TokenType::PUNCTUATOR_GREATER_THAN_EQUAL: {
            Ra().Emit<EcmaGreatereqdyn>(node, lhs);
            break;
        }
        case lexer::TokenType::PUNCTUATOR_PLUS:
        case lexer::TokenType::PUNCTUATOR_PLUS_EQUAL: {
            Ra().Emit<EcmaAdd2dyn>(node, lhs);
            break;
        }
        case lexer::TokenType::PUNCTUATOR_MINUS:
        case lexer::TokenType::PUNCTUATOR_MINUS_EQUAL: {
            Ra().Emit<EcmaSub2dyn>(node, lhs);
            break;
        }
        case lexer::TokenType::PUNCTUATOR_MULTIPLY:
        case lexer::TokenType::PUNCTUATOR_MULTIPLY_EQUAL: {
            Ra().Emit<EcmaMul2dyn>(node, lhs);
            break;
        }
        case lexer::TokenType::PUNCTUATOR_DIVIDE:
        case lexer::TokenType::PUNCTUATOR_DIVIDE_EQUAL: {
            Ra().Emit<EcmaDiv2dyn>(node, lhs);
            break;
        }
        case lexer::TokenType::PUNCTUATOR_MOD:
        case lexer::TokenType::PUNCTUATOR_MOD_EQUAL: {
            Ra().Emit<EcmaMod2dyn>(node, lhs);
            break;
        }
        case lexer::TokenType::PUNCTUATOR_EXPONENTIATION_EQUAL:
        case lexer::TokenType::PUNCTUATOR_EXPONENTIATION: {
            Ra().Emit<EcmaExpdyn>(node, lhs);
            break;
        }
        case lexer::TokenType::PUNCTUATOR_LEFT_SHIFT:
        case lexer::TokenType::PUNCTUATOR_LEFT_SHIFT_EQUAL: {
            Ra().Emit<EcmaShl2dyn>(node, lhs);
            break;
        }
        case lexer::TokenType::PUNCTUATOR_RIGHT_SHIFT:
        case lexer::TokenType::PUNCTUATOR_RIGHT_SHIFT_EQUAL: {
            Ra().Emit<EcmaShr2dyn>(node, lhs);
            break;
        }
        case lexer::TokenType::PUNCTUATOR_UNSIGNED_RIGHT_SHIFT:
        case lexer::TokenType::PUNCTUATOR_UNSIGNED_RIGHT_SHIFT_EQUAL: {
            Ra().Emit<EcmaAshr2dyn>(node, lhs);
            break;
        }
        case lexer::TokenType::PUNCTUATOR_BITWISE_AND:
        case lexer::TokenType::PUNCTUATOR_BITWISE_AND_EQUAL: {
            Ra().Emit<EcmaAnd2dyn>(node, lhs);
            break;
        }
        case lexer::TokenType::PUNCTUATOR_BITWISE_OR:
        case lexer::TokenType::PUNCTUATOR_BITWISE_OR_EQUAL: {
            Ra().Emit<EcmaOr2dyn>(node, lhs);
            break;
        }
        case lexer::TokenType::PUNCTUATOR_BITWISE_XOR:
        case lexer::TokenType::PUNCTUATOR_BITWISE_XOR_EQUAL: {
            Ra().Emit<EcmaXor2dyn>(node, lhs);
            break;
        }
        case lexer::TokenType::KEYW_IN: {
            Ra().Emit<EcmaIsindyn>(node, lhs);
            break;
        }
        case lexer::TokenType::KEYW_INSTANCEOF: {
            Ra().Emit<EcmaInstanceofdyn>(node, lhs);
            break;
        }
        case lexer::TokenType::PUNCTUATOR_NULLISH_COALESCING:
        case lexer::TokenType::PUNCTUATOR_LOGICAL_NULLISH_EQUAL: {
            Unimplemented();
            break;
        }
        default: {
            UNREACHABLE();
        }
    }
}

void PandaGen::BranchIfUndefined(const ir::AstNode *node, Label *target)
{
    Sa().Emit<EcmaIsundefined>(node);
    BranchIfTrue(node, target);
}

void PandaGen::BranchIfNotUndefined(const ir::AstNode *node, Label *target)
{
    Sa().Emit<EcmaIsundefined>(node);
    BranchIfFalse(node, target);
}

void PandaGen::BranchIfTrue(const ir::AstNode *node, Label *target)
{
    Sa().Emit<EcmaJtrue>(node, target);
}

void PandaGen::BranchIfNotTrue(const ir::AstNode *node, Label *target)
{
    Sa().Emit<EcmaIstrue>(node);
    BranchIfFalse(node, target);
}

void PandaGen::BranchIfFalse(const ir::AstNode *node, Label *target)
{
    Sa().Emit<EcmaJfalse>(node, target);
}

void PandaGen::BranchIfCoercible(const ir::AstNode *node, Label *target)
{
    Sa().Emit<EcmaIscoercible>(node);
    BranchIfTrue(node, target);
}

void PandaGen::EmitThrow(const ir::AstNode *node)
{
    Sa().Emit<EcmaThrowdyn>(node);
}

void PandaGen::EmitRethrow(const ir::AstNode *node)
{
    Sa().Emit<EcmaRethrowdyn>(node);
}

void PandaGen::EmitReturn(const ir::AstNode *node)
{
    Sa().Emit<EcmaReturnDyn>(node);
}

void PandaGen::EmitReturnUndefined(const ir::AstNode *node)
{
    Sa().Emit<EcmaReturnundefined>(node);
}

void PandaGen::ImplicitReturn(const ir::AstNode *node)
{
    builder_->ImplicitReturn(node);
}

void PandaGen::DirectReturn(const ir::AstNode *node)
{
    builder_->DirectReturn(node);
}

void PandaGen::ValidateClassDirectReturn(const ir::AstNode *node)
{
    const ir::ScriptFunction *func = util::Helpers::GetContainingFunction(node);

    if (func == nullptr || !func->IsConstructor()) {
        return;
    }

    RegScope rs(this);
    VReg value = AllocReg();
    StoreAccumulator(node, value);

    auto *not_undefined = AllocLabel();
    auto *cond_end = AllocLabel();

    BranchIfNotUndefined(node, not_undefined);
    GetThis(func);
    ThrowIfSuperNotCorrectCall(func, 0);
    Branch(node, cond_end);

    SetLabel(node, not_undefined);
    LoadAccumulator(node, value);

    SetLabel(node, cond_end);
}

void PandaGen::EmitAwait(const ir::AstNode *node)
{
    builder_->Await(node);
}

void PandaGen::Call0This(const ir::AstNode *node, VReg callee, VReg this_reg)
{
    LoadAccumulator(node, this_reg);
    Ra().Emit<EcmaCall0thisdyn>(node, callee);
}

void PandaGen::Call1This(const ir::AstNode *node, VReg callee, VReg this_reg, VReg arg0)
{
    LoadAccumulator(node, arg0);
    Ra().Emit<EcmaCall1thisdyn>(node, callee, this_reg);
}

void PandaGen::Call(const ir::AstNode *node, VReg callee, VReg this_reg, const ArenaVector<ir::Expression *> &arguments)
{
    bool has_this = !this_reg.IsInvalid();

    switch (arguments.size()) {
        case 0: {  // 0 args
            if (has_this) {
                Call0This(node, callee, this_reg);
            } else {
                Sa().Emit<EcmaCall0dyn>(node);
            }
            return;
        }
        case 1: {  // 1 arg
            const auto *arg0 = arguments[0];
            arg0->Compile(this);

            if (has_this) {
                Ra().Emit<EcmaCall1thisdyn>(node, callee, this_reg);
            } else {
                Ra().Emit<EcmaCall1dyn>(node, callee);
            }
            return;
        }
        case 2: {  // 2 args
            const auto *arg0 = arguments[0];
            arg0->Compile(this);
            compiler::VReg arg0_reg = AllocReg();
            StoreAccumulator(arg0, arg0_reg);

            const auto *arg1 = arguments[1];
            arg1->Compile(this);

            if (has_this) {
                Ra().Emit<EcmaCall2thisdyn>(node, callee, this_reg, arg0_reg);
            } else {
                Ra().Emit<EcmaCall2dyn>(node, callee, arg0_reg);
            }
            return;
        }
        case 3: {  // 3 args
            const auto *arg0 = arguments[0];
            arg0->Compile(this);
            compiler::VReg arg0_reg = AllocReg();
            StoreAccumulator(arg0, arg0_reg);

            const auto *arg1 = arguments[1];
            arg1->Compile(this);
            compiler::VReg arg1_reg = AllocReg();
            StoreAccumulator(arg1, arg1_reg);

            const auto *arg2 = arguments[2];
            arg2->Compile(this);

            if (has_this) {
                Ra().Emit<EcmaCall3thisdyn>(node, callee, this_reg, arg0_reg, arg1_reg);
            } else {
                Ra().Emit<EcmaCall3dyn>(node, callee, arg0_reg, arg1_reg);
            }
            return;
        }
        default: {
            break;
        }
    }

    for (const auto *it : arguments) {
        it->Compile(this);
        compiler::VReg arg = AllocReg();
        StoreAccumulator(it, arg);
    }

    if (has_this) {
        size_t arg_count = arguments.size() + 1;
        auto constexpr EXTRA_ARGS = 2;
        Rra().Emit<EcmaCallithisrangedyn>(node, callee, arg_count + EXTRA_ARGS, static_cast<int64_t>(arg_count),
                                          callee);
    } else {
        size_t arg_count = arguments.size();
        Rra().Emit<EcmaCallirangedyn>(node, callee, arg_count + 1, static_cast<int64_t>(arg_count), callee);
    }
}

void PandaGen::CallTagged(const ir::AstNode *node, VReg callee, VReg this_reg,
                          const ArenaVector<ir::Expression *> &arguments)
{
    bool has_this = !this_reg.IsInvalid();

    StoreAccumulator(node, callee);
    Literals::GetTemplateObject(this, node->AsTaggedTemplateExpression());

    if (arguments.empty()) {
        if (has_this) {
            Ra().Emit<EcmaCall1thisdyn>(node, callee, this_reg);
        } else {
            Sa().Emit<EcmaCall1dyn>(node, callee);
        }
        return;
    }

    VReg arg0_reg = AllocReg();
    StoreAccumulator(node, arg0_reg);

    switch (arguments.size()) {
        case 1: {
            const auto *arg = arguments[0];
            arg->Compile(this);

            if (has_this) {
                Ra().Emit<EcmaCall2thisdyn>(node, callee, this_reg, arg0_reg);
            } else {
                Ra().Emit<EcmaCall2dyn>(node, callee, arg0_reg);
            }
            return;
        }
        case 2: {  // 2:2 args
            const auto *arg1 = arguments[0];
            arg1->Compile(this);
            compiler::VReg arg1_reg = AllocReg();
            StoreAccumulator(arg1, arg1_reg);

            const auto *arg2 = arguments[1];
            arg2->Compile(this);

            if (has_this) {
                Ra().Emit<EcmaCall3thisdyn>(node, callee, this_reg, arg0_reg, arg1_reg);
            } else {
                Ra().Emit<EcmaCall3dyn>(node, callee, arg0_reg, arg1_reg);
            }
            return;
        }
        default: {
            break;
        }
    }

    for (const auto *it : arguments) {
        it->Compile(this);
        compiler::VReg arg = AllocReg();
        StoreAccumulator(it, arg);
    }

    if (has_this) {
        auto constexpr EXTRA_ARGS = 2;
        size_t arg_count = arguments.size() + EXTRA_ARGS;
        Rra().Emit<EcmaCallithisrangedyn>(node, callee, arg_count + EXTRA_ARGS, static_cast<int64_t>(arg_count),
                                          callee);
    } else {
        size_t arg_count = arguments.size() + 1;
        Rra().Emit<EcmaCallirangedyn>(node, callee, arg_count + 1, static_cast<int64_t>(arg_count), callee);
    }
}

void PandaGen::SuperCall(const ir::AstNode *node, VReg start_reg, size_t arg_count)
{
    Rra().Emit<EcmaSupercall>(node, start_reg, arg_count, static_cast<int64_t>(arg_count), start_reg);
}

void PandaGen::SuperCallSpread(const ir::AstNode *node, VReg vs)
{
    Ra().Emit<EcmaSupercallspread>(node, vs);
}

void PandaGen::NewObject(const ir::AstNode *node, VReg start_reg, size_t arg_count)
{
    Rra().Emit<EcmaNewobjdynrange>(node, start_reg, arg_count, static_cast<int64_t>(arg_count), start_reg);
}

void PandaGen::LoadHomeObject(const ir::AstNode *node)
{
    Sa().Emit<EcmaLdhomeobject>(node);
}

void PandaGen::DefineMethod(const ir::AstNode *node, const util::StringView &name)
{
    Ra().Emit<EcmaDefinemethod>(node, name, LexEnv());
}

void PandaGen::DefineFunction(const ir::AstNode *node, const ir::ScriptFunction *real_node,
                              const util::StringView &name)
{
    if (real_node->IsAsyncFunc()) {
        if (real_node->IsGenerator()) {
            Ra().Emit<EcmaDefineasyncgeneratorfunc>(node, name, LexEnv());
        } else {
            Ra().Emit<EcmaDefineasyncfunc>(node, name, LexEnv());
        }
    } else if (real_node->IsGenerator()) {
        Ra().Emit<EcmaDefinegeneratorfunc>(node, name, LexEnv());
    } else if (real_node->IsArrow()) {
        LoadHomeObject(node);
        Ra().Emit<EcmaDefinencfuncdyn>(node, name, LexEnv());
    } else if (real_node->IsMethod()) {
        DefineMethod(node, name);
    } else {
        Ra().Emit<EcmaDefinefuncdyn>(node, name, LexEnv());
    }
}

void PandaGen::TypeOf(const ir::AstNode *node)
{
    Sa().Emit<EcmaTypeofdyn>(node);
}

void PandaGen::CallSpread(const ir::AstNode *node, VReg func, VReg this_reg, VReg args)
{
    Ra().Emit<EcmaCallspreaddyn>(node, func, this_reg, args);
}

void PandaGen::NewObjSpread(const ir::AstNode *node, VReg obj, VReg target)
{
    Ra().Emit<EcmaNewobjspreaddyn>(node, obj, target);
}

void PandaGen::GetUnmappedArgs(const ir::AstNode *node)
{
    Sa().Emit<EcmaGetunmappedargs>(node);
}

void PandaGen::Negate(const ir::AstNode *node)
{
    Sa().Emit<EcmaNegate>(node);
}

void PandaGen::ToBoolean(const ir::AstNode *node)
{
    Sa().Emit<EcmaToboolean>(node);
}

void PandaGen::ToNumber(const ir::AstNode *node, VReg arg)
{
    Ra().Emit<EcmaTonumber>(node, arg);
}

void PandaGen::GetMethod(const ir::AstNode *node, VReg obj, const util::StringView &name)
{
    Ra().Emit<EcmaGetmethod>(node, name, obj);
}

void PandaGen::CreateGeneratorObj(const ir::AstNode *node, VReg func_obj)
{
    Ra().Emit<EcmaCreategeneratorobj>(node, func_obj);
}

void PandaGen::CreateAsyncGeneratorObj(const ir::AstNode *node, VReg func_obj)
{
    Ra().Emit<EcmaCreateasyncgeneratorobj>(node, func_obj);
}

void PandaGen::CreateIterResultObject(const ir::AstNode *node, bool done)
{
    Ra().Emit<EcmaCreateiterresultobj>(node, static_cast<int32_t>(done));
}

void PandaGen::SuspendGenerator(const ir::AstNode *node, VReg gen_obj)
{
    Ra().Emit<EcmaSuspendgenerator>(node, gen_obj);
}

void PandaGen::SuspendAsyncGenerator(const ir::AstNode *node, VReg async_gen_obj)
{
    Ra().Emit<EcmaSuspendasyncgenerator>(node, async_gen_obj);
}

void PandaGen::GeneratorYield(const ir::AstNode *node, VReg gen_obj)
{
    Ra().Emit<EcmaSetgeneratorstate>(node, gen_obj, static_cast<int32_t>(GeneratorState::SUSPENDED_YIELD));
}

void PandaGen::GeneratorComplete(const ir::AstNode *node, VReg gen_obj)
{
    Ra().Emit<EcmaSetgeneratorstate>(node, gen_obj, static_cast<int32_t>(GeneratorState::COMPLETED));
}

void PandaGen::ResumeGenerator(const ir::AstNode *node, VReg gen_obj)
{
    Ra().Emit<EcmaResumegenerator>(node, gen_obj);
}

void PandaGen::GetResumeMode(const ir::AstNode *node, VReg gen_obj)
{
    Ra().Emit<EcmaGetresumemode>(node, gen_obj);
}

void PandaGen::AsyncFunctionEnter(const ir::AstNode *node)
{
    Sa().Emit<EcmaAsyncfunctionenter>(node);
}

void PandaGen::AsyncFunctionAwait(const ir::AstNode *node, VReg async_func_obj)
{
    Ra().Emit<EcmaAsyncfunctionawait>(node, async_func_obj);
}

void PandaGen::AsyncFunctionResolve(const ir::AstNode *node, VReg async_func_obj)
{
    Ra().Emit<EcmaAsyncfunctionresolve>(node, async_func_obj);
}

void PandaGen::AsyncFunctionReject(const ir::AstNode *node, VReg async_func_obj)
{
    Ra().Emit<EcmaAsyncfunctionreject>(node, async_func_obj);
}

void PandaGen::AsyncGeneratorResolve(const ir::AstNode *node, VReg async_gen_obj)
{
    Ra().Emit<EcmaAsyncgeneratorresolve>(node, async_gen_obj);
}

void PandaGen::AsyncGeneratorReject(const ir::AstNode *node, VReg async_gen_obj)
{
    Ra().Emit<EcmaAsyncgeneratorreject>(node, async_gen_obj);
}

void PandaGen::GetTemplateObject(const ir::AstNode *node, VReg value)
{
    Ra().Emit<EcmaGettemplateobject>(node, value);
}

void PandaGen::CopyRestArgs(const ir::AstNode *node, uint32_t index)
{
    Sa().Emit<EcmaCopyrestargs>(node, index);
}

void PandaGen::GetPropIterator(const ir::AstNode *node)
{
    Sa().Emit<EcmaGetpropiterator>(node);
}

void PandaGen::GetNextPropName(const ir::AstNode *node, VReg iter)
{
    Ra().Emit<EcmaGetnextpropname>(node, iter);
}

void PandaGen::CreateEmptyObject(const ir::AstNode *node)
{
    Sa().Emit<EcmaCreateemptyobject>(node);
}

void PandaGen::CreateObjectWithBuffer(const ir::AstNode *node, uint32_t idx)
{
    ASSERT(util::Helpers::IsInteger<uint32_t>(idx));
    Sa().Emit<EcmaCreateobjectwithbuffer>(node, util::Helpers::ToStringView(Allocator(), idx));
}

void PandaGen::CreateObjectHavingMethod(const ir::AstNode *node, uint32_t idx)
{
    ASSERT(util::Helpers::IsInteger<uint32_t>(idx));
    LoadAccumulator(node, LexEnv());
    Sa().Emit<EcmaCreateobjecthavingmethod>(node, util::Helpers::ToStringView(Allocator(), idx));
}

void PandaGen::SetObjectWithProto(const ir::AstNode *node, VReg proto, VReg obj)
{
    Ra().Emit<EcmaSetobjectwithproto>(node, proto, obj);
}

void PandaGen::CopyDataProperties(const ir::AstNode *node, VReg dst, VReg src)
{
    Ra().Emit<EcmaCopydataproperties>(node, dst, src);
}

void PandaGen::DefineGetterSetterByValue(const ir::AstNode *node, VReg obj, VReg name, VReg getter, VReg setter,
                                         bool set_name)
{
    LoadConst(node, set_name ? Constant::JS_TRUE : Constant::JS_FALSE);
    Ra().Emit<EcmaDefinegettersetterbyvalue>(node, obj, name, getter, setter);
}

void PandaGen::CreateEmptyArray(const ir::AstNode *node)
{
    Sa().Emit<EcmaCreateemptyarray>(node);
}

void PandaGen::CreateArrayWithBuffer(const ir::AstNode *node, uint32_t idx)
{
    ASSERT(util::Helpers::IsInteger<uint32_t>(idx));
    Sa().Emit<EcmaCreatearraywithbuffer>(node, util::Helpers::ToStringView(Allocator(), idx));
}

void PandaGen::CreateArray(const ir::AstNode *node, const ArenaVector<ir::Expression *> &elements, VReg obj)
{
    if (elements.empty()) {
        CreateEmptyArray(node);
        StoreAccumulator(node, obj);
        return;
    }

    LiteralBuffer buf {};

    size_t i = 0;
    // This loop handles constant literal data by collecting it into a literal buffer
    // until a non-constant element is encountered.
    while (i < elements.size()) {
        Literal lit = util::Helpers::ToConstantLiteral(elements[i]);
        if (lit.IsInvalid()) {
            break;
        }

        buf.emplace_back(std::move(lit));
        i++;
    }

    if (buf.empty()) {
        CreateEmptyArray(node);
    } else {
        uint32_t buf_idx = AddLiteralBuffer(std::move(buf));
        CreateArrayWithBuffer(node, buf_idx);
    }

    StoreAccumulator(node, obj);

    if (i == elements.size()) {
        return;
    }

    bool has_spread = false;

    // This loop handles array elements until a spread element is encountered
    for (; i < elements.size(); i++) {
        const ir::Expression *elem = elements[i];

        if (elem->IsOmittedExpression()) {
            continue;
        }

        if (elem->IsSpreadElement()) {
            // The next loop will handle arrays that have a spread element
            has_spread = true;
            break;
        }

        elem->Compile(this);
        StOwnByIndex(elem, obj, i);
    }

    RegScope rs(this);
    VReg idx_reg {};

    if (has_spread) {
        idx_reg = AllocReg();
        LoadAccumulatorInt(node, i);
        StoreAccumulator(node, idx_reg);
    }

    // This loop handles arrays that contain spread elements
    for (; i < elements.size(); i++) {
        const ir::Expression *elem = elements[i];

        if (elem->IsSpreadElement()) {
            elem->AsSpreadElement()->Argument()->Compile(this);

            StoreArraySpread(elem, obj, idx_reg);
            StoreAccumulator(elem, idx_reg);
            continue;
        }

        if (!elem->IsOmittedExpression()) {
            elem->Compile(this);
            StOwnByValue(elem, obj, idx_reg);
        }

        Unary(elem, lexer::TokenType::PUNCTUATOR_PLUS_PLUS, idx_reg);
        StoreAccumulator(elem, idx_reg);
    }

    // If the last element is omitted, we also have to update the length property
    if (elements.back()->IsOmittedExpression()) {
        // if there was a spread value then acc already contains the length
        if (!has_spread) {
            LoadAccumulatorInt(node, i);
        }

        StOwnByName(node, obj, "length");
    }

    LoadAccumulator(node, obj);
}

void PandaGen::StoreArraySpread(const ir::AstNode *node, VReg array, VReg index)
{
    Ra().Emit<EcmaStarrayspread>(node, array, index);
}

void PandaGen::CreateRegExpWithLiteral(const ir::AstNode *node, const util::StringView &pattern, uint8_t flags)
{
    Sa().Emit<EcmaCreateregexpwithliteral>(node, pattern, flags);
}

void PandaGen::ThrowIfNotObject(const ir::AstNode *node)
{
    Ra().Emit<EcmaThrowifnotobject>(node);
}

void PandaGen::ThrowThrowNotExist(const ir::AstNode *node)
{
    Sa().Emit<EcmaThrowthrownotexists>(node);
}

void PandaGen::GetIterator(const ir::AstNode *node)
{
    Sa().Emit<EcmaGetiterator>(node);
}

void PandaGen::GetAsyncIterator(const ir::AstNode *node)
{
    Sa().Emit<EcmaGetasynciterator>(node);
}

void PandaGen::CreateObjectWithExcludedKeys(const ir::AstNode *node, VReg obj, VReg arg_start, size_t arg_count)
{
    ASSERT(arg_start.GetIndex() == obj.GetIndex() - 1);
    if (arg_count == 0) {  // Do not emit undefined register
        arg_start = obj;
    }

    Rra().Emit<EcmaCreateobjectwithexcludedkeys>(node, arg_start, arg_count, static_cast<int64_t>(arg_count), obj,
                                                 arg_start);
}

void PandaGen::ThrowObjectNonCoercible(const ir::AstNode *node)
{
    Sa().Emit<EcmaThrowpatternnoncoercible>(node);
}

void PandaGen::CloseIterator(const ir::AstNode *node, VReg iter)
{
    Ra().Emit<EcmaCloseiterator>(node, iter);
}

void PandaGen::ImportModule(const ir::AstNode *node, const util::StringView &name)
{
    Sa().Emit<EcmaImportmodule>(node, name);
}

void PandaGen::SetClassComputedFields(const ir::AstNode *node, VReg class_reg, VReg computed_instance_field_array)
{
    Ra().Emit<EcmaSetclasscomputedfields>(node, class_reg, computed_instance_field_array);
}

void PandaGen::DefineClassWithBuffer(const ir::AstNode *node, const util::StringView &ctor_id, int32_t lit_idx,
                                     VReg lexenv, VReg base)
{
    Ra().Emit<EcmaDefineclasswithbuffer>(node, ctor_id, lit_idx, lexenv, base);
}

void PandaGen::LoadClassComputedInstanceFields(const ir::AstNode *node, VReg ctor)
{
    Sa().Emit<EcmaLoadclasscomputedinstancefields>(node, ctor);
}

void PandaGen::DefineClassPrivateFields(const ir::AstNode *node, int32_t private_buf_idx)
{
    Sa().Emit<EcmaDefineclassprivatefields>(node, util::Helpers::ToStringView(Allocator(), private_buf_idx), LexEnv());
}

void PandaGen::ClassFieldAdd(const ir::AstNode *node, VReg obj, VReg prop)
{
    Ra().Emit<EcmaClassfieldadd>(node, obj, prop);
}

void PandaGen::ClassPrivateFieldAdd(const ir::AstNode *node, VReg ctor, VReg obj, const util::StringView &prop)
{
    Ra().Emit<EcmaClassprivatefieldadd>(node, prop, ctor, obj);
}

void PandaGen::ClassPrivateMethodOrAccessorAdd(const ir::AstNode *node, VReg ctor, VReg obj)
{
    Ra().Emit<EcmaClassprivatemethodoraccessoradd>(node, ctor, obj);
}

void PandaGen::ClassPrivateFieldGet(const ir::AstNode *node, VReg ctor, VReg obj, const util::StringView &prop)
{
    Ra().Emit<EcmaClassprivatefieldget>(node, prop, ctor, obj);
}

void PandaGen::ClassPrivateFieldSet(const ir::AstNode *node, VReg ctor, VReg obj, const util::StringView &prop)
{
    Ra().Emit<EcmaClassprivatefieldset>(node, prop, ctor, obj);
}

void PandaGen::ClassPrivateFieldIn(const ir::AstNode *node, VReg ctor, const util::StringView &prop)
{
    Ra().Emit<EcmaClassprivatefieldin>(node, prop, ctor);
}

void PandaGen::LoadModuleVariable(const ir::AstNode *node, VReg module, const util::StringView &name)
{
    Ra().Emit<EcmaLdmodvarbyname>(node, name, module);
}

void PandaGen::StoreModuleVar(const ir::AstNode *node, const util::StringView &name)
{
    Sa().Emit<EcmaStmodulevar>(node, name);
}

void PandaGen::CopyModule(const ir::AstNode *node, VReg module)
{
    Ra().Emit<EcmaCopymodule>(node, module);
}

void PandaGen::StSuperByName(const ir::AstNode *node, VReg obj, const util::StringView &key)
{
    Ra().Emit<EcmaStsuperbyname>(node, key, obj);
}

void PandaGen::LdSuperByName(const ir::AstNode *node, const util::StringView &key)
{
    Ra().Emit<EcmaLdsuperbyname>(node, key);
}

void PandaGen::StSuperByValue(const ir::AstNode *node, VReg obj, VReg prop)
{
    Ra().Emit<EcmaStsuperbyvalue>(node, obj, prop);
}

void PandaGen::LdSuperByValue(const ir::AstNode *node, VReg obj)
{
    Ra().Emit<EcmaLdsuperbyvalue>(node, obj);
}

void PandaGen::StoreSuperProperty(const ir::AstNode *node, VReg obj, const Operand &prop)
{
    if (std::holds_alternative<util::StringView>(prop)) {
        StSuperByName(node, obj, std::get<util::StringView>(prop));
        return;
    }

    ASSERT(std::holds_alternative<VReg>(prop));
    StSuperByValue(node, obj, std::get<VReg>(prop));
}

void PandaGen::LoadSuperProperty(const ir::AstNode *node, const Operand &prop)
{
    if (std::holds_alternative<util::StringView>(prop)) {
        LdSuperByName(node, std::get<util::StringView>(prop));
        return;
    }

    ASSERT(std::holds_alternative<VReg>(prop));
    LdSuperByValue(node, std::get<VReg>(prop));
}

void PandaGen::LoadLexicalVar(const ir::AstNode *node, uint32_t level, uint32_t slot)
{
    Sa().Emit<EcmaLdlexvardyn>(node, level, slot);
}

void PandaGen::LoadLexical(const ir::AstNode *node, const util::StringView &name, uint32_t level, uint32_t slot)
{
    Sa().Emit<EcmaLdlexdyn>(node, name, level, slot);
}

void PandaGen::StoreLexicalVar(const ir::AstNode *node, uint32_t level, uint32_t slot)
{
    Ra().Emit<EcmaStlexvardyn>(node, level, slot);
}

void PandaGen::StoreLexical(const ir::AstNode *node, const util::StringView &name, uint32_t level, uint32_t slot)
{
    Ra().Emit<EcmaStlexdyn>(node, name, level, slot);
}

void PandaGen::ThrowIfSuperNotCorrectCall(const ir::AstNode *node, int64_t num)
{
    Sa().Emit<EcmaThrowifsupernotcorrectcall>(node, num);
}

void PandaGen::ThrowTdz(const ir::AstNode *node, const util::StringView &name)
{
    Sa().Emit<EcmaThrowtdz>(node, name);
}

void PandaGen::ThrowConstAssignment(const ir::AstNode *node, const util::StringView &name)
{
    Ra().Emit<EcmaThrowconstassignment>(node, name);
}

void PandaGen::PopLexEnv(const ir::AstNode *node)
{
    Sa().Emit<EcmaPoplexenvdyn>(node);
}

void PandaGen::CopyLexEnv(const ir::AstNode *node)
{
    Sa().Emit<EcmaCopylexenvdyn>(node);
}

void PandaGen::NewLexEnv(const ir::AstNode *node, uint32_t num)
{
    Sa().Emit<EcmaNewlexenvdyn>(node, num);
}

void PandaGen::LdLexEnv(const ir::AstNode *node)
{
    Sa().Emit<EcmaLdlexenvdyn>(node);
}

Operand PandaGen::ToNamedPropertyKey(const ir::Expression *prop, bool is_computed)
{
    VReg res {VReg::REG_START};

    if (!is_computed) {
        if (prop->IsIdentifier()) {
            return prop->AsIdentifier()->Name();
        }
        return res;
    }

    if (prop->IsStringLiteral()) {
        const util::StringView &str = prop->AsStringLiteral()->Str();

        /* NOTE: dbatyai. remove this when runtime handles __proto__ as property name correctly */
        if (str.Is("__proto__")) {
            return res;
        }

        int64_t index = util::Helpers::GetIndex(str);
        if (index != util::Helpers::INVALID_INDEX) {
            return index;
        }

        return str;
    }

    if (prop->IsNumberLiteral()) {
        auto num = prop->AsNumberLiteral()->Number().GetDouble();
        if (util::Helpers::IsIndex(num)) {
            return static_cast<int64_t>(num);
        }

        return prop->AsNumberLiteral()->Str();
    }

    return res;
}

Operand PandaGen::ToPropertyKey(const ir::Expression *prop, bool is_computed, bool is_super_expression)
{
    Operand op = ToNamedPropertyKey(prop, is_computed);
    if (std::holds_alternative<util::StringView>(op)) {
        return op;
    }

    if (std::holds_alternative<int64_t>(op) && !is_super_expression) {
        return op;
    }

    VReg obj_reg = AllocReg();
    StoreAccumulator(prop, obj_reg);
    prop->Compile(this);

    return obj_reg;
}

Operand PandaGen::ToOwnPropertyKey(const ir::Expression *prop, bool is_computed)
{
    Operand op = ToNamedPropertyKey(prop, is_computed);
    if (std::holds_alternative<util::StringView>(op)) {
        ASSERT(std::holds_alternative<util::StringView>(op) || std::holds_alternative<int64_t>(op));
        return op;
    }

    if (std::holds_alternative<int64_t>(op)) {
        return op;
    }

    VReg prop_reg = AllocReg();
    prop->Compile(this);
    StoreAccumulator(prop, prop_reg);

    return prop_reg;
}

void PandaGen::LoadPropertyKeyAcc(const ir::Expression *prop, bool is_computed)
{
    Operand op = ToNamedPropertyKey(prop, is_computed);
    if (std::holds_alternative<util::StringView>(op)) {
        LoadAccumulatorString(prop, std::get<util::StringView>(op));
    } else if (std::holds_alternative<int64_t>(op)) {
        LoadAccumulatorInt(prop, static_cast<size_t>(std::get<int64_t>(op)));
    } else {
        prop->Compile(this);
    }
}

VReg PandaGen::LoadPropertyKey(const ir::Expression *prop, bool is_computed)
{
    LoadPropertyKeyAcc(prop, is_computed);

    VReg prop_reg = AllocReg();
    StoreAccumulator(prop, prop_reg);

    return prop_reg;
}

void PandaGen::LoadEvalVariable(const ir::AstNode *node, const util::StringView &name)
{
    RegScope rs(this);
    LoadLexicalContext(node);
    Ra().Emit<EcmaLdevalvar>(node, name);
}

void PandaGen::StoreEvalVariable(const ir::AstNode *node, const util::StringView &name)
{
    RegScope rs(this);
    VReg value = AllocReg();
    StoreAccumulator(node, value);
    LoadLexicalContext(node);
    Ra().Emit<EcmaStevalvar>(node, name, value);
}

void PandaGen::DirectEval(const ir::AstNode *node, uint32_t parser_status)
{
    RegScope rs(this);

    uint32_t index = 0;
    uint32_t eval_bindings_index = 0;
    VReg arg0 = AllocReg();
    StoreAccumulator(node, arg0);
    VReg bindings = AllocReg();
    CreateEmptyArray(node);
    StoreAccumulator(node, bindings);

    GetFunctionObject(node);
    StOwnByIndex(node, bindings, index++);
    GetNewTarget(node);
    StOwnByIndex(node, bindings, index++);
    GetThis(node);
    StOwnByIndex(node, bindings, index++);

    VReg eval_bindings = AllocReg();
    CreateEmptyArray(node);
    StoreAccumulator(node, eval_bindings);

    LoadAccumulator(node, LexEnv());
    StOwnByIndex(node, eval_bindings, eval_bindings_index++);

    const auto *iter = Scope()->EnclosingVariableScope();

    while (true) {
        uint32_t scope_bindings_buf = iter->EvalBindings();
        if (scope_bindings_buf != INVALID_LITERAL_BUFFER_ID) {
            Sa().Emit<EcmaLdevalbindings>(node, util::Helpers::ToStringView(Allocator(), scope_bindings_buf));
            StOwnByIndex(node, eval_bindings, eval_bindings_index++);
        }

        if (iter->Parent() == nullptr) {
            break;
        }

        iter = iter->Parent()->EnclosingVariableScope();
    }

    LoadAccumulator(node, eval_bindings);
    StOwnByIndex(node, bindings, index++);

    Sa().Emit<EcmaDirecteval>(node, static_cast<int64_t>(parser_status), arg0, bindings);
}

void PandaGen::LoadLexicalContext(const ir::AstNode *node)
{
    auto result = Scope()->Find(varbinder::VarBinder::LEXICAL_CONTEXT_PARAM);
    LoadLexicalVar(node, result.lex_level, result.variable->AsLocalVariable()->LexIdx());
}

bool PandaGen::IsDirectEval() const
{
    return Context()->IsDirectEval();
}

bool PandaGen::IsEval() const
{
    return Context()->IsEval();
}

const checker::Type *PandaGen::GetVRegType(VReg vreg) const
{
    // We assume that all used regs have any type
    if (vreg.GetIndex() > NextReg().GetIndex()) {
        return Context()->Checker()->GetGlobalTypesHolder()->GlobalAnyType();
    }

    return nullptr;
}

}  // namespace panda::es2panda::compiler
