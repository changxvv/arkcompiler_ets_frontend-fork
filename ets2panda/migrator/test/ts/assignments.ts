/*
 * Copyright (c) 2022-2022 Huawei Device Co., Ltd.
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

function foo(): void {
    let a: number;
    a = 0;
    a += 1;
    a -= 2;
    a *= 3;
    a /= 4;
    a %= 5;
    a **= 6;
    a &= 7;
    a |= 8;
    a ^= 9;
    a <<= 10;
    a >>= 11;
    a >>>= 12;
    a &&= 13;
    a ||= 14;
    a ??= 15;

    let b: number;
    b = a = 1000;
}
