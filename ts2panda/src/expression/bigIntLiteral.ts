/*
 * Copyright (c) 2022 Huawei Device Co., Ltd.
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

import * as ts from "typescript";
import * as jshelpers from "../jshelpers";
import { PandaGen } from "../pandagen";

export function compileBigIntLiteral(pandaGen: PandaGen, lit: ts.BigIntLiteral): void {
    let text = jshelpers.getTextOfIdentifierOrLiteral(lit);
    text = text.substring(0, text.length-1);
    pandaGen.loadAccumulatorBigInt(lit, text);
}