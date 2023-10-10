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

class C {
    firstName: string;
    lastName: string;
    constructor(firstName: string, lastName: string) {
        this.firstName = firstName;
        this.lastName = lastName;
    }

    fullName(): string { return this.firstName + " " + this.lastName; }
}

function foo(): void {
    let person1 = new C("Arthur", "Clarke");
    let person2 = new C("Ray", "Bradbury");

    let fullName = person1.fullName.apply(person2);

    let f = person2.fullName.bind(person1);
    f();

    bar.call(undefined, person1);
}

function bar(c: C): string {
    let person = new C("Stanislaw", "Lem");
    return c.fullName.call(person);
}
