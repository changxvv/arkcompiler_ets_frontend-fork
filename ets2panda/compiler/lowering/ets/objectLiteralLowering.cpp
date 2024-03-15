/*
 * Copyright (c) 2024 Huawei Device Co., Ltd.
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

#include "objectLiteralLowering.h"

#include "checker/ETSchecker.h"
#include "compiler/lowering/scopesInit/scopesInitPhase.h"
#include "compiler/lowering/util.h"

namespace ark::es2panda::compiler {

std::string_view ObjectLiteralLowering::Name() const
{
    return "ObjectLiteralLowering";
}

static constexpr std::string_view NESTED_BLOCK_EXPRESSION = "_$NESTED_BLOCK_EXPRESSION$_";

static void RestoreNestedBlockExpression(const ArenaVector<ir::Statement *> &statements,
                                         std::deque<ir::BlockExpression *> &nestedBlckExprs, varbinder::Scope *scope)
{
    if (!nestedBlckExprs.empty()) {
        for (auto stmt : statements) {
            if (!stmt->IsExpressionStatement() ||
                !stmt->AsExpressionStatement()->GetExpression()->IsAssignmentExpression()) {
                continue;
            }

            auto *assign = stmt->AsExpressionStatement()->GetExpression()->AsAssignmentExpression();

            if (assign->Right()->IsStringLiteral() &&
                assign->Right()->AsStringLiteral()->Str().Is(NESTED_BLOCK_EXPRESSION)) {
                auto nestedBlckExpr = nestedBlckExprs.front();
                nestedBlckExprs.pop_front();
                nestedBlckExpr->Scope()->SetParent(scope);
                assign->SetRight(nestedBlckExpr);
            }
        }
        // All nested block expressions should be restored
        ASSERT(nestedBlckExprs.empty());
    }
}

static void GenerateNewStatements(checker::ETSChecker *checker, ir::ObjectExpression *objExpr, std::stringstream &ss,
                                  std::vector<ir::AstNode *> &newStmts,
                                  std::deque<ir::BlockExpression *> &nestedBlckExprs)
{
    auto *const allocator = checker->Allocator();

    auto *const classType = objExpr->PreferredType()->AsETSObjectType();

    auto addNode = [&newStmts](ir::AstNode *node) -> int {
        newStmts.emplace_back(node);
        return newStmts.size();
    };

    // Generating: let <genSym>: <preferredType> = new <preferredType>();
    auto *genSymIdent = Gensym(allocator);
    auto *preferredType = checker->AllocNode<ir::OpaqueTypeNode>(classType);
    ss << "let @@I" << addNode(genSymIdent) << ": @@T" << addNode(preferredType) << " = new @@I"
       << addNode(checker->AllocNode<ir::Identifier>(classType->Name(), allocator));

    // Type params of class type
    if (!classType->TypeArguments().empty()) {
        ss << "<";
        for (auto *type : classType->TypeArguments()) {
            type->ToString(ss);
            ss << ",";
        }
        ss << ">";
    }
    ss << "();" << std::endl;

    // Generating: <genSym>.key_i = value_i      ( i <= [0, object_literal.properties.size) )
    for (auto *propExpr : objExpr->Properties()) {
        ASSERT(propExpr->IsProperty());
        auto *prop = propExpr->AsProperty();
        ir::Expression *key = prop->Key();
        ir::Expression *value = prop->Value();

        ir::Identifier *keyIdent = key->IsStringLiteral()
                                       ? checker->AllocNode<ir::Identifier>(key->AsStringLiteral()->Str(), allocator)
                                       : key->AsIdentifier();

        ss << "@@I" << addNode(genSymIdent->Clone(allocator, nullptr)) << ".@@I" << addNode(keyIdent);

        if (value->IsBlockExpression()) {
            // Case of nested object literal (all nested object literals has already been processed)
            // Corresponding nested block expressions should be stored somewhere and restored after ScopesPhase
            // Because it has already processed them
            // Predefined String Literal acts as placeholder
            ss << " = \"" << NESTED_BLOCK_EXPRESSION << "\";" << std::endl;
            nestedBlckExprs.emplace_back(value->AsBlockExpression());
        } else {
            ss << " = @@E" << addNode(value) << ";" << std::endl;
        }
    }

    ss << "(@@I" << addNode(genSymIdent->Clone(allocator, nullptr)) << ");" << std::endl;
}

static ir::AstNode *HandleObjectLiteralLowering(public_lib::Context *ctx, ir::ObjectExpression *objExpr)
{
    /*
     * For given object literal of class type generates following block expression:
     *
     *  ({
     *     let <genSym>: <preferredType> = new <preferredType>();
     *     <genSym>.key_i = value_i      ( i <= [0, object_literal.properties.size) )
     *     <genSym>;                     <-- NOTE: result of block expression
     *  })
     */

    if (objExpr->PreferredType() == nullptr) {
        return objExpr;
    }

    auto *const checker = ctx->checker->AsETSChecker();
    auto *const parser = ctx->parser->AsETSParser();
    auto *const varbinder = ctx->compilerContext->VarBinder()->AsETSBinder();

    std::stringstream ss;
    // Double-ended queue for storing nested block expressions that have already been processed earlier
    std::deque<ir::BlockExpression *> nestedBlckExprs;
    std::vector<ir::AstNode *> newStmts;

    GenerateNewStatements(checker, objExpr, ss, newStmts, nestedBlckExprs);

    auto *loweringResult = parser->CreateFormattedExpression(ss.str(), newStmts, parser::DEFAULT_SOURCE_FILE);
    loweringResult->SetParent(objExpr->Parent());

    auto scopeCtx = varbinder::LexicalScope<varbinder::Scope>::Enter(varbinder, NearestScope(objExpr));
    InitScopesPhaseETS::RunExternalNode(loweringResult, varbinder);

    // Restoring nested block expressions
    RestoreNestedBlockExpression(loweringResult->AsBlockExpression()->Statements(), nestedBlckExprs,
                                 loweringResult->Scope());

    varbinder->ResolveReferencesForScope(loweringResult, NearestScope(loweringResult));

    checker::SavedCheckerContext scc {checker, checker::CheckerStatus::IGNORE_VISIBILITY};
    loweringResult->Check(checker);

    return loweringResult;
}

