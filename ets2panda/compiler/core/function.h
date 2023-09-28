/**
 * Copyright (c) 2021-2022 Huawei Device Co., Ltd.
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

#ifndef ES2PANDA_COMPILER_CORE_FUNCTION_H
#define ES2PANDA_COMPILER_CORE_FUNCTION_H

#include "plugins/ecmascript/es2panda/ir/irnode.h"

namespace panda::es2panda::ir {
class ScriptFunction;
class AstNode;
}  // namespace panda::es2panda::ir

namespace panda::es2panda::compiler {
class PandaGen;

class Function {
public:
    Function() = delete;

    static void Compile(PandaGen *pg);
    static void CompileInstanceFields(PandaGen *pg, const ir::ScriptFunction *decl);

    static void LoadClassContexts(const ir::AstNode *node, PandaGen *pg, VReg ctor, const util::StringView &name);
};
}  // namespace panda::es2panda::compiler

#endif
