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

#ifndef ES2PANDA_IR_STATEMENT_SWITCH_CASE_STATEMENT_H
#define ES2PANDA_IR_STATEMENT_SWITCH_CASE_STATEMENT_H

#include "ir/statement.h"

namespace panda::es2panda::ir {
class Expression;

class SwitchCaseStatement : public Statement {
public:
    explicit SwitchCaseStatement(Expression *test, ArenaVector<Statement *> &&consequent)
        : Statement(AstNodeType::SWITCH_CASE_STATEMENT), test_(test), consequent_(std::move(consequent))
    {
    }

    Expression *Test()
    {
        return test_;
    }

    const Expression *Test() const
    {
        return test_;
    }

    const ArenaVector<Statement *> &Consequent() const
    {
        return consequent_;
    }

    void TransformChildren(const NodeTransformer &cb) override;
    void SetReturnType(checker::ETSChecker *checker, checker::Type *type) override
    {
        for (auto *statement : consequent_) {
            if (statement != nullptr) {
                statement->SetReturnType(checker, type);
            }
        }
    }

    void Iterate(const NodeTraverser &cb) const override;
    void Dump(ir::AstDumper *dumper) const override;
    void Compile(compiler::PandaGen *pg) const override;
    void Compile(compiler::ETSGen *etsg) const override;
    checker::Type *Check(checker::TSChecker *checker) override;
    checker::Type *Check(checker::ETSChecker *checker) override;

private:
    Expression *test_;
    ArenaVector<Statement *> consequent_;
};
}  // namespace panda::es2panda::ir

#endif
