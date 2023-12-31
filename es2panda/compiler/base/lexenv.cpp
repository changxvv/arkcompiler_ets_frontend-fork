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

#include "lexenv.h"

#include <binder/variable.h>
#include <compiler/core/compilerContext.h>
#include <compiler/core/envScope.h>
#include <compiler/core/pandagen.h>
#include <ir/base/classDefinition.h>
#include <ir/expressions/identifier.h>
#include <typescript/extractor/typeRecorder.h>

namespace panda::es2panda::compiler {

// Helpers

static bool CheckTdz(const ir::AstNode *node)
{
    return node->IsIdentifier() && node->AsIdentifier()->IsTdz();
}

static void CheckConstAssignment(PandaGen *pg, const ir::AstNode *node, binder::Variable *variable)
{
    if (!variable->Declaration()->IsConstDecl()) {
        return;
    }

    pg->ThrowConstAssignment(node, variable->Name());
}

// VirtualLoadVar

static void ExpandLoadLexVar(PandaGen *pg, const ir::AstNode *node, const binder::ScopeFindResult &result)
{
    if (result.scope->IsClassScope()) {
        auto classDef = result.scope->AsClassScope()->Node()->AsClassDefinition();
        if (classDef->IsSendable()) {
            ASSERT(classDef->Ident() != nullptr);
            ASSERT(result.name == classDef->Ident()->Name());
            pg->LoadSendableClass(node, result.lexLevel);
            return;
        }
    }
    pg->LoadLexicalVar(node, result.lexLevel, result.variable->AsLocalVariable()->LexIdx(), result.variable->Name());
    const auto *decl = result.variable->Declaration();
    if (decl->IsLetOrConstOrClassDecl()) {
        pg->ThrowUndefinedIfHole(node, result.variable->Name());
    }
}

static void ExpandLoadNormalVar(PandaGen *pg, const ir::AstNode *node, const binder::ScopeFindResult &result)
{
    auto *local = result.variable->AsLocalVariable();

    if (CheckTdz(node)) {
        pg->LoadConst(node, Constant::JS_HOLE);
        pg->ThrowUndefinedIfHole(node, local->Name());
    } else {
        pg->LoadAccumulator(node, local->Vreg());
    }
}

void VirtualLoadVar::Expand(PandaGen *pg, const ir::AstNode *node, const binder::ScopeFindResult &result)
{
    if (result.variable->LexicalBound()) {
        ExpandLoadLexVar(pg, node, result);
    } else {
        ExpandLoadNormalVar(pg, node, result);
    }
}

// VirtualStoreVar

static void ExpandStoreLexVar(PandaGen *pg, const ir::AstNode *node, const binder::ScopeFindResult &result, bool isDecl)
{
    if (result.scope->IsClassScope()) {
        auto classDef = result.scope->AsClassScope()->Node()->AsClassDefinition();
        if (classDef->IsSendable()) {
            ASSERT(classDef->Ident() != nullptr);
            ASSERT(result.name == classDef->Ident()->Name());
            return;
        }
    }

    binder::LocalVariable *local = result.variable->AsLocalVariable();

    const auto *decl = result.variable->Declaration();

    if (decl->IsLetOrConstOrClassDecl() && !isDecl) {
        RegScope rs(pg);

        VReg valueReg = pg->AllocReg();
        pg->StoreAccumulator(node, valueReg);

        ExpandLoadLexVar(pg, node, result);

        if (decl->IsConstDecl()) {
            pg->ThrowConstAssignment(node, local->Name());
        }

        pg->LoadAccumulator(node, valueReg);
    }

    pg->StoreLexicalVar(node, result.lexLevel, local->LexIdx(), local);
}

static void ExpandStoreNormalVar(PandaGen *pg, const ir::AstNode *node, const binder::ScopeFindResult &result,
                                 bool isDecl)
{
    auto *local = result.variable->AsLocalVariable();
    VReg localReg = local->Vreg();

    if (!isDecl) {
        if (CheckTdz(node)) {
            pg->LoadConst(node, Constant::JS_HOLE);
            pg->ThrowUndefinedIfHole(node, local->Name());
        }

        CheckConstAssignment(pg, node, local);
    }

    auto context = pg->Context();
    if (context->IsTypeExtractorEnabled() && pg->TypedVars().count(local) == 0U) {
        auto fn = [&pg, &node, &local, &localReg](auto typeIndex, const auto &tag) {
            if (typeIndex != extractor::TypeRecorder::PRIMITIVETYPE_ANY) {
                pg->StoreAccumulatorWithType(node, typeIndex, localReg);
                pg->TypedVars().emplace(local);
                DCOUT << "[LOG]Local vreg in " << tag << " has type index: " << local->Name() <<
                    "@" << local << " | " << typeIndex << std::endl;
                return true;
            }
            return false;
        };
        if (fn(context->TypeRecorder()->GetVariableTypeIndex(local), "variable")) {
            return;
        }
        if (fn(context->TypeRecorder()->GetNodeTypeIndex(node), "declnode")) {
            return;
        }
        DCOUT << "[WARNING]Local vreg lose type index: " << local->Name() << "@" << local << std::endl;
    }
    pg->StoreAccumulator(node, localReg);
}

void VirtualStoreVar::Expand(PandaGen *pg, const ir::AstNode *node, const binder::ScopeFindResult &result, bool isDecl)
{
    if (result.variable->LexicalBound()) {
        ExpandStoreLexVar(pg, node, result, isDecl);
    } else {
        ExpandStoreNormalVar(pg, node, result, isDecl);
    }
}

}  // namespace panda::es2panda::compiler
