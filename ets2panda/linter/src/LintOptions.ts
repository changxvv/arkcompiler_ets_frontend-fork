/*
 * Copyright (c) 2023-2023 Huawei Device Co., Ltd.
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

import * as ts from 'typescript';
import { CommandLineOptions } from './CommandLineOptions';
import { IncrementalLintInfo } from './IncrementalLintInfo';
import { ReportAutofixCallback } from './autofixes/ReportAutofixCallback';
import { IsFileFromModuleCallback } from './IsFileFromModuleCallback';

// common options interface, additional fields may be used by plugins
export interface LintOptions {
  cmdOptions: CommandLineOptions;
  realtimeLint: boolean;
  cancellationToken?: ts.CancellationToken;
  incrementalLintInfo?: IncrementalLintInfo;
  tsProgram?: ts.Program;
  reportAutofixCb?: ReportAutofixCallback;
  isFileFromModuleCb?: IsFileFromModuleCallback;
  [key: string]: any;
}
