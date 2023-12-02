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

#include "generateDeclarations.h"
#include "checker/checker.h"
#include "compiler/core/compilerContext.h"
#include "util/declgenEts2Ts.h"

namespace panda::es2panda::compiler {
bool GenerateTsDeclarationsPhase::Perform(public_lib::Context *ctx, parser::Program *program)
{
    auto *checker = ctx->checker;
    return (
        ctx->compiler_context->Options()->ts_decl_out.empty() ||
        util::GenerateTsDeclarations(checker->AsETSChecker(), program, ctx->compiler_context->Options()->ts_decl_out));
}

}  // namespace panda::es2panda::compiler