bool ObjectLiteralLowering::Perform(public_lib::Context *ctx, parser::Program *program)
{
    for (auto &[_, extPrograms] : program->ExternalSources()) {
        (void)_;
        for (auto *extProg : extPrograms) {
            Perform(ctx, extProg);
        }
    }

    program->Ast()->TransformChildrenRecursively([ctx](ir::AstNode *ast) -> ir::AstNode * {
        // Skip processing dynamic objects
        if (ast->IsObjectExpression() && !ast->AsObjectExpression()->PreferredType()->AsETSObjectType()->HasObjectFlag(
                                             checker::ETSObjectFlags::DYNAMIC)) {
            return HandleObjectLiteralLowering(ctx, ast->AsObjectExpression());
        }
        return ast;
    });

    return true;
}

bool ObjectLiteralLowering::Postcondition(public_lib::Context *ctx, const parser::Program *program)
{
    for (auto &[_, extPrograms] : program->ExternalSources()) {
        (void)_;
        for (auto *extProg : extPrograms) {
            if (!Postcondition(ctx, extProg)) {
                return false;
            }
        }
    }

    // In all object literal contexts (except dynamic) a substitution should take place
    return !program->Ast()->IsAnyChild([](const ir::AstNode *ast) -> bool {
        return ast->IsObjectExpression() &&
               !ast->AsObjectExpression()->PreferredType()->AsETSObjectType()->HasObjectFlag(
                   checker::ETSObjectFlags::DYNAMIC);
    });
}

}  // namespace ark::es2panda::compiler