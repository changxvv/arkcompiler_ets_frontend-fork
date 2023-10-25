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

#ifndef ES2PANDA_COMPILER_CORE_ENV_SCOPE_H
#define ES2PANDA_COMPILER_CORE_ENV_SCOPE_H

#include "binder/scope.h"
#include "ir/irnode.h"
#include "compiler/core/dynamicContext.h"
#include "compiler/core/regScope.h"
#include "compiler/core/labelTarget.h"

namespace panda::es2panda::ir {
class AstNode;
class Statement;
}  // namespace panda::es2panda::ir

namespace panda::es2panda::compiler {
class PandaGen;

class ScopeContext {
public:
    explicit ScopeContext(CodeGen *cg, binder::Scope *new_scope);
    ~ScopeContext();

    NO_COPY_SEMANTIC(ScopeContext);
    NO_MOVE_SEMANTIC(ScopeContext);

private:
    CodeGen *cg_;
    binder::Scope *prev_scope_;
};

class EnvScope {
public:
    explicit EnvScope() = default;

    NO_COPY_SEMANTIC(EnvScope);
    NO_MOVE_SEMANTIC(EnvScope);
    ~EnvScope();

    void Initialize(PandaGen *pg, VReg lex_env);

    VReg LexEnv() const
    {
        return lex_env_;
    }

    EnvScope *Prev() const
    {
        return prev_;
    }

protected:
    friend class PandaGen;

    // NOLINTBEGIN(misc-non-private-member-variables-in-classes)
    PandaGen *pg_ {};
    EnvScope *prev_ {};
    VReg lex_env_ {};
    // NOLINTEND(misc-non-private-member-variables-in-classes)
};

class LoopEnvScope : public EnvScope {
public:
    explicit LoopEnvScope(PandaGen *pg, binder::LoopScope *scope, LabelTarget target)
        : scope_(NeedEnv(scope) ? scope : nullptr), reg_scope_(pg, scope), lex_env_ctx_(this, pg, target)
    {
        CopyBindings(pg, scope, binder::VariableFlags::PER_ITERATION);
    }

    explicit LoopEnvScope(PandaGen *pg, LabelTarget target, binder::LoopScope *scope)
        : scope_(NeedEnv(scope) ? scope : nullptr), reg_scope_(pg), lex_env_ctx_(this, pg, target)
    {
        CopyBindings(pg, scope, binder::VariableFlags::PER_ITERATION);
    }

    explicit LoopEnvScope(PandaGen *pg, binder::LoopDeclarationScope *scope)
        : scope_(NeedEnv(scope) ? scope : nullptr), reg_scope_(pg), lex_env_ctx_(this, pg, {})
    {
        CopyBindings(pg, scope, binder::VariableFlags::LOOP_DECL);
    }

    binder::VariableScope *Scope() const
    {
        ASSERT(HasEnv());
        return scope_;
    }

    bool HasEnv() const
    {
        return scope_ != nullptr;
    }

    void CopyPetIterationCtx();

private:
    static bool NeedEnv(binder::VariableScope *scope)
    {
        return scope->IsVariableScope() && scope->AsVariableScope()->NeedLexEnv();
    }

    void CopyBindings(PandaGen *pg, binder::VariableScope *scope, binder::VariableFlags flag);

    binder::VariableScope *scope_ {};
    LocalRegScope reg_scope_;
    LexEnvContext lex_env_ctx_;
};
}  // namespace panda::es2panda::compiler

#endif