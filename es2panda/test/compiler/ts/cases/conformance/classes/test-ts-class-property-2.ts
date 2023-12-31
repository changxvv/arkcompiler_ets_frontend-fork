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


class A {
  zero : number = 0
  ten : number = 10
  "1" : number = 1
  "-1" : number = -1
  0 : number = 0
  readonly [-2] : number = -2
}

let a : A = new A()
print(a["ten"])
print(a["1"])
print(a[+1])
print(a["-1"])
print(a[0])
print(a[-2])
