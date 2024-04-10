/*
 * Copyright (c) 2022-2024 Huawei Device Co., Ltd.
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

import { TypeScriptLinter } from '../lib/TypeScriptLinter';
import { parseCommandLine } from './CommandLineParser';
import { Logger } from '../lib/Logger';
import * as fs from 'node:fs';
import * as os from 'node:os';
import * as readline from 'node:readline';
import * as path from 'node:path';
import type { CommandLineOptions } from '../lib/CommandLineOptions';
import { lint } from '../lib/LinterRunner';
import { compileLintOptions } from './Compiler';

export function run(): void {
  const commandLineArgs = process.argv.slice(2);
  if (commandLineArgs.length === 0) {
    Logger.info('Command line error: no arguments');
    process.exit(-1);
  }

  const cmdOptions = parseCommandLine(commandLineArgs);

  if (cmdOptions.testMode) {
    TypeScriptLinter.testMode = true;
  }

  TypeScriptLinter.initGlobals();

  if (!cmdOptions.ideMode) {
    const result = lint(compileLintOptions(cmdOptions));
    process.exit(result.errorNodes > 0 ? 1 : 0);
  } else {
    runIDEMode(cmdOptions);
  }
}

function getTempFileName(): string {
  return path.join(os.tmpdir(), Math.floor(Math.random() * 10000000).toString() + '_linter_tmp_file.ts');
}

function runIDEMode(cmdOptions: CommandLineOptions): void {
  TypeScriptLinter.ideMode = true;
  const tmpFileName = getTempFileName();
  // read data from stdin
  const writeStream = fs.createWriteStream(tmpFileName, { flags: 'w' });
  const rl = readline.createInterface({
    input: process.stdin,
    output: writeStream,
    terminal: false
  });

  rl.on('line', (line: string) => {
    fs.appendFileSync(tmpFileName, line + '\n');
  });
  rl.once('close', () => {
    // end of input
    writeStream.close();
    cmdOptions.inputFiles = [tmpFileName];
    if (cmdOptions.parsedConfigFile) {
      cmdOptions.parsedConfigFile.fileNames.push(tmpFileName);
    }
    const result = lint(compileLintOptions(cmdOptions));
    const problems = Array.from(result.problemsInfos.values());
    if (problems.length === 1) {
      const jsonMessage = problems[0].map((x) => {
        return {
          line: x.line,
          column: x.column,
          start: x.start,
          end: x.end,
          type: x.type,
          suggest: x.suggest,
          rule: x.rule,
          severity: x.severity,
          autofix: x.autofix
        };
      });
      Logger.info(`{"linter messages":${JSON.stringify(jsonMessage)}}`);
    } else {
      Logger.error('Unexpected error: could not lint file');
    }
    fs.unlinkSync(tmpFileName);
  });
}
