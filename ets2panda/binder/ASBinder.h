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

#ifndef ES2PANDA_BINDER_AS_BINDER_H
#define ES2PANDA_BINDER_AS_BINDER_H

#include "plugins/ecmascript/es2panda/binder/binder.h"

namespace panda::es2panda::binder {
class ASBinder : public Binder {
public:
    explicit ASBinder(ArenaAllocator *allocator) : Binder(allocator) {}

    NO_COPY_SEMANTIC(ASBinder);
    NO_MOVE_SEMANTIC(ASBinder);
    ~ASBinder() = default;

    ScriptExtension Extension() const override
    {
        return ScriptExtension::AS;
    }

    ResolveBindingOptions BindingOptions() const override
    {
        return ResolveBindingOptions::BINDINGS;
    }

    void IdentifierAnalysis() override {}

private:
};
}  // namespace panda::es2panda::binder

#endif
