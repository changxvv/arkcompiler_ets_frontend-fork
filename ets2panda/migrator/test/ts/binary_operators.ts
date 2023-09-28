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
    let a = 0;
    let b = true;

    // Arithmetic
    a = 1 + 2;
    a = 2 - 3;
    a = 3 * 2;
    a = 4 / 3;
    a = 5 % 2;
    a = 2 ** 3;
    a = 10 / (3 + 4) * 2;

    // Bitwise
    a = 5 & 6;
    a = 10 | 4;
    a = 15 ^ 5;
    a = a << 2;
    a = a >> 3;
    a = a >>> 4;

    // Comparison
    b = a == 1;
    b = a != 2;
    b = a === 3;
    b = a !== 4;
    b = a > 5;
    b = a < 6;
    b = a >= 7;
    b = a <= 8;

    // Conditional
    b = a > 0 && a < 10;
    b = a >= 0 || a <= 10;
}