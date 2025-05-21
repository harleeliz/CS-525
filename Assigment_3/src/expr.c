/************************************************************
 * File name:      expr.c
 * Course:         CS 525 Advanced Database Organization (Spring 2025)
 * Authors:        Harlee Ramos, Jisun Yun, Baozhu Xie
 *
 * Description:
 *   This source file implements the functions for expression
 *   evaluation used in the Record Manager project. Expressions
 *   represent conditions for record filtering and can be of types:
 *     - Constant (EXPR_CONST)
 *     - Attribute reference (EXPR_ATTRREF)
 *     - Operator (EXPR_OP) that combines one or two sub-expressions.
 *   The file provides functions to compare values, perform boolean
 *   operations, evaluate expressions on records, and free expression
 *   structures.
 ************************************************************/

#include <string.h>
#include <stdlib.h>

#include "dberror.h"
#include "record_mgr.h"
#include "expr.h"
#include "tables.h"

/*
 * Function: valueEquals
 * ---------------------
 * Compares two Value structures for equality.
 *
 * Parameters:
 *   left   - Pointer to the left operand Value.
 *   right  - Pointer to the right operand Value.
 *   result - Pointer to a Value where the boolean result is stored.
 *
 * Returns:
 *   RC_OK on success or an appropriate error code.
 *
 * Notes:
 *   - The two values must be of the same datatype.
 *   - The result is set to a boolean (DT_BOOL).
 */
RC valueEquals(Value *left, Value *right, Value *result)
{
    if(left->dt != right->dt)
        THROW(RC_RM_COMPARE_VALUE_OF_DIFFERENT_DATATYPE,
              "equality comparison only supported for values of the same datatype");

    result->dt = DT_BOOL;

    switch(left->dt) {
    case DT_INT:
        result->v.boolV = (left->v.intV == right->v.intV);
        break;
    case DT_FLOAT:
        result->v.boolV = (left->v.floatV == right->v.floatV);
        break;
    case DT_BOOL:
        result->v.boolV = (left->v.boolV == right->v.boolV);
        break;
    case DT_STRING:
        result->v.boolV = (strcmp(left->v.stringV, right->v.stringV) == 0);
        break;
    default:
        // Should not reach here
        break;
    }

    return RC_OK;
}

/*
 * Function: valueSmaller
 * ----------------------
 * Compares two Value structures to determine if the left value is
 * smaller than the right value.
 *
 * Parameters:
 *   left   - Pointer to the left operand Value.
 *   right  - Pointer to the right operand Value.
 *   result - Pointer to a Value where the boolean result is stored.
 *
 * Returns:
 *   RC_OK on success or an appropriate error code.
 *
 * Notes:
 *   - Both values must be of the same datatype.
 *   - The result is set to a boolean (DT_BOOL).
 */
RC valueSmaller(Value *left, Value *right, Value *result)
{
    if(left->dt != right->dt)
        THROW(RC_RM_COMPARE_VALUE_OF_DIFFERENT_DATATYPE,
              "comparison only supported for values of the same datatype");

    result->dt = DT_BOOL;

    switch(left->dt) {
    case DT_INT:
        result->v.boolV = (left->v.intV < right->v.intV);
        break;
    case DT_FLOAT:
        result->v.boolV = (left->v.floatV < right->v.floatV);
        break;
    case DT_BOOL:
        // Note: For booleans, a numeric comparison is not very meaningful,
        // but we assume false < true.
        result->v.boolV = (left->v.boolV < right->v.boolV);
        break;
    case DT_STRING:
        result->v.boolV = (strcmp(left->v.stringV, right->v.stringV) < 0);
        break;
    default:
        // Should not reach here
        break;
    }

    return RC_OK;
}

/*
 * Function: boolNot
 * -----------------
 * Computes the boolean NOT of the input value.
 *
 * Parameters:
 *   input  - Pointer to the input Value.
 *   result - Pointer to a Value where the result is stored.
 *
 * Returns:
 *   RC_OK on success or an appropriate error code.
 *
 * Notes:
 *   - The input must be of type DT_BOOL.
 */
RC boolNot(Value *input, Value *result)
{
    if (input->dt != DT_BOOL)
        THROW(RC_RM_BOOLEAN_EXPR_ARG_IS_NOT_BOOLEAN,
              "boolean NOT requires boolean input");

    result->dt = DT_BOOL;
    result->v.boolV = !(input->v.boolV);

    return RC_OK;
}

/*
 * Function: boolAnd
 * -----------------
 * Computes the logical AND of two boolean values.
 *
 * Parameters:
 *   left   - Pointer to the left operand Value.
 *   right  - Pointer to the right operand Value.
 *   result - Pointer to a Value where the result is stored.
 *
 * Returns:
 *   RC_OK on success or an appropriate error code.
 *
 * Notes:
 *   - Both inputs must be of type DT_BOOL.
 */
