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

package com.ohos.migrator.tests.java;

interface test_interface {
    int i = 10;
    double pi = 3.1416;

    void foo();

    static void foo(int i) {}

    private void foo(boolean b) {}

    public void foo(String s);
}

strictfp interface iface_FP {
    void foo(float f);
}


interface iface_C extends test_interface, iface_FP {
    public void foo(double d);

    { foo(2.5); }
}
