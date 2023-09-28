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

interface E1 extends Error {}
interface E2 extends E1 {}

class A1 implements E1 {
  name: string = "";
  message: string = "";
}
class A11 extends A1 {}

class A2 implements E2 {
  name: string = "";
  message: string = "";
}
class A22 extends A2 {}

class A {}

function foo() {
  const a = 1;

  throw new A1();
  throw new A11();

  throw new A2();
  throw new A22();

  throw "Error";
  throw "Error";
  throw `Error${a}`;
}
