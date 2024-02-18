/**
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

#include "parser/program/program.h"
#include "checker/ETSchecker.h"
#include "libpandabase/os/file.h"

#ifndef ES2PANDA_UTIL_DECLGEN_ETS2TS_H
#define ES2PANDA_UTIL_DECLGEN_ETS2TS_H

namespace ark::es2panda::util {

// Consume program after checker stage and generate out_path typescript file with declarations
bool GenerateTsDeclarations(checker::ETSChecker *checker, const ark::es2panda::parser::Program *program,
                            const std::string &outPath);

class TSDeclGen {
public:
    TSDeclGen(checker::ETSChecker *checker, const ark::es2panda::parser::Program *program)
        : checker_(checker), program_(program)
    {
    }

    std::stringstream &Output()
    {
        return output_;
    }

    void Generate();

    static constexpr std::string_view INDENT = "    ";

private:
    void ThrowError(std::string_view message, const lexer::SourcePosition &pos);
    std::string GetKeyName(const ir::Expression *key);

    void GenType(const checker::Type *checkerType);
    void GenFunctionType(const checker::ETSFunctionType *functionType, const ir::MethodDefinition *methodDef = nullptr);
    void GenObjectType(const checker::ETSObjectType *objectType);
    void GenEnumType(const checker::ETSEnumType *enumType);
    void GenUnionType(const checker::ETSUnionType *unionType);

    void GenImportDeclaration(const ir::ETSImportDeclaration *importDeclaration);
    void GenTypeAliasDeclaration(const ir::TSTypeAliasDeclaration *typeAlias);
    void GenEnumDeclaration(const ir::TSEnumDeclaration *enumDecl);
    void GenInterfaceDeclaration(const ir::TSInterfaceDeclaration *interfaceDecl);
    void GenClassDeclaration(const ir::ClassDeclaration *classDecl);
    void GenMethodDeclaration(const ir::MethodDefinition *methodDef);
    void GenPropDeclaration(const ir::ClassProperty *classProp);
    void GenLiteral(const ir::Literal *literal);

    template <class T>
    void GenModifier(const T *node);
    void GenTypeParameters(const ir::TSTypeParameterDeclaration *typeParams);

    template <class T, class CB>
    void GenCommaSeparated(const T &container, const CB &cb);

    void Out() {}
    template <class F, class... T>
    void Out(F &&first, T &&...rest)
    {
        output_ << first;
        Out(std::forward<T>(rest)...);
    }
    void OutEndl(const std::size_t count = 1)
    {
        ark::os::file::File::GetEndLine(output_, count);
    }

    void ResetState()
    {
        state_ = GenState();
    }

    struct GenState {
        const ir::Expression *super {nullptr};
        bool inInterface {false};
        bool inGlobalClass {false};
        std::string currentClassDescriptor {};
    } state_ {};

    std::stringstream output_ {};
    checker::ETSChecker *checker_ {};
    const ark::es2panda::parser::Program *program_ {};
};
}  // namespace ark::es2panda::util

#endif  // ES2PANDA_UTIL_DECLGEN_ETS2TS_H
