/*
 * Copyright (c) 2021 Huawei Device Co., Ltd.
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *     http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */

import {
    expect
} from 'chai';
import 'mocha';
import { DiagnosticCode, DiagnosticError } from '../../src/diagnostic';
import {
    EcmaReturnundefined,
    EcmaStconsttoglobalrecord,
    EcmaStglobalvar,
    EcmaStlettoglobalrecord,
    Imm,
    LdaDyn,
    LdaiDyn,
    ResultType,
    StaDyn,
    VReg
} from "../../src/irnodes";
import {
    FunctionScope,
    GlobalScope
} from "../../src/scope";
import {
    GlobalVariable,
    LocalVariable
} from "../../src/variable";
import { checkInstructions, SnippetCompiler } from "../utils/base";

describe("VariableDeclarationTest", function () {

    it('var i in the global scope', function () {
        let snippetCompiler = new SnippetCompiler();

        snippetCompiler.compile("var i;");
        let globalScope = <GlobalScope>snippetCompiler.getGlobalScope();
        let insns = snippetCompiler.getGlobalInsns();

        let expected = [
            new LdaDyn(new VReg()),
            new EcmaStglobalvar("i"),
            new EcmaReturnundefined()
        ];
        expect(checkInstructions(insns, expected)).to.be.true;
        let v = globalScope.findLocal("i");
        expect(v instanceof GlobalVariable).to.be.true;
    });

    it('let i in the global scope', function () {
        let snippetCompiler = new SnippetCompiler();
        snippetCompiler.compile("let i;");
        let globalScope = <GlobalScope>snippetCompiler.getGlobalScope();
        let insns = snippetCompiler.getGlobalInsns();
        let expected = [
            new LdaDyn(new VReg()),
            new EcmaStlettoglobalrecord('i'),
            new EcmaReturnundefined()
        ];
        expect(checkInstructions(insns, expected)).to.be.true;
        let v = globalScope.findLocal("i");
        expect(v instanceof LocalVariable).to.be.true;
    });

    it('const i in the global scope', function () {
        let snippetCompiler = new SnippetCompiler();
        snippetCompiler.compile("const i = 5;");
        let globalScope = <GlobalScope>snippetCompiler.getGlobalScope();
        let insns = snippetCompiler.getGlobalInsns();
        let expected = [
            new LdaiDyn(new Imm(5)),
            new EcmaStconsttoglobalrecord('i'),
            new EcmaReturnundefined()
        ];
        expect(checkInstructions(insns, expected)).to.be.true;
        let v = globalScope.findLocal("i");
        expect(v instanceof LocalVariable).to.be.true;
    });

    it('var i = 5 in the global scope', function () {
        let snippetCompiler = new SnippetCompiler();
        snippetCompiler.compile("var i = 5;");
        let globalScope = <GlobalScope>snippetCompiler.getGlobalScope();
        let insns = snippetCompiler.getGlobalInsns();
        let expected = [
            new LdaDyn(new VReg()),
            new EcmaStglobalvar("i"),
            new LdaiDyn(new Imm(5)),
            new EcmaStglobalvar("i"),
            new EcmaReturnundefined()
        ];
        expect(checkInstructions(insns, expected)).to.be.true;
        let v = globalScope.findLocal("i");
        expect(v instanceof GlobalVariable).to.be.true;
    });

    it('let i = 5 in the global scope', function () {
        let snippetCompiler = new SnippetCompiler();
        snippetCompiler.compile("let i = 5;");
        let globalScope = <GlobalScope>snippetCompiler.getGlobalScope();
        let insns = snippetCompiler.getGlobalInsns();
        let expected = [
            new LdaiDyn(new Imm(5)),
            new EcmaStlettoglobalrecord('i'),
            new EcmaReturnundefined()
        ];
        expect(checkInstructions(insns, expected)).to.be.true;
        let v = globalScope.findLocal("i");
        expect(v instanceof LocalVariable).to.be.true;
    });

    it('var i, j in the global scope', function () {
        let snippetCompiler = new SnippetCompiler();
        snippetCompiler.compile("var i, j;");
        let globalScope = <GlobalScope>snippetCompiler.getGlobalScope();
        let i = globalScope.findLocal("i");
        expect(i instanceof GlobalVariable).to.be.true;
        let j = globalScope.findLocal("j");
        expect(j instanceof GlobalVariable).to.be.true;
    });

    it('let i, j in the global scope', function () {
        let snippetCompiler = new SnippetCompiler();
        snippetCompiler.compile("let i, j;");
        let globalScope = <GlobalScope>snippetCompiler.getGlobalScope();
        let i = globalScope.findLocal("i");
        expect(i instanceof LocalVariable).to.be.true;
        let j = globalScope.findLocal("j");
        expect(j instanceof LocalVariable).to.be.true;
    });

    it('const i, j in the global scope', function () {
        let snippetCompiler = new SnippetCompiler();
        snippetCompiler.compile("const i=5, j=5;");
        let globalScope = <GlobalScope>snippetCompiler.getGlobalScope();
        let i = globalScope.findLocal("i");
        expect(i instanceof LocalVariable).to.be.true;
        let j = globalScope.findLocal("j");
        expect(j instanceof LocalVariable).to.be.true;
    });

    it('var i in a function scope', function () {
        let snippetCompiler = new SnippetCompiler();
        snippetCompiler.compile("function a() {var i;}");
        let funcPg = snippetCompiler.getPandaGenByName("a");
        let functionScope = <FunctionScope>funcPg!.getScope();
        let insns = funcPg!.getInsns();
        let expected = [
            new LdaDyn(new VReg()),
            new StaDyn(new VReg()),
            new EcmaReturnundefined()
        ];

        expect(checkInstructions(insns, expected)).to.be.true;
        let i = functionScope.findLocal("i");
        expect(i).to.not.be.equal(undefined);
        expect(i instanceof LocalVariable).to.be.true;
    });

    it('let i in a function scope', function () {
        let snippetCompiler = new SnippetCompiler();
        snippetCompiler.compile("function a() {let i;}");
        let funcPg = snippetCompiler.getPandaGenByName("a");
        let functionScope = <FunctionScope>funcPg!.getScope();
        let insns = funcPg!.getInsns();
        let expected = [
            new LdaDyn(new VReg()),
            new StaDyn(new VReg()),
            new EcmaReturnundefined()
        ];
        expect(checkInstructions(insns, expected)).to.be.true;
        let i = functionScope.findLocal("i");
        expect(i).to.be.equal(undefined);
    });

    it('const i in a function scope', function () {
        let snippetCompiler = new SnippetCompiler();
        snippetCompiler.compile("function a() {const i = 5;}");
        let funcPg = snippetCompiler.getPandaGenByName("a");
        let functionScope = <FunctionScope>funcPg!.getScope();
        let insns = funcPg!.getInsns();
        let expected = [
            new LdaiDyn(new Imm(5)),
            new StaDyn(new VReg()),
            new EcmaReturnundefined()
        ];
        expect(checkInstructions(insns, expected)).to.be.true;
        let i = functionScope.findLocal("i");
        expect(i).to.be.equal(undefined);
    });

    it('let i in a local scope', function () {
        let snippetCompiler = new SnippetCompiler();
        snippetCompiler.compile("{let i;}");
        let funcPg = snippetCompiler.getPandaGenByName("func_main_0");
        let localScope = funcPg!.getScope();
        let insns = funcPg!.getInsns();

        let expected = [
            new LdaDyn(new VReg()),
            new StaDyn(new VReg()),
            new EcmaReturnundefined()
        ];
        expect(checkInstructions(insns, expected)).to.be.true;
        let i = localScope!.findLocal("i");
        expect(i).to.be.equal(undefined);
    });

    it('let declaration syntax error', function () {
        let errorThrown = false;
        let snippetCompiler = new SnippetCompiler();
        try {
            snippetCompiler.compile("label: let i = 5;");
        } catch (err) {
            expect(err instanceof DiagnosticError).to.be.true;
            expect((<DiagnosticError>err).code).to.equal(DiagnosticCode.Lexical_declaration_let_not_allowed_in_statement_position);
            errorThrown = true;
        }
        expect(errorThrown).to.be.true;
    });

    it('const i in a local scope', function () {
        let snippetCompiler = new SnippetCompiler();
        snippetCompiler.compile("{const i = 5;}");
        let insns = snippetCompiler.getGlobalInsns();
        let scope = snippetCompiler.getGlobalScope();
        let expected = [
            new LdaiDyn(new Imm(5)),
            new StaDyn(new VReg()),
            new EcmaReturnundefined()
        ];
        expect(checkInstructions(insns, expected)).to.be.true;
        let i = scope!.findLocal("i");
        expect(i == undefined).to.be.true; // not in global
    });
});
