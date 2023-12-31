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

#ifndef ES2PANDA_IR_STATEMENT_FUNCTION_DECLARATION_H
#define ES2PANDA_IR_STATEMENT_FUNCTION_DECLARATION_H

#include "ir/statement.h"

namespace panda::es2panda::ir {
class ScriptFunction;

class FunctionDeclaration : public Statement {
public:
    explicit FunctionDeclaration(ArenaAllocator *allocator, ScriptFunction *func)
        : Statement(AstNodeType::FUNCTION_DECLARATION), decorators_(allocator->Adapter()), func_(func)
    {
    }

    ScriptFunction *Function()
    {
        return func_;
    }

    const ScriptFunction *Function() const
    {
        return func_;
    }

    void AddDecorators([[maybe_unused]] ArenaVector<ir::Decorator *> &&decorators) override
    {
        decorators_ = std::move(decorators);
    }

    bool CanHaveDecorator([[maybe_unused]] bool in_ts) const override
    {
        return !in_ts;
    }

    void TransformChildren(const NodeTransformer &cb) override;
    void Iterate(const NodeTraverser &cb) const override;
    void Dump(ir::AstDumper *dumper) const override;
    void Compile(compiler::PandaGen *pg) const override;
    void Compile(compiler::ETSGen *etsg) const override;
    checker::Type *Check(checker::TSChecker *checker) override;
    checker::Type *Check(checker::ETSChecker *checker) override;

private:
    ArenaVector<Decorator *> decorators_;
    ScriptFunction *func_;
};
}  // namespace panda::es2panda::ir

#endif
