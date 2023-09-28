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

export function fun1(): void {
    let a = 10;
    let b = true;

    if (b) {}

    if (b) a++;

    if (b) {
        a--;
    }

    if (b)
        a -= 1;
    else
        a += 1;

    if (b) {
        a = 10;
    } else a = 20;

    if (b) a = 30;
    else {
        a = 40;
    }

    if (a > 10) {
        a = 100;
    } else if (a < 0) {
        a = 200;
    } else {
        a = 0;
    }

    if (b)
        if (a > 100)
            a += 25;
        else if (a < 0) 
            a -= 50;
        else
            a *= 10;
    else
        a /= 5;
}
