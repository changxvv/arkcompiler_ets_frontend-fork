/*
* Copyright (c) 2022-2023 Huawei Device Co., Ltd.
* Licensed under the Apache License, Version 2.0 (the "License");
* you may not use this file except in compliance with the License.
* You may obtain a copy of the License at
*
* http://www.apache.org/licenses/LICENSE-2.0
*
* Unless required by applicable low or agreed to in writing, software
* distributed under the License is distributed on an "AS IS" BASIS,
* WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
* See the License for the specific language governing permissions and
* limitations under the License.
*/

export {}

@Component
struct Child {

  @State childState : string = "childState value";
  @BuilderParam aBuilderParam : BuilderType<{}>;

  @Builder childBuilder($ : { builderFunc: BuilderOneStringType }) {
    $.builderFunc({ param1: this.childState})    
  }

  build() {
    Column() {
      this.childBuilder({ builderFunc: this.aBuilderParam })
      Button("Mod this Child state")
      .height(75)
      .onClick(() => {
        this.childState = `C${this.childState}`;
      })
    }
    .height(3*50)
  }
} 



@Builder function GlobalBuilderAsParam($$ : { param1: string }) {
      Text( `GlobalBuilderAsParam builderParam ${$$.param1} .` )
        .width("100%")
        .height(60)
        .backgroundColor(Color.Green)
    }


@Entry
@Component
struct Parent {

  @State parentState : string = "parent state value";
  
  @Builder parentComponentBuilder($$ : { param1: string }) {
      Text( `ParentCompBuilder parentState: ${this.parentState} builderParam ${$$.param1} .` )
        .width("100%")
        .height(60)
        .backgroundColor(Color.Green)
    }

  build() {
    Column() {
      Text("Component Builder func param to Component Builder func")
      Child({ aBuilderParam: this.parentComponentBuilder})
      Button("Mod Parent state")
        .height(75)
        .onClick(() => {
          this.parentState = `P${this.parentState}`;
        })
      Divider().height(8)
      Text("Global builder func param to Component Builder func")
      Child({ aBuilderParam: GlobalBuilderAsParam })
    }
  }
}
