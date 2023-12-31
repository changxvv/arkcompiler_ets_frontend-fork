/*
 * Copyright (c) 2022 Huawei Device Co., Ltd.
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


namespace E {
    export enum E {
        A = 0,
        E = 1,
        E_2 = 2,
        B = (() => {enum E{
                            C = A + 1,
                            E = 3,
                            E_1 = 5,
                            E_2 = E + 10,
                            E_3 = 1 && 2,
                            G = E_3 + 2
                        } return E.C + 5;})(),
        G = A + 10
    }
}

print(E.E.E);
print(E.E.E_2);
print(E.E.B);
print(E.E.G);
print(E.E[6]);