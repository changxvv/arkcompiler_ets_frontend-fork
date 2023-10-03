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

#ifndef ES2PANDA_IR_BASE_CATCH_CLAUSE_H
#define ES2PANDA_IR_BASE_CATCH_CLAUSE_H

#include "ir/statement.h"

namespace panda::es2panda::binder {
class CatchScope;
}  // namespace panda::es2panda::binder

namespace panda::es2panda::ir {
class BlockStatement;
class Expression;

class CatchClause : public TypedStatement {
public:
    explicit CatchClause(binder::CatchScope *scope, Expression *param, BlockStatement *body)
        : TypedStatement(AstNodeType::CATCH_CLAUSE), scope_(scope), param_(param), body_(body)
    {
    }

    Expression *Param()
    {
        return param_;
    }

    const Expression *Param() const
    {
        return param_;
    }

    BlockStatement *Body()
    {
        return body_;
    }

    const BlockStatement *Body() const
    {
        return body_;
    }

    binder::CatchScope *Scope() const
    {
        return scope_;
    }

    bool IsDefaultCatchClause() const;
    void Iterate(const NodeTraverser &cb) const override;
    void Dump(ir::AstDumper *dumper) const override;
    void Compile([[maybe_unused]] compiler::PandaGen *pg) const override;
    void Compile([[maybe_unused]] compiler::ETSGen *etsg) const override;
    checker::Type *Check([[maybe_unused]] checker::TSChecker *checker) override;
    checker::Type *Check([[maybe_unused]] checker::ETSChecker *checker) override;

private:
    binder::CatchScope *scope_;
    Expression *param_;
    BlockStatement *body_;
};
}  // namespace panda::es2panda::ir

#endif
