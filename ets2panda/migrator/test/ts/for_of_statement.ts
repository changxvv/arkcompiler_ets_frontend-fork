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

interface I {
    a: number,
    b: string
}

export function fun(): void {
    let sum = 0;
    for (let x of [1, 2, 3])
        sum += x; 

    const iterable = new Map([
        ["a", 1],
        ["b", 2],
        ["c", 3],
    ]);
    for (const entry of iterable) {
        console.log(entry);
    }

    let n: number;
    let product = 1;
    for (n of [1, 2, 3, 4, 5]) {
        product *= n;
    }

    let objects: I[] = [
        {a: 10, b: "q"},
        {a: 20, b: "w"},
        {a: 30, b: "e"}
    ];
    for (let {a, b} of objects) {
        console.log(a + " " + b);
    }

    let x: number, y: number;
    let arrays = [[1, 2], [3, 4], [5, 6]]
    for ([x, y] of arrays) {
        console.log(x + " " + y);
    }
}
