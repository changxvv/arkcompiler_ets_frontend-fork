/**
 * Copyright (c) 2023 Huawei Device Co., Ltd.
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *     http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */

#ifndef ES2PANDA_UTIL_PATH_H
#define ES2PANDA_UTIL_PATH_H

#include <cstdio>
#include <string>
#include "util/ustring.h"

namespace panda::es2panda::util {
const char PATH_DELIMITER =
#ifdef _WIN32
    '\\';
#else
    '/';
#endif

class Path {
public:
    Path();
    Path(const std::string &absolute_path, ArenaAllocator *allocator);
    Path(const std::string &relative_path, const std::string &base_path, ArenaAllocator *allocator);
    Path(const util::StringView &absolute_path, ArenaAllocator *allocator);
    Path(const util::StringView &relative_path, const util::StringView &base_path, ArenaAllocator *allocator);
    bool IsRelative();
    bool IsAbsolute();
    const util::StringView &GetPath() const;
    const util::StringView &GetAbsolutePath() const;
    const util::StringView &GetExtension() const;
    const util::StringView &GetFileName() const;
    const util::StringView &GetParentFolder() const;
    const util::StringView &GetAbsoluteParentFolder() const;
    constexpr static char GetPathDelimiter()
    {
        return PATH_DELIMITER;
    }

private:
    ArenaAllocator *allocator_ {};
    bool is_relative_ {};
    util::StringView path_ {};
    util::StringView base_path_ {};
    util::StringView absolute_path_ {};
    util::StringView file_name_ {};
    util::StringView file_extension_ {};
    util::StringView parent_folder_ {};
    util::StringView absolute_parent_folder_ {};

    void Initializer(const std::string &absolute_path, ArenaAllocator *allocator);
    void InitializeBasePath(std::string base_path);
    void InitializeAbsolutePath();
    void InitializeParentFolder();
    void InitializeAbsoluteParentFolder();
    void InitializeFileName();
    void InitializeFileExtension();
};
}  // namespace panda::es2panda::util

#endif