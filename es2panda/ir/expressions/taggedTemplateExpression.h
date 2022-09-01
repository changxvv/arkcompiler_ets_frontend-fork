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

#ifndef ES2PANDA_IR_EXPRESSION_TAGGED_TEMPLATE_EXPRESSION_H
#define ES2PANDA_IR_EXPRESSION_TAGGED_TEMPLATE_EXPRESSION_H

#include <ir/expression.h>

namespace panda::es2panda::compiler {
class PandaGen;
}  // namespace panda::es2panda::compiler

namespace panda::es2panda::checker {
class Checker;
class Type;
}  // namespace panda::es2panda::checker

namespace panda::es2panda::ir {

class TemplateLiteral;
class TSTypeParameterInstantiation;

class TaggedTemplateExpression : public Expression {
public:
    explicit TaggedTemplateExpression(Expression *tag, TemplateLiteral *quasi,
                                      TSTypeParameterInstantiation *typeParams)
        : Expression(AstNodeType::TAGGED_TEMPLATE_EXPRESSION), tag_(tag), quasi_(quasi), typeParams_(typeParams)
    {
    }

    const Expression *Tag() const
    {
        return tag_;
    }

    const TemplateLiteral *Quasi() const
    {
        return quasi_;
    }

    const TSTypeParameterInstantiation *TypeParams() const
    {
        return typeParams_;
    }

    void Iterate(const NodeTraverser &cb) const override;
    void Dump(ir::AstDumper *dumper) const override;
    void Compile(compiler::PandaGen *pg) const override;
    checker::Type *Check(checker::Checker *checker) const override;

protected:
    Expression *tag_;
    TemplateLiteral *quasi_;
    TSTypeParameterInstantiation *typeParams_;
};

}  // namespace panda::es2panda::ir

#endif
