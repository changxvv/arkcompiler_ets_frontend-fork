/**
 * Copyright (c) 2022 Huawei Device Co., Ltd.
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

#ifndef MERGE_ABC_MERGE_PROGRAM_H
#define MERGE_ABC_MERGE_PROGRAM_H

#include "assembly-ins.h"
#include "assembly-program.h"
#include "assembler/assembly-function.h"

namespace panda::proto {
class MergeProgram {
public:
    static bool GetProtoFiles(std::string &protoBinPath, std::string &protoBinSuffix,
                              std::vector<std::string> &directoryFiles);
    static bool AppendProtoFiles(std::string filePath, std::string protoBinSuffix,
                                 std::vector<std::string> &protoFiles);
    static bool CollectProtoFiles(std::string input, std::string protoBinSuffix, std::vector<std::string> &protoFiles);
};
} // namespace panda::proto
#endif  // MERGE_ABC_MERGE_PROGRAM_H
