/*
 * Copyright (c) 2023 Huawei Device Co., Ltd.
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

class F {
  #f = 1;
  static #staticField = 2;
  #m() { }
  static #staticM() { }

  goodRhs(v: any) {
    const a = #f in v;

    const b = #f in v.p1.p2;

    const c = #f in (v as {});

    const d = #f in (v as F);

    const e = #f in (v as never);

    for (let f in #f in v as any) { /**/ } // unlikely but valid
  }
  whitespace(v: any) {
    const a = v && /*0*/#f/*1*/
      /*2*/ in/*3*/
          /*4*/v/*5*/
  }
}

class FooSub extends F { subTypeOfFoo = true }
class Bar { notFoo = true }
