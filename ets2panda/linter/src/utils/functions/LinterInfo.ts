/*
 * Copyright (c) 2022-2023 Huawei Device Co., Ltd.
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

import type { ProblemInfo } from '../../ProblemInfo';
import type { AutofixInfo } from '../../autofixes/AutofixInfo';

export function encodeProblemInfo(problem: ProblemInfo): string {
  return `${problem.problem}%${problem.start}%${problem.end}`;
}

export function decodeAutofixInfo(info: string): AutofixInfo {
  const infos = info.split('%');
  return {
    problemID: infos[0],
    start: Number.parseInt(infos[1]),
    end: Number.parseInt(infos[2])
  };
}
