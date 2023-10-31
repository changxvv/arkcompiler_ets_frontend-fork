/*
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

#include "catchTable.h"

#include "compiler/core/pandagen.h"

namespace panda::es2panda::compiler {
TryLabelSet::TryLabelSet(CodeGen *cg)
    : try_(cg->AllocLabel(), cg->AllocLabel()), catch_(cg->AllocLabel(), cg->AllocLabel())
{
}

TryLabelSet::TryLabelSet(CodeGen *cg, LabelPair try_label_pair)
    : try_(try_label_pair), catch_(cg->AllocLabel(), cg->AllocLabel())
{
}
}  // namespace panda::es2panda::compiler
