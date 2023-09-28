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

import * as a from "dummy";
import type * as b from "foo";

import { x, y, default as z } from "bar";
import { x, type y } from "foobar";
import { x, y as z } from "barfoo";
import { type x as y, z } from "goo";
import { default as d } from "goobar";

import x from "bargoo";

import "foogoo";
