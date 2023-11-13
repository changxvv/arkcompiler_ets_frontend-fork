/**
 * Copyright (c) 2021-2022 Huawei Device Co., Ltd.
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

#ifndef ES2PANDA_PARSER_INCLUDE_TOKEN_TYPE_H
#define ES2PANDA_PARSER_INCLUDE_TOKEN_TYPE_H

namespace panda::es2panda::lexer {
enum class TokenType {
    EOS,

    LITERAL_IDENT,
    LITERAL_STRING,
    LITERAL_CHAR,
    LITERAL_NUMBER,
    LITERAL_REGEXP,
    LITERAL_TRUE,
    LITERAL_FALSE,
    LITERAL_NULL,

    // Keep this order
    PUNCTUATOR_NULLISH_COALESCING,
    PUNCTUATOR_LOGICAL_OR,
    PUNCTUATOR_LOGICAL_AND,
    PUNCTUATOR_BITWISE_OR,
    PUNCTUATOR_BITWISE_XOR,
    PUNCTUATOR_BITWISE_AND,
    PUNCTUATOR_EQUAL,
    PUNCTUATOR_NOT_EQUAL,
    PUNCTUATOR_STRICT_EQUAL,
    PUNCTUATOR_NOT_STRICT_EQUAL,
    PUNCTUATOR_LESS_THAN,
    PUNCTUATOR_LESS_THAN_EQUAL,
    PUNCTUATOR_GREATER_THAN,
    PUNCTUATOR_GREATER_THAN_EQUAL,
    PUNCTUATOR_LEFT_SHIFT,
    PUNCTUATOR_RIGHT_SHIFT,
    PUNCTUATOR_UNSIGNED_RIGHT_SHIFT,
    PUNCTUATOR_PLUS,
    PUNCTUATOR_MINUS,
    PUNCTUATOR_MULTIPLY,
    PUNCTUATOR_DIVIDE,
    PUNCTUATOR_MOD,
    KEYW_IN,
    KEYW_INSTANCEOF,
    PUNCTUATOR_EXPONENTIATION,

    PUNCTUATOR_LEFT_PARENTHESIS,
    PUNCTUATOR_RIGHT_PARENTHESIS,
    PUNCTUATOR_LEFT_BRACE,
    PUNCTUATOR_RIGHT_BRACE,
    PUNCTUATOR_PERIOD,
    PUNCTUATOR_PERIOD_PERIOD_PERIOD,
    PUNCTUATOR_PERIOD_QUESTION,
    PUNCTUATOR_COMMA,
    PUNCTUATOR_COLON,
    PUNCTUATOR_SEMI_COLON,
    PUNCTUATOR_LEFT_SQUARE_BRACKET,
    PUNCTUATOR_RIGHT_SQUARE_BRACKET,
    PUNCTUATOR_QUESTION_MARK,
    PUNCTUATOR_QUESTION_DOT,
    PUNCTUATOR_TILDE,
    PUNCTUATOR_EXCLAMATION_MARK,
    PUNCTUATOR_PLUS_PLUS,
    PUNCTUATOR_MINUS_MINUS,
    PUNCTUATOR_SUBSTITUTION,
    PUNCTUATOR_UNSIGNED_RIGHT_SHIFT_EQUAL,
    PUNCTUATOR_RIGHT_SHIFT_EQUAL,
    PUNCTUATOR_LEFT_SHIFT_EQUAL,
    PUNCTUATOR_PLUS_EQUAL,
    PUNCTUATOR_MINUS_EQUAL,
    PUNCTUATOR_MULTIPLY_EQUAL,
    PUNCTUATOR_DIVIDE_EQUAL,
    PUNCTUATOR_MOD_EQUAL,
    PUNCTUATOR_BITWISE_AND_EQUAL,
    PUNCTUATOR_BITWISE_OR_EQUAL,
    PUNCTUATOR_BITWISE_XOR_EQUAL,
    PUNCTUATOR_LOGICAL_AND_EQUAL,
    PUNCTUATOR_LOGICAL_OR_EQUAL,
    PUNCTUATOR_LOGICAL_NULLISH_EQUAL,
    PUNCTUATOR_EXPONENTIATION_EQUAL,
    PUNCTUATOR_ARROW,
    PUNCTUATOR_BACK_TICK,
    PUNCTUATOR_HASH_MARK,
    PUNCTUATOR_AT,
    PUNCTUATOR_DOLLAR_DOLLAR,

    /* contextual keywords */
    KEYW_GET,
    KEYW_SET,
    KEYW_OF,
    KEYW_FROM,
    KEYW_AS,
    KEYW_META,
    KEYW_REQUIRE,
    KEYW_ABSTRACT,
    KEYW_TARGET,
    KEYW_OUT,

    /* reserved keywords */
    FIRST_KEYW,
    KEYW_ANY = FIRST_KEYW,
    KEYW_AWAIT,
    KEYW_BIGINT,
    KEYW_BOOLEAN,
    KEYW_BYTE,
    KEYW_BREAK,
    KEYW_CASE,
    KEYW_CATCH,
    KEYW_CHAR,
    KEYW_CLASS,
    KEYW_CONST,
    KEYW_CONSTRUCTOR,
    KEYW_CONTINUE,
    KEYW_DEBUGGER,
    KEYW_DEFAULT,
    KEYW_DELETE,
    KEYW_DO,
    KEYW_DOUBLE,
    KEYW_ELSE,
    KEYW_ENUM,
    KEYW_EXPORT,
    KEYW_EXTENDS,
    KEYW_FINALLY,
    KEYW_FLOAT,
    KEYW_FOR,
    KEYW_FUNCTION,
    KEYW_IF,
    KEYW_IMPORT,
    KEYW_INT,
    KEYW_LAUNCH,
    KEYW_LONG,
    KEYW_NATIVE,
    KEYW_NEVER,
    KEYW_NEW,
    KEYW_NUMBER,
    KEYW_OBJECT,
    KEYW_FINAL,
    KEYW_OVERRIDE,
    KEYW_REGEXP,
    KEYW_RETHROWS,
    KEYW_RETURN,
    KEYW_SHORT,
    KEYW_STRING,
    KEYW_STRUCT,
    KEYW_SUPER,
    KEYW_SWITCH,
    KEYW_THIS,
    KEYW_THROW,
    KEYW_THROWS,
    KEYW_TYPE,
    KEYW_TRY,
    KEYW_TYPEOF,
    KEYW_UNDEFINED,
    KEYW_UNKNOWN,
    KEYW_VAR,
    KEYW_VOID,
    KEYW_WHILE,
    KEYW_WITH,
    KEYW_I8,
    KEYW_I16,
    KEYW_I32,
    KEYW_I64,
    KEYW_ISIZE,
    KEYW_U8,
    KEYW_U16,
    KEYW_U32,
    KEYW_U64,
    KEYW_USIZE,
    KEYW_F32,
    KEYW_F64,
    KEYW_V128,
    KEYW_FUNCREF,
    KEYW_EXTERNREF,
    KEYW_ANYREF,
    KEYW_EQREF,
    KEYW_I31REF,
    KEYW_DATAREF,

    KEYW_ASYNC,
    KEYW_READONLY,
    KEYW_KEYOF,
    KEYW_MODULE,
    KEYW_NAMESPACE,
    KEYW_INFER,
    KEYW_DECLARE,
    KEYW_ARGUMENTS,
    KEYW_EVAL,
    KEYW_STATIC,
    KEYW_GLOBAL,
    KEYW_IS,
    KEYW_ASSERTS,
    KEYW_ASSERT,
    KEYW_TRAP,

    /* strict mode future reserved keywords */
    KEYW_PRIVATE,
    KEYW_PROTECTED,
    KEYW_PUBLIC,
    KEYW_INTERNAL,
    KEYW_IMPLEMENTS,
    KEYW_INTERFACE,
    KEYW_PACKAGE,

    /* context dependent future */
    KEYW_LET,
    KEYW_YIELD,
};

const char *TokenToString(TokenType type);
}  // namespace panda::es2panda::lexer

#endif
