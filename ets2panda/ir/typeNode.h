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

#ifndef ES2PANDA_IR_TYPE_NODE_H
#define ES2PANDA_IR_TYPE_NODE_H

#include "plugins/ecmascript/es2panda/ir/expression.h"

namespace panda::es2panda::checker {
class TSChecker;
class Type;
}  // namespace panda::es2panda::checker

namespace panda::es2panda::ir {
class TypeNode : public Expression {
public:
    explicit TypeNode(AstNodeType type) : Expression(type) {}
    explicit TypeNode(AstNodeType type, ModifierFlags flags) : Expression(type, flags) {}

    bool IsTypeNode() const override
    {
        return true;
    }

    virtual checker::Type *GetType([[maybe_unused]] checker::TSChecker *checker)
    {
        return nullptr;
    }

    virtual checker::Type *GetType([[maybe_unused]] checker::ETSChecker *checker)
    {
        return nullptr;
    }
};
}  // namespace panda::es2panda::ir

#endif /* ES2PANDA_IR_TYPE_NODE_H */
