/*
 * Copyright (c) 2023-2024 Huawei Device Co., Ltd.
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

#ifndef ES2PANDA_COMPILER_LOWERING_LOCAL_CLASS_LOWERING_H
#define ES2PANDA_COMPILER_LOWERING_LOCAL_CLASS_LOWERING_H

#include "compiler/lowering/phase.h"

namespace ark::es2panda::compiler {

class LocalClassConstructionPhase : public Phase {
public:
    std::string_view Name() const override;
    bool Perform(public_lib::Context *ctx, parser::Program *program) override;

protected:
    void ReplaceReferencesFromTheParametersToTheLocalVariavbles(
        ir::ClassDefinition *classDef,
        const ArenaMap<varbinder::Variable *, varbinder::Variable *> &newLocalVariablesMap,
        const ArenaSet<ir::Identifier *> &initializers);

    void CreateTemporalLocalVariableForModifiedParameters(public_lib::Context *ctx, ir::ClassDefinition *classDef);

    void CreateClassPropertiesForCapturedVariables(public_lib::Context *ctx, ir::ClassDefinition *classDef,
                                                   ArenaMap<varbinder::Variable *, varbinder::Variable *> &variableMap,
                                                   ArenaMap<varbinder::Variable *, ir::ClassProperty *> &propertyMap);

    void ModifyConstructorParameters(public_lib::Context *ctx, ir::ClassDefinition *classDef,
                                     ArenaMap<varbinder::Variable *, varbinder::Variable *> &variableMap,
                                     ArenaMap<varbinder::Variable *, varbinder::Variable *> &parameterMap);

    void RemapReferencesFromCapturedVariablesToClassProperties(
        ir::ClassDefinition *classDef, ArenaMap<varbinder::Variable *, varbinder::Variable *> &variableMap);
};

}  // namespace ark::es2panda::compiler

#endif
