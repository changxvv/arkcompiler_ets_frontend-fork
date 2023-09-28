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

function fun(): void {
    let empty = ``;
    let template_text = `First line \n second line
third line \n fourth line`;
    let template_head = `template-head ${20 + 40}`;
    let template_tail = `${empty.length > 0 ? 100 : 200} template-tail`;
    let template_compound = `apple ${400} orange \n ${500} 
"banana" ${600} 'grapes'`;
    let nested = `outer ${`inner ${1000} inner-end`} outer-end`
    
    // Templates with single expression and no strings:
    let x = 2500;
    let y = "hello";
    let num = `${10}`;
    let num2 = `${x}`;
    let str = `${"world"}`;
    let str2 = `${y}`;
}