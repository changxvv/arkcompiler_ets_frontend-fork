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

import { obj } from "something.json" assert { type: "json" };

function foo(): void {
    const obj1 = import("something.json", { assert: { type: "json" } });
    const obj2 = await import("something.json", { "assert": { type: "json" } });

    let assert = { type: "json" };
    const obj3 = import("something.json", { assert });
}
