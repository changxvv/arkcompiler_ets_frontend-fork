/*
 * Copyright (c) 2022-2024 Huawei Device Co., Ltd.
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

import { ff } from "./oh_modules/ohos_lib";
import { f, f2, bar, applyToUnknown, fooExecute, C1, resourceFoo, ResourceStr, myFoo, myFoo2 } from "./dynamic_lib";

let a1: C1 = new C1()

function g1(): C1 | undefined {
    if (a1) {
        return a1
    }
    return undefined
}

f2(g1())

bar(undefined);
bar(undefined, undefined);
applyToUnknown(undefined);
fooExecute(undefined);

function fff(a: Array<number>): void {}

fff(undefined);
ff(undefined);
f(undefined);

function resourceFoo2(a: ResourceStr) { return "0" }

class A {
    buttonTitle1?: string;
    buttonTitle2?: string;
    buttonTitle3?: string;

    constructor(menu?: Partial<A>) {
        this.buttonTitle1 = resourceFoo(menu?.buttonTitle1);
        this.buttonTitle2 = resourceFoo(menu?.buttonTitle2);
        this.buttonTitle3 = resourceFoo2(menu?.buttonTitle3);
    }
}

myFoo({x: "", y: undefined})

class A2 {
    public a: Object = undefined
    f() {
        myFoo2(
            () => {
                this.a = undefined;
            }
        )
    }
}
