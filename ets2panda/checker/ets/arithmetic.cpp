/*
 * Copyright (c) 2021 - 2023 Huawei Device Co., Ltd.
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

#include "arithmetic.h"

#include "ir/expressions/identifier.h"
#include "varbinder/variable.h"
#include "varbinder/scope.h"
#include "varbinder/declaration.h"
#include "checker/ETSchecker.h"

namespace panda::es2panda::checker {

Type *ETSChecker::NegateNumericType(Type *type, ir::Expression *node)
{
    ASSERT(type->HasTypeFlag(TypeFlag::CONSTANT | TypeFlag::ETS_NUMERIC));

    TypeFlag typeKind = ETSType(type);
    Type *result = nullptr;

    switch (typeKind) {
        case TypeFlag::BYTE: {
            result = CreateByteType(-(type->AsByteType()->GetValue()));
            break;
        }
        case TypeFlag::CHAR: {
            result = CreateCharType(-(type->AsCharType()->GetValue()));
            break;
        }
        case TypeFlag::SHORT: {
            result = CreateShortType(-(type->AsShortType()->GetValue()));
            break;
        }
        case TypeFlag::INT: {
            result = CreateIntType(-(type->AsIntType()->GetValue()));
            break;
        }
        case TypeFlag::LONG: {
            result = CreateLongType(-(type->AsLongType()->GetValue()));
            break;
        }
        case TypeFlag::FLOAT: {
            result = CreateFloatType(-(type->AsFloatType()->GetValue()));
            break;
        }
        case TypeFlag::DOUBLE: {
            result = CreateDoubleType(-(type->AsDoubleType()->GetValue()));
            break;
        }
        default: {
            UNREACHABLE();
        }
    }

    node->SetTsType(result);
    return result;
}

Type *ETSChecker::BitwiseNegateNumericType(Type *type, ir::Expression *node)
{
    ASSERT(type->HasTypeFlag(TypeFlag::CONSTANT | TypeFlag::ETS_INTEGRAL));

    TypeFlag typeKind = ETSType(type);

    Type *result = nullptr;

    switch (typeKind) {
        case TypeFlag::BYTE: {
            result = CreateByteType(static_cast<int8_t>(~static_cast<uint8_t>(type->AsByteType()->GetValue())));
            break;
        }
        case TypeFlag::CHAR: {
            result = CreateCharType(~(type->AsCharType()->GetValue()));
            break;
        }
        case TypeFlag::SHORT: {
            result = CreateShortType(static_cast<int16_t>(~static_cast<uint16_t>(type->AsShortType()->GetValue())));
            break;
        }
        case TypeFlag::INT: {
            result = CreateIntType(static_cast<int32_t>(~static_cast<uint32_t>(type->AsIntType()->GetValue())));
            break;
        }
        case TypeFlag::LONG: {
            result = CreateLongType(static_cast<int64_t>(~static_cast<uint64_t>(type->AsLongType()->GetValue())));
            break;
        }
        case TypeFlag::FLOAT: {
            result = CreateIntType(
                ~static_cast<uint32_t>(CastFloatToInt<FloatType::UType, int32_t>(type->AsFloatType()->GetValue())));
            break;
        }
        case TypeFlag::DOUBLE: {
            result = CreateLongType(
                ~static_cast<uint64_t>(CastFloatToInt<DoubleType::UType, int64_t>(type->AsDoubleType()->GetValue())));
            break;
        }
        default: {
            UNREACHABLE();
        }
    }

    node->SetTsType(result);
    return result;
}

Type *ETSChecker::HandleRelationOperationOnTypes(Type *left, Type *right, lexer::TokenType operationType)
{
    ASSERT(left->HasTypeFlag(TypeFlag::CONSTANT | TypeFlag::ETS_NUMERIC) &&
           right->HasTypeFlag(TypeFlag::CONSTANT | TypeFlag::ETS_NUMERIC));

    if (left->IsDoubleType() || right->IsDoubleType()) {
        return PerformRelationOperationOnTypes<DoubleType>(left, right, operationType);
    }

    if (left->IsFloatType() || right->IsFloatType()) {
        return PerformRelationOperationOnTypes<FloatType>(left, right, operationType);
    }

    if (left->IsLongType() || right->IsLongType()) {
        return PerformRelationOperationOnTypes<LongType>(left, right, operationType);
    }

    return PerformRelationOperationOnTypes<IntType>(left, right, operationType);
}

checker::Type *ETSChecker::CheckBinaryOperatorMulDivMod(ir::Expression *left, ir::Expression *right,
                                                        lexer::TokenType operationType, lexer::SourcePosition pos,
                                                        bool isEqualOp, checker::Type *const leftType,
                                                        checker::Type *const rightType, Type *unboxedL, Type *unboxedR)
{
    checker::Type *tsType {};
    auto [promotedType, bothConst] =
        ApplyBinaryOperatorPromotion(unboxedL, unboxedR, TypeFlag::ETS_NUMERIC, !isEqualOp);

    FlagExpressionWithUnboxing(leftType, unboxedL, left);
    FlagExpressionWithUnboxing(rightType, unboxedR, right);

    if (leftType->IsETSUnionType() || rightType->IsETSUnionType()) {
        ThrowTypeError("Bad operand type, unions are not allowed in binary expressions except equality.", pos);
    }

    if (promotedType == nullptr && !bothConst) {
        ThrowTypeError("Bad operand type, the types of the operands must be numeric type.", pos);
    }

    if (bothConst) {
        tsType = HandleArithmeticOperationOnTypes(leftType, rightType, operationType);
    }

    tsType = (tsType != nullptr) ? tsType : promotedType;
    return tsType;
}

checker::Type *ETSChecker::CheckBinaryOperatorPlus(ir::Expression *left, ir::Expression *right,
                                                   lexer::TokenType operationType, lexer::SourcePosition pos,
                                                   bool isEqualOp, checker::Type *const leftType,
                                                   checker::Type *const rightType, Type *unboxedL, Type *unboxedR)
{
    if (leftType->IsETSUnionType() || rightType->IsETSUnionType()) {
        ThrowTypeError("Bad operand type, unions are not allowed in binary expressions except equality.", pos);
    }

    if (leftType->IsETSStringType() || rightType->IsETSStringType()) {
        return HandleStringConcatenation(leftType, rightType);
    }

    auto [promotedType, bothConst] =
        ApplyBinaryOperatorPromotion(unboxedL, unboxedR, TypeFlag::ETS_NUMERIC, !isEqualOp);

    FlagExpressionWithUnboxing(leftType, unboxedL, left);
    FlagExpressionWithUnboxing(rightType, unboxedR, right);

    if (promotedType == nullptr && !bothConst) {
        ThrowTypeError("Bad operand type, the types of the operands must be numeric type or String.", pos);
    }

    if (bothConst) {
        return HandleArithmeticOperationOnTypes(leftType, rightType, operationType);
    }

    return promotedType;
}

checker::Type *ETSChecker::CheckBinaryOperatorShift(ir::Expression *left, ir::Expression *right,
                                                    lexer::TokenType operationType, lexer::SourcePosition pos,
                                                    bool isEqualOp, checker::Type *const leftType,
                                                    checker::Type *const rightType, Type *unboxedL, Type *unboxedR)
{
    if (leftType->IsETSUnionType() || rightType->IsETSUnionType()) {
        ThrowTypeError("Bad operand type, unions are not allowed in binary expressions except equality.", pos);
    }

    auto promotedLeftType = ApplyUnaryOperatorPromotion(unboxedL, false, !isEqualOp);
    auto promotedRightType = ApplyUnaryOperatorPromotion(unboxedR, false, !isEqualOp);

    FlagExpressionWithUnboxing(leftType, unboxedL, left);
    FlagExpressionWithUnboxing(rightType, unboxedR, right);

    if (promotedLeftType == nullptr || !promotedLeftType->HasTypeFlag(checker::TypeFlag::ETS_NUMERIC) ||
        promotedRightType == nullptr || !promotedRightType->HasTypeFlag(checker::TypeFlag::ETS_NUMERIC)) {
        ThrowTypeError("Bad operand type, the types of the operands must be numeric type.", pos);
    }

    if (promotedLeftType->HasTypeFlag(TypeFlag::CONSTANT) && promotedRightType->HasTypeFlag(TypeFlag::CONSTANT)) {
        return HandleBitwiseOperationOnTypes(promotedLeftType, promotedRightType, operationType);
    }

    switch (ETSType(promotedLeftType)) {
        case TypeFlag::BYTE: {
            return GlobalByteType();
        }
        case TypeFlag::SHORT: {
            return GlobalShortType();
        }
        case TypeFlag::CHAR: {
            return GlobalCharType();
        }
        case TypeFlag::INT:
        case TypeFlag::FLOAT: {
            return GlobalIntType();
        }
        case TypeFlag::LONG:
        case TypeFlag::DOUBLE: {
            return GlobalLongType();
        }
        default: {
            UNREACHABLE();
        }
    }
    return nullptr;
}

checker::Type *ETSChecker::CheckBinaryOperatorBitwise(ir::Expression *left, ir::Expression *right,
                                                      lexer::TokenType operationType, lexer::SourcePosition pos,
                                                      bool isEqualOp, checker::Type *const leftType,
                                                      checker::Type *const rightType, Type *unboxedL, Type *unboxedR)
{
    if (leftType->IsETSUnionType() || rightType->IsETSUnionType()) {
        ThrowTypeError("Bad operand type, unions are not allowed in binary expressions except equality.", pos);
    }

    if (unboxedL != nullptr && unboxedL->HasTypeFlag(checker::TypeFlag::ETS_BOOLEAN) && unboxedR != nullptr &&
        unboxedR->HasTypeFlag(checker::TypeFlag::ETS_BOOLEAN)) {
        FlagExpressionWithUnboxing(leftType, unboxedL, left);
        FlagExpressionWithUnboxing(rightType, unboxedR, right);
        return HandleBooleanLogicalOperators(unboxedL, unboxedR, operationType);
    }

    auto [promotedType, bothConst] =
        ApplyBinaryOperatorPromotion(unboxedL, unboxedR, TypeFlag::ETS_NUMERIC, !isEqualOp);

    FlagExpressionWithUnboxing(leftType, unboxedL, left);
    FlagExpressionWithUnboxing(rightType, unboxedR, right);

    if (promotedType == nullptr && !bothConst) {
        ThrowTypeError("Bad operand type, the types of the operands must be numeric type.", pos);
    }

    if (bothConst) {
        return HandleBitwiseOperationOnTypes(leftType, rightType, operationType);
    }

    return SelectGlobalIntegerTypeForNumeric(promotedType);
}

checker::Type *ETSChecker::CheckBinaryOperatorLogical(ir::Expression *left, ir::Expression *right, ir::Expression *expr,
                                                      lexer::SourcePosition pos, checker::Type *const leftType,
                                                      checker::Type *const rightType, Type *unboxedL, Type *unboxedR)
{
    if (leftType->IsETSUnionType() || rightType->IsETSUnionType()) {
        ThrowTypeError("Bad operand type, unions are not allowed in binary expressions except equality.", pos);
    }

    if (unboxedL == nullptr || !unboxedL->IsConditionalExprType() || unboxedR == nullptr ||
        !unboxedR->IsConditionalExprType()) {
        ThrowTypeError("Bad operand type, the types of the operands must be of possible condition type.", pos);
    }

    if (unboxedL->HasTypeFlag(checker::TypeFlag::ETS_PRIMITIVE)) {
        FlagExpressionWithUnboxing(leftType, unboxedL, left);
    }

    if (unboxedR->HasTypeFlag(checker::TypeFlag::ETS_PRIMITIVE)) {
        FlagExpressionWithUnboxing(rightType, unboxedR, right);
    }

    if (expr->IsBinaryExpression()) {
        return HandleBooleanLogicalOperatorsExtended(unboxedL, unboxedR, expr->AsBinaryExpression());
    }

    UNREACHABLE();
}

std::tuple<Type *, Type *> ETSChecker::CheckBinaryOperatorStrictEqual(ir::Expression *left, lexer::SourcePosition pos,
                                                                      checker::Type *const leftType,
                                                                      checker::Type *const rightType)
{
    checker::Type *tsType {};
    if (!(leftType->HasTypeFlag(checker::TypeFlag::ETS_ARRAY_OR_OBJECT) || leftType->IsETSUnionType()) ||
        !(rightType->HasTypeFlag(checker::TypeFlag::ETS_ARRAY_OR_OBJECT) || rightType->IsETSUnionType())) {
        ThrowTypeError("Both operands have to be reference types", pos);
    }

    Relation()->SetNode(left);
    if (!Relation()->IsCastableTo(leftType, rightType) && !Relation()->IsCastableTo(rightType, leftType)) {
        ThrowTypeError("The operands of strict equality are not compatible with each other", pos);
    }
    tsType = GlobalETSBooleanType();
    if (rightType->IsETSDynamicType() && leftType->IsETSDynamicType()) {
        return {tsType, GlobalBuiltinJSValueType()};
    }
    return {tsType, GlobalETSObjectType()};
}

std::tuple<Type *, Type *> ETSChecker::CheckBinaryOperatorEqual(
    ir::Expression *left, ir::Expression *right, lexer::TokenType operationType, lexer::SourcePosition pos,
    checker::Type *const leftType, checker::Type *const rightType, Type *unboxedL, Type *unboxedR)
{
    checker::Type *tsType {};
    if (leftType->IsETSEnumType() && rightType->IsETSEnumType()) {
        if (!leftType->AsETSEnumType()->IsSameEnumType(rightType->AsETSEnumType())) {
            ThrowTypeError("Bad operand type, the types of the operands must be the same enum type.", pos);
        }

        tsType = GlobalETSBooleanType();
        return {tsType, leftType};
    }

    if (leftType->IsETSStringEnumType() && rightType->IsETSStringEnumType()) {
        if (!leftType->AsETSStringEnumType()->IsSameEnumType(rightType->AsETSStringEnumType())) {
            ThrowTypeError("Bad operand type, the types of the operands must be the same enum type.", pos);
        }

        tsType = GlobalETSBooleanType();
        return {tsType, leftType};
    }

    if (leftType->IsETSDynamicType() || rightType->IsETSDynamicType()) {
        return CheckBinaryOperatorEqualDynamic(left, right, pos);
    }

    if (IsReferenceType(leftType) && IsReferenceType(rightType)) {
        tsType = GlobalETSBooleanType();
        auto *opType = GlobalETSObjectType();
        return {tsType, opType};
    }

    if (unboxedL != nullptr && unboxedL->HasTypeFlag(checker::TypeFlag::ETS_BOOLEAN) && unboxedR != nullptr &&
        unboxedR->HasTypeFlag(checker::TypeFlag::ETS_BOOLEAN)) {
        if (unboxedL->HasTypeFlag(checker::TypeFlag::CONSTANT) && unboxedR->HasTypeFlag(checker::TypeFlag::CONSTANT)) {
            bool res = unboxedL->AsETSBooleanType()->GetValue() == unboxedR->AsETSBooleanType()->GetValue();

            tsType = CreateETSBooleanType(operationType == lexer::TokenType::PUNCTUATOR_EQUAL ? res : !res);
            return {tsType, tsType};
        }

        FlagExpressionWithUnboxing(leftType, unboxedL, left);
        FlagExpressionWithUnboxing(rightType, unboxedR, right);

        tsType = GlobalETSBooleanType();
        return {tsType, tsType};
    }
    return {nullptr, nullptr};
}

std::tuple<Type *, Type *> ETSChecker::CheckBinaryOperatorEqualDynamic(ir::Expression *left, ir::Expression *right,
                                                                       lexer::SourcePosition pos)
{
    // NOTE: vpukhov. enforce intrinsic call in any case?
    // canonicalize
    auto *const dynExp = left->TsType()->IsETSDynamicType() ? left : right;
    auto *const otherExp = dynExp == left ? right : left;

    if (otherExp->TsType()->IsETSDynamicType()) {
        return {GlobalETSBooleanType(), GlobalBuiltinJSValueType()};
    }
    if (dynExp->TsType()->AsETSDynamicType()->IsConvertible(otherExp->TsType())) {
        // NOTE: vpukhov. boxing flags are not set in dynamic values
        return {GlobalETSBooleanType(), otherExp->TsType()};
    }
    if (IsReferenceType(otherExp->TsType())) {
        // have to prevent casting dyn_exp via ApplyCast without nullish flag
        return {GlobalETSBooleanType(), GlobalETSNullishObjectType()};
    }
    ThrowTypeError("Unimplemented case in dynamic type comparison.", pos);
}

std::tuple<Type *, Type *> ETSChecker::CheckBinaryOperatorLessGreater(
    ir::Expression *left, ir::Expression *right, lexer::TokenType operationType, lexer::SourcePosition pos,
    bool isEqualOp, checker::Type *const leftType, checker::Type *const rightType, Type *unboxedL, Type *unboxedR)
{
    if ((leftType->IsETSUnionType() || rightType->IsETSUnionType()) &&
        operationType != lexer::TokenType::PUNCTUATOR_EQUAL &&
        operationType != lexer::TokenType::PUNCTUATOR_NOT_EQUAL) {
        ThrowTypeError("Bad operand type, unions are not allowed in binary expressions except equality.", pos);
    }

    checker::Type *tsType {};
    auto [promotedType, bothConst] =
        ApplyBinaryOperatorPromotion(unboxedL, unboxedR, TypeFlag::ETS_NUMERIC, !isEqualOp);

    FlagExpressionWithUnboxing(leftType, unboxedL, left);
    FlagExpressionWithUnboxing(rightType, unboxedR, right);

    if (leftType->IsETSUnionType()) {
        tsType = GlobalETSBooleanType();
        return {tsType, leftType->AsETSUnionType()};
    }

    if (rightType->IsETSUnionType()) {
        tsType = GlobalETSBooleanType();
        return {tsType, rightType->AsETSUnionType()};
    }

    if (promotedType == nullptr && !bothConst) {
        ThrowTypeError("Bad operand type, the types of the operands must be numeric type.", pos);
    }

    if (bothConst) {
        tsType = HandleRelationOperationOnTypes(leftType, rightType, operationType);
        return {tsType, tsType};
    }

    tsType = GlobalETSBooleanType();
    auto *opType = promotedType;
    return {tsType, opType};
}

std::tuple<Type *, Type *> ETSChecker::CheckBinaryOperatorInstanceOf(lexer::SourcePosition pos,
                                                                     checker::Type *const leftType,
                                                                     checker::Type *const rightType)
{
    checker::Type *tsType {};
    if (!IsReferenceType(leftType) || !IsReferenceType(rightType)) {
        ThrowTypeError("Bad operand type, the types of the operands must be same type.", pos);
    }

    if (rightType->IsETSDynamicType() || leftType->IsETSDynamicType()) {
        if (!(rightType->IsETSDynamicType() && leftType->IsETSDynamicType())) {
            ThrowTypeError("Bad operand type, both types of the operands must be dynamic.", pos);
        }
    }

    tsType = GlobalETSBooleanType();
    checker::Type *opType = rightType->IsETSDynamicType() ? GlobalBuiltinJSValueType() : GlobalETSObjectType();
    return {tsType, opType};
}

Type *ETSChecker::CheckBinaryOperatorNullishCoalescing(ir::Expression *right, lexer::SourcePosition pos,
                                                       checker::Type *const leftType, checker::Type *const rightType)
{
    if (!leftType->HasTypeFlag(checker::TypeFlag::ETS_ARRAY_OR_OBJECT)) {
        ThrowTypeError("Left-hand side expression must be a reference type.", pos);
    }

    checker::Type *nonNullishLeftType = leftType;

    if (leftType->IsNullish()) {
        nonNullishLeftType = GetNonNullishType(leftType);
    }

    // NOTE: vpukhov. check convertibility and use numeric promotion

    if (rightType->HasTypeFlag(checker::TypeFlag::ETS_PRIMITIVE)) {
        Relation()->SetNode(right);
        auto boxedRightType = PrimitiveTypeAsETSBuiltinType(rightType);
        if (boxedRightType == nullptr) {
            ThrowTypeError("Invalid right-hand side expression", pos);
        }
        right->AddBoxingUnboxingFlags(GetBoxingFlag(boxedRightType));
        return FindLeastUpperBound(nonNullishLeftType, boxedRightType);
    }

    return FindLeastUpperBound(nonNullishLeftType, rightType);
}

// NOLINTNEXTLINE(readability-function-size)
std::tuple<Type *, Type *> ETSChecker::CheckBinaryOperator(ir::Expression *left, ir::Expression *right,
                                                           ir::Expression *expr, lexer::TokenType operationType,
                                                           lexer::SourcePosition pos, bool forcePromotion)
{
    checker::Type *const leftType = left->Check(this);
    checker::Type *const rightType = right->Check(this);
    const bool isLogicalExtendedOperator = (operationType == lexer::TokenType::PUNCTUATOR_LOGICAL_AND) ||
                                           (operationType == lexer::TokenType::PUNCTUATOR_LOGICAL_OR);
    Type *unboxedL =
        isLogicalExtendedOperator ? ETSBuiltinTypeAsConditionalType(leftType) : ETSBuiltinTypeAsPrimitiveType(leftType);
    Type *unboxedR = isLogicalExtendedOperator ? ETSBuiltinTypeAsConditionalType(rightType)
                                               : ETSBuiltinTypeAsPrimitiveType(rightType);

    checker::Type *tsType {};
    bool isEqualOp = (operationType > lexer::TokenType::PUNCTUATOR_SUBSTITUTION &&
                      operationType < lexer::TokenType::PUNCTUATOR_ARROW) &&
                     !forcePromotion;

    switch (operationType) {
        case lexer::TokenType::PUNCTUATOR_MULTIPLY:
        case lexer::TokenType::PUNCTUATOR_MULTIPLY_EQUAL:
        case lexer::TokenType::PUNCTUATOR_DIVIDE:
        case lexer::TokenType::PUNCTUATOR_DIVIDE_EQUAL:
        case lexer::TokenType::PUNCTUATOR_MOD:
        case lexer::TokenType::PUNCTUATOR_MOD_EQUAL: {
            tsType = CheckBinaryOperatorMulDivMod(left, right, operationType, pos, isEqualOp, leftType, rightType,
                                                  unboxedL, unboxedR);
            break;
        }
        case lexer::TokenType::PUNCTUATOR_MINUS:
        case lexer::TokenType::PUNCTUATOR_MINUS_EQUAL: {
            if (leftType->IsETSStringType() || rightType->IsETSStringType()) {
                ThrowTypeError("Bad operand type, the types of the operands must be numeric type.", pos);
            }

            [[fallthrough]];
        }
        case lexer::TokenType::PUNCTUATOR_PLUS:
        case lexer::TokenType::PUNCTUATOR_PLUS_EQUAL: {
            tsType = CheckBinaryOperatorPlus(left, right, operationType, pos, isEqualOp, leftType, rightType, unboxedL,
                                             unboxedR);
            break;
        }
        case lexer::TokenType::PUNCTUATOR_LEFT_SHIFT:
        case lexer::TokenType::PUNCTUATOR_LEFT_SHIFT_EQUAL:
        case lexer::TokenType::PUNCTUATOR_RIGHT_SHIFT:
        case lexer::TokenType::PUNCTUATOR_RIGHT_SHIFT_EQUAL:
        case lexer::TokenType::PUNCTUATOR_UNSIGNED_RIGHT_SHIFT:
        case lexer::TokenType::PUNCTUATOR_UNSIGNED_RIGHT_SHIFT_EQUAL: {
            tsType = CheckBinaryOperatorShift(left, right, operationType, pos, isEqualOp, leftType, rightType, unboxedL,
                                              unboxedR);
            break;
        }
        case lexer::TokenType::PUNCTUATOR_BITWISE_OR:
        case lexer::TokenType::PUNCTUATOR_BITWISE_OR_EQUAL:
        case lexer::TokenType::PUNCTUATOR_BITWISE_AND:
        case lexer::TokenType::PUNCTUATOR_BITWISE_AND_EQUAL:
        case lexer::TokenType::PUNCTUATOR_BITWISE_XOR_EQUAL:
        case lexer::TokenType::PUNCTUATOR_BITWISE_XOR: {
            tsType = CheckBinaryOperatorBitwise(left, right, operationType, pos, isEqualOp, leftType, rightType,
                                                unboxedL, unboxedR);
            break;
        }
        case lexer::TokenType::PUNCTUATOR_LOGICAL_AND:
        case lexer::TokenType::PUNCTUATOR_LOGICAL_OR: {
            tsType = CheckBinaryOperatorLogical(left, right, expr, pos, leftType, rightType, unboxedL, unboxedR);
            break;
        }
        case lexer::TokenType::PUNCTUATOR_STRICT_EQUAL:
        case lexer::TokenType::PUNCTUATOR_NOT_STRICT_EQUAL: {
            return CheckBinaryOperatorStrictEqual(left, pos, leftType, rightType);
        }
        case lexer::TokenType::PUNCTUATOR_EQUAL:
        case lexer::TokenType::PUNCTUATOR_NOT_EQUAL: {
            std::tuple<Type *, Type *> res =
                CheckBinaryOperatorEqual(left, right, operationType, pos, leftType, rightType, unboxedL, unboxedR);
            if (!(std::get<0>(res) == nullptr && std::get<1>(res) == nullptr)) {
                return res;
            }
            [[fallthrough]];
        }
        case lexer::TokenType::PUNCTUATOR_LESS_THAN:
        case lexer::TokenType::PUNCTUATOR_LESS_THAN_EQUAL:
        case lexer::TokenType::PUNCTUATOR_GREATER_THAN:
        case lexer::TokenType::PUNCTUATOR_GREATER_THAN_EQUAL: {
            return CheckBinaryOperatorLessGreater(left, right, operationType, pos, isEqualOp, leftType, rightType,
                                                  unboxedL, unboxedR);
        }
        case lexer::TokenType::KEYW_INSTANCEOF: {
            return CheckBinaryOperatorInstanceOf(pos, leftType, rightType);
        }
        case lexer::TokenType::PUNCTUATOR_NULLISH_COALESCING: {
            tsType = CheckBinaryOperatorNullishCoalescing(right, pos, leftType, rightType);
            break;
        }
        default: {
            // NOTE
            UNREACHABLE();
            break;
        }
    }

    return {tsType, tsType};
}

Type *ETSChecker::HandleArithmeticOperationOnTypes(Type *left, Type *right, lexer::TokenType operationType)
{
    ASSERT(left->HasTypeFlag(TypeFlag::CONSTANT | TypeFlag::ETS_NUMERIC) &&
           right->HasTypeFlag(TypeFlag::CONSTANT | TypeFlag::ETS_NUMERIC));

    if (left->IsDoubleType() || right->IsDoubleType()) {
        return PerformArithmeticOperationOnTypes<DoubleType>(left, right, operationType);
    }

    if (left->IsFloatType() || right->IsFloatType()) {
        return PerformArithmeticOperationOnTypes<FloatType>(left, right, operationType);
    }

    if (left->IsLongType() || right->IsLongType()) {
        return PerformArithmeticOperationOnTypes<LongType>(left, right, operationType);
    }

    return PerformArithmeticOperationOnTypes<IntType>(left, right, operationType);
}

Type *ETSChecker::HandleBitwiseOperationOnTypes(Type *left, Type *right, lexer::TokenType operationType)
{
    ASSERT(left->HasTypeFlag(TypeFlag::CONSTANT | TypeFlag::ETS_NUMERIC) &&
           right->HasTypeFlag(TypeFlag::CONSTANT | TypeFlag::ETS_NUMERIC));

    if (left->IsDoubleType() || right->IsDoubleType()) {
        return HandleBitWiseArithmetic<DoubleType, LongType>(left, right, operationType);
    }

    if (left->IsFloatType() || right->IsFloatType()) {
        return HandleBitWiseArithmetic<FloatType, IntType>(left, right, operationType);
    }

    if (left->IsLongType() || right->IsLongType()) {
        return HandleBitWiseArithmetic<LongType>(left, right, operationType);
    }

    return HandleBitWiseArithmetic<IntType>(left, right, operationType);
}

void ETSChecker::FlagExpressionWithUnboxing(Type *type, Type *unboxedType, ir::Expression *typeExpression)
{
    if (type->IsETSObjectType() && (unboxedType != nullptr)) {
        typeExpression->AddBoxingUnboxingFlags(GetUnboxingFlag(unboxedType));
    }
}

}  // namespace panda::es2panda::checker
