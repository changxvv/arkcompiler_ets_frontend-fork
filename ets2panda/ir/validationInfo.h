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

#ifndef ES2PANDA_IR_VALIDATION_INFO_H
#define ES2PANDA_IR_VALIDATION_INFO_H

#include "lexer/token/sourceLocation.h"
#include "util/ustring.h"

namespace panda::es2panda::ir {
class ValidationInfo {
public:
    ValidationInfo() noexcept = default;
    ValidationInfo(util::StringView m, lexer::SourcePosition p) noexcept : msg(m), pos(p) {}

    inline bool Fail() const
    {
        return !msg.Empty();
    }

    // NOLINTBEGIN(misc-non-private-member-variables-in-classes)
    util::StringView msg {};
    lexer::SourcePosition pos {};
    // NOLINTEND(misc-non-private-member-variables-in-classes)
};
}  // namespace panda::es2panda::ir

#endif
