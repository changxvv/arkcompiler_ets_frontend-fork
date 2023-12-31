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

/// Global
eval('console.log("foo")');
let inf = Infinity;
let nan = NaN;
isFinite(1);
isNaN(2);
parseFloat('3');
parseInt('4', 10);
encodeURI('');
encodeURIComponent('');
decodeURI('');
decodeURIComponent('');
escape('');
unescape('');

// global and window are not portabe, so they are excluded from test suite
globalThis.eval('console.log("foo")');

class C {}
let c = new C();

/// Object
Object.assign<C, C>(c, c);
Object.create(c);
Object.defineProperties<C>(c, {});
Object.defineProperty<C>(c, 'p', c);
Object.freeze(() => {});
Object.fromEntries<number>([]);
Object.getOwnPropertyDescriptor(c, 'p');
Object.getOwnPropertyDescriptors<C>(c);
Object.getOwnPropertySymbols(c);
Object.getPrototypeOf(c);
Object.hasOwnProperty('p');
Object.is(c, c);
Object.isExtensible(c);
Object.isFrozen(c);
Object.isPrototypeOf(c);
Object.isSealed(c);
Object.preventExtensions<C>(c);
Object.propertyIsEnumerable('p');
Object.seal<C>(c);
Object.setPrototypeOf(c, c);

/// Reflect
Reflect.apply<C, number[], void>(() => {}, c, []);
Reflect.construct<number[], C>(C, []);
Reflect.defineProperty(c, 'p', {});
Reflect.deleteProperty(c, 'p');
Reflect.getOwnPropertyDescriptor<C, string>(c, 'p');
Reflect.getPrototypeOf(c);
Reflect.isExtensible(c);
Reflect.preventExtensions(c);
Reflect.setPrototypeOf(c, c);

/// Proxy
let handler: ProxyHandler<C> = {};
if (handler.apply) handler.apply(c, c, []);
if (handler.construct) handler.construct(c, [], () => {});
if (handler.defineProperty) handler.defineProperty(c, "prop", {});
if (handler.deleteProperty) handler.deleteProperty(c, "prop");
if (handler.get) handler.get(c, "prop", {});
if (handler.getOwnPropertyDescriptor) handler.getOwnPropertyDescriptor(c, "prop");
if (handler.getPrototypeOf) handler.getPrototypeOf(c);
if (handler.has) handler.has(c, "prop");
if (handler.isExtensible) handler.isExtensible(c);
if (handler.ownKeys) handler.ownKeys(c);
if (handler.preventExtensions) handler.preventExtensions(c);
if (handler.set) handler.set(c, "prop", 1, c);
if (handler.setPrototypeOf) handler.setPrototypeOf(c, null);

/// Array
ArrayBuffer.isView({});

Number.NaN;
Number.isFinite(1);
Number.isNaN(2);
Number.parseFloat('3');
Number.parseInt('4', 10);
