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

//
// desc: Object index access syntax is translated to the call of special setter (in case of assignment):
//       obj[i] = val; => obj.S_set(i, val);
//   	 or getter (in all the other cases):
//   	 ...obj[i]... => ...obj.S_get(i)...
//      methods.
//

#include "objectIndexAccess.h"

#include "checker/ETSchecker.h"
#include "parser/ETSparser.h"

namespace panda::es2panda::compiler {

std::string_view ObjectIndexLowering::Name()
{
    static std::string const NAME = "object-index-access";
    return NAME;
}

ir::Expression *ObjectIndexLowering::ProcessIndexSetAccess(parser::ETSParser *parser, checker::ETSChecker *checker,
                                                           ir::AssignmentExpression *assignment_expression) const
{
    //  Note! We assume that parser and checker phase nave been already passed correctly, thus the class has
    //  required accessible index method[s] and all the types are properly resolved.
    static std::string const CALL_EXPRESSION =
        std::string {"@@E1."} + std::string {compiler::Signatures::SET_INDEX_METHOD} + "(@@E2, @@E3)";

    // Parse ArkTS code string and create and process corresponding AST node(s)
    auto *const member_expression = assignment_expression->Left()->AsMemberExpression();
    auto *const lowering_result =
        parser->CreateFormattedExpression(CALL_EXPRESSION, parser::DEFAULT_SOURCE_FILE, member_expression->Object(),
                                          member_expression->Property(), assignment_expression->Right());
    lowering_result->SetParent(assignment_expression->Parent());

    lowering_result->Check(checker);
    return lowering_result;
}

ir::Expression *ObjectIndexLowering::ProcessIndexGetAccess(parser::ETSParser *parser, checker::ETSChecker *checker,
                                                           ir::MemberExpression *member_expression) const
{
    //  Note! We assume that parser and checker phase nave been already passed correctly, thus the class has
    //  required accessible index method[s] and all the types are properly resolved.
    static std::string const CALL_EXPRESSION =
        std::string {"@@E1."} + std::string {compiler::Signatures::GET_INDEX_METHOD} + "(@@E2)";

    // Parse ArkTS code string and create and process corresponding AST node(s)
    auto *const lowering_result = parser->CreateFormattedExpression(
        CALL_EXPRESSION, parser::DEFAULT_SOURCE_FILE, member_expression->Object(), member_expression->Property());
    lowering_result->SetParent(member_expression->Parent());

    lowering_result->Check(checker);
    return lowering_result;
}

bool ObjectIndexLowering::Perform(public_lib::Context *ctx, parser::Program *program)
{
    if (ctx->compiler_context->Options()->compilation_mode == CompilationMode::GEN_STD_LIB) {
        for (auto &[_, ext_programs] : program->ExternalSources()) {
            (void)_;
            for (auto *ext_prog : ext_programs) {
                Perform(ctx, ext_prog);
            }
        }
    }

    auto *const parser = ctx->parser->AsETSParser();
    ASSERT(parser != nullptr);
    auto *const checker = ctx->checker->AsETSChecker();
    ASSERT(checker != nullptr);

    program->Ast()->TransformChildrenRecursively([this, parser, checker](ir::AstNode *const ast) -> ir::AstNode * {
        if (ast->IsAssignmentExpression() && ast->AsAssignmentExpression()->Left()->IsMemberExpression() &&
            ast->AsAssignmentExpression()->Left()->AsMemberExpression()->Kind() ==
                ir::MemberExpressionKind::ELEMENT_ACCESS) {
            if (auto const *const object_type = ast->AsAssignmentExpression()->Left()->AsMemberExpression()->ObjType();
                object_type != nullptr && !object_type->IsETSDynamicType()) {
                return ProcessIndexSetAccess(parser, checker, ast->AsAssignmentExpression());
            }
        }
        return ast;
    });

    program->Ast()->TransformChildrenRecursively([this, parser, checker](ir::AstNode *const ast) -> ir::AstNode * {
        if (ast->IsMemberExpression() &&
            ast->AsMemberExpression()->Kind() == ir::MemberExpressionKind::ELEMENT_ACCESS) {
            if (auto const *const object_type = ast->AsMemberExpression()->ObjType();
                object_type != nullptr && !object_type->IsETSDynamicType()) {
                return ProcessIndexGetAccess(parser, checker, ast->AsMemberExpression());
            }
        }
        return ast;
    });

    return true;
}

bool ObjectIndexLowering::Postcondition(public_lib::Context *ctx, const parser::Program *program)
{
    if (ctx->compiler_context->Options()->compilation_mode == CompilationMode::GEN_STD_LIB) {
        for (auto &[_, ext_programs] : program->ExternalSources()) {
            (void)_;
            for (auto *ext_prog : ext_programs) {
                if (!Postcondition(ctx, ext_prog)) {
                    return false;
                }
            }
        }
    }

    return !program->Ast()->IsAnyChild([](const ir::AstNode *ast) {
        if (ast->IsMemberExpression() &&
            ast->AsMemberExpression()->Kind() == ir::MemberExpressionKind::ELEMENT_ACCESS) {
            if (auto const *const object_type = ast->AsMemberExpression()->ObjType(); object_type != nullptr) {
                return !object_type->IsETSDynamicType();
            }
        }
        return false;
    });
}

}  // namespace panda::es2panda::compiler
