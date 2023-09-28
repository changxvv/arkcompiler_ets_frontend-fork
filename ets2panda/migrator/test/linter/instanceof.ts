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

class X {}

let a = (new X()) instanceof Object // true
let b = (new X()) instanceof X // true

// left operand is a type:
let c = X instanceof Object // Compile-time error
let d = X instanceof X // Compile-time error

// left operand may be of any reference type, like number
let e = (5.0 as Number) instanceof Number // false

let f = (X | String) instanceof Object;

let g = 3 instanceof Number;