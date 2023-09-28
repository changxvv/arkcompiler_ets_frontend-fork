export declare interface I {
    f1: Object;
    f2: Array<Object>;
    [key: string]: Object;
}

export declare function foo(p: Object): void

export declare interface I2 {
    f1?: Object;
    f2?: Array<Object>;
    f3?: any;
}

export declare interface I3 {
    f1: object;
    f2?: object;
    f3: Array<object>;
}

export declare class TaskGroup {}
export declare enum Priority { HIGH, MEDIUM, LOW }

export declare function bar(p: object, q?: object): void;
export declare function applyToUnknown(fn: (a: unknown) => void);
export declare function fooExecute(func: Function, ...args: unknown[]): Promise<unknown>;
export declare function fooExecute(group: TaskGroup, priority?: Priority): Promise<unknown[]>;

export declare class C {
    a?: any;
    b?: {
        [key: string]: Object;
    };
}
export declare function getDynamicObject(): {
    [key: string]: Object
};
export declare function f(a: number): void;

export class DynLibC {}
export interface DynLibI {}
export interface DynLibIC extends DynLibC {}
export interface DynLibII extends DynLibI {}
export class DynLibCC extends DynLibC {}
export class DynLibCI implements DynLibI {}

export let dynamic_array: Array<any>;
export declare class C1 {}

export declare function f2(c: C1): void;

export type Length = string | number | Resource;
export interface Resource {
  readonly id: number;
  readonly type: number;
}
export type Padding = {
  top?: Length;
  right?: Length;
  bottom?: Length;
  left?: Length;
}
export type Margin = Padding;
export interface Position {
    x?: Length;
    y?: Length;
}

export function padding(value: Padding | Length): any;
export function margin(value: Margin | Length): any;
export function position(value: Position): any;

export declare interface Resource {}
export declare type ResourceStr = string | Resource;

export declare function resourceFoo(p: ResourceStr): string;