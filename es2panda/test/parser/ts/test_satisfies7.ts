/*
 * Copyright (c) 2023 Huawei Device Co., Ltd.
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


type Keys = 'a' | 'b' | 'c' | 'd';

const p = {
    a: 0,
    b: "hello",
    x: 8
} satisfies Partial<Record<Keys, unknown>>;

let a = p.a.toFixed();
let b = p.b.substring(1);
let d = p.d;