RC boolAnd(Value *left, Value *right, Value *result)
{
    if (left->dt != DT_BOOL || right->dt != DT_BOOL)
        THROW(RC_RM_BOOLEAN_EXPR_ARG_IS_NOT_BOOLEAN,
              "boolean AND requires boolean inputs");

    result->dt = DT_BOOL;
    result->v.boolV = (left->v.boolV && right->v.boolV);

    return RC_OK;
}

/*
 * Function: boolOr
 * ----------------
 * Computes the logical OR of two boolean values.
 *
 * Parameters:
 *   left   - Pointer to the left operand Value.
 *   right  - Pointer to the right operand Value.
 *   result - Pointer to a Value where the result is stored.
 *
 * Returns:
 *   RC_OK on success or an appropriate error code.
 *
 * Notes:
 *   - Both inputs must be of type DT_BOOL.
 */
RC boolOr(Value *left, Value *right, Value *result)
{
    if (left->dt != DT_BOOL || right->dt != DT_BOOL)
        THROW(RC_RM_BOOLEAN_EXPR_ARG_IS_NOT_BOOLEAN,
              "boolean OR requires boolean inputs");

    result->dt = DT_BOOL;
    result->v.boolV = (left->v.boolV || right->v.boolV);

    return RC_OK;
}

/*
 * Function: evalExpr
 * ------------------
 * Recursively evaluates an expression against a given record and schema.
 *
 * Parameters:
 *   record - Pointer to the record on which the expression is evaluated.
 *   schema - Pointer to the schema associated with the record.
 *   expr   - Pointer to the expression to be evaluated.
 *   result - Double pointer to a Value where the evaluation result is stored.
 *
 * Returns:
 *   RC_OK on success or an appropriate error code.
 *
 * Notes:
 *   - If the expression is an operator (EXPR_OP), it recursively
 *     evaluates its arguments and then applies the operator.
 *   - For EXPR_CONST, a copy of the constant value is assigned.
 *   - For EXPR_ATTRREF, the attribute value is retrieved from the record.
 */
RC evalExpr(Record *record, Schema *schema, Expr *expr, Value **result)
{
    Value *lIn;
    Value *rIn;
    // Allocate a temporary default result (initialized to -1)
    MAKE_VALUE(*result, DT_INT, -1);

    switch(expr->type)
    {
    case EXPR_OP:
    {
        Operator *op = expr->expr.op;
        // For boolean NOT, there is only one argument.
        bool twoArgs = (op->type != OP_BOOL_NOT);

        // Recursively evaluate the first operand
        CHECK(evalExpr(record, schema, op->args[0], &lIn));
        if (twoArgs)
            CHECK(evalExpr(record, schema, op->args[1], &rIn));

        // Apply the operator based on its type
        switch(op->type)
        {
        case OP_BOOL_NOT:
            CHECK(boolNot(lIn, *result));
            break;
        case OP_BOOL_AND:
            CHECK(boolAnd(lIn, rIn, *result));
            break;
        case OP_BOOL_OR:
            CHECK(boolOr(lIn, rIn, *result));
            break;
        case OP_COMP_EQUAL:
            CHECK(valueEquals(lIn, rIn, *result));
            break;
        case OP_COMP_SMALLER:
            CHECK(valueSmaller(lIn, rIn, *result));
            break;
        default:
            // Unknown operator; could throw an error here.
            break;
        }
        // Clean up temporary values
        freeVal(lIn);
        if (twoArgs)
            freeVal(rIn);
    }
    break;
    case EXPR_CONST:
        // For a constant expression, copy the constant value into result.
        CPVAL(*result, expr->expr.cons);
        break;
    case EXPR_ATTRREF:
        // For an attribute reference, free the default result and
        // retrieve the attribute's value from the record.
        free(*result);
        CHECK(getAttr(record, schema, expr->expr.attrRef, result));
        break;
    }

    return RC_OK;
}

/*
 * Function: freeExpr
 * ------------------
 * Recursively frees the memory allocated for an expression, including
 * its sub-expressions and operator arguments.
 *
 * Parameters:
 *   expr - Pointer to the expression to be freed.
 *
 * Returns:
 *   RC_OK on success.
 *
 * Notes:
 *   - For operator expressions (EXPR_OP), it frees each argument
 *     and then frees the operator structure.
 *   - For constant and attribute reference expressions, only the
 *     expression structure is freed.
 */
RC freeExpr(Expr *expr)
{
    switch(expr->type)
    {
    case EXPR_OP:
    {
        Operator *op = expr->expr.op;
        // Free sub-expressions based on the operator type
        if(op->type == OP_BOOL_NOT)
        {
            freeExpr(op->args[0]);
        }
        else
        {
            freeExpr(op->args[0]);
            freeExpr(op->args[1]);
        }
        // Free the arguments array and the operator itself
        free(op->args);
        free(op);
    }
    break;
    case EXPR_CONST:
        // For constant expressions, we assume that the constant Value
        // is managed elsewhere or does not require freeing here.
        break;
    case EXPR_ATTRREF:
        // For attribute reference expressions, no extra memory needs to be freed.
        break;
    }
    // Finally, free the expression structure itself.
    free(expr);
    return RC_OK;
}