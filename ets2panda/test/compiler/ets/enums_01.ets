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

enum E1 {
  e1_item1,
  e1_item2 = 7,
  e1_item3,
  e1_item4 = 1
}

//let cons = new Console;

function main() {

  assert(E1.e1_item1.toString() == "0");
  assert(E1.e1_item1.getValue() == 0);
  assert(E1.e1_item2.toString() == "7");
  assert(E1.e1_item2.getValue() == 7);
  assert(E1.e1_item3.toString() == "8");
  assert(E1.e1_item3.getValue() == 8);

 // cons.print("First element of 'E1': '" + E1.e1_item1.toString() + "' = " + E1.e1_item1.ordinal()  + 
 //   ", second: '" + E1.e1_item2.toString() + "' = " + E1.e1_item2.ordinal() + " and third: '" + 
 //   E1.e1_item3.toString() + "' = " + E1.e1_item3.ordinal() + ".\n")

  try {

    let test1 : E1 = E1.valueOf("e1_item1");
    let test2 : E1 = E1.valueOf("e1_item2");
    let test3 : E1 = E1.valueOf("e1_item3");

    assert(test1.getValue() == 0);
    assert(test1.getName() == "e1_item1");
    assert(test2.getValue() == 7);
    assert(test2.getName() == "e1_item2");
    assert(test3.getValue() == 8);
    assert(test3.getName() == "e1_item3");

    //cons.print("First element of 'E1': '" + test1.toString() + "' = " + test1.ordinal() +
    //  ", second: '" + test2.toString() + "' = " + test2.ordinal() + " and third: '"  + test3.toString() + "' = " + test3.ordinal()  + ".\n")
  }
  catch (e) {
    //cons.print("Exception happened")
  }
}
