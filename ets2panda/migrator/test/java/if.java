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

package com.ohos.migrator.test.java;

class IfTest {
        
    public void Test() {
	boolean t = true;
	int a, b;
	
	if( t )	a = 1;
	
	if( t ) a = 2; else a = 3; 

	if( t ) {
	    a = 4; b = 100;
	}


	if( t ) {
		a = 5; b = 101;
	} else {
		a = 6; b = 102;
	}

	if( t ) a = 7; else { a = 8; b = 103; }

	if( t ) { a = 9; b = 104; } else a = 10;

	boolean p = false;

	if( t ) if ( p ) a = 11; else a = 12;

	if( t ) a = 13; else if ( p ) { a= 14; }

	if( t ) a = 15; else { if( p ) a = 16; else a = 17; }

	if( t ) { if( p ) { a = 18; b = 105; } else { a = 19; b = 106; } } else if (p) a = 20; else a = 21;
    }
}
