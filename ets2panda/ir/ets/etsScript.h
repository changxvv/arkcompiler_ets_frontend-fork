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

#ifndef ES2PANDA_IR_ETS_SCRIPT_H
#define ES2PANDA_IR_ETS_SCRIPT_H

#include "ir/statements/blockStatement.h"

namespace panda::es2panda::parser {
class Program;
}  // namespace panda::es2panda::parser

namespace panda::es2panda::ir {

class ETSScript : public BlockStatement {
public:
    explicit ETSScript(ArenaAllocator *allocator, varbinder::Scope *scope, ArenaVector<Statement *> &&statement_list,
                       parser::Program *program)
        : BlockStatement(allocator, scope, std::move(statement_list)), program_(program)
    {
    }

    parser::Program *Program()
    {
        return program_;
    }

    const parser::Program *Program() const
    {
        return program_;
    }

private:
    parser::Program *program_;
};
}  // namespace panda::es2panda::ir

#endif
