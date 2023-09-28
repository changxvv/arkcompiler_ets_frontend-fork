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

interface Test {
    void Outtermethod();

    public class A {
	    int i;
    }

    abstract interface dummy {
    	void getI();
    }


    interface innerInterface {
        public int Innermethod();
    }
 
    public interface inner_A {
    	 double coeff();
    }
}


class TestClass implements Test.innerInterface, Test.inner_A {
    @Override
    public int Innermethod() {
        return 37037;
    }

    @Override
    public double coeff() {
	    return 3.14;
    }

    public interface constants {
    	public double Pi();
	static double E() { return 2.7828; };
    }

}


