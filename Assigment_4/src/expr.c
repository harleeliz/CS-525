/************************************************************
 *     File name:                      expr.c
 *     CS 525 Advanced Database Organization (Spring 2025)
 *     Harlee Ramos, Jisun Yun, Baozhu Xie
 ************************************************************/

#include <string.h>
#include <stdlib.h>
#include <stdio.h>
#include "dberror.h"
#include "record_mgr.h"
#include "expr.h"
#include "tables.h"

/* 
 * Compares two values for equality.
 * Both values must be of the same datatype.
 * The result is a boolean (DT_BOOL) value.
 */
RC valueEquals(Value *left, Value *right, Value *result) {
    if (left->dt != right->dt)
        THROW(RC_RM_COMPARE_VALUE_OF_DIFFERENT_DATATYPE, "Equality comparison only supported for values of the same datatype");

    result->dt = DT_BOOL;
    switch(left->dt) {
        case DT_INT:
            result->v.boolV = (left->v.intV == right->v.intV);
        printf("valueEquals(): %d == %d -> %d\n", left->v.intV, right->v.intV, result->v.boolV);
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
            THROW(RC_RM_EXPR_NOT_SUPPORTED, "Unsupported datatype in valueEquals");
    }
    return RC_OK;
}


/*
 * Determines whether the left value is smaller than the right value.
 * Both values must be of the same datatype.
 * The result is a boolean (DT_BOOL) value.
 */
RC valueSmaller(Value *left, Value *right, Value *result) {
    if (left->dt != right->dt)
        THROW(RC_RM_COMPARE_VALUE_OF_DIFFERENT_DATATYPE, "Comparison requires values of the same datatype");

    result->dt = DT_BOOL;
    switch(left->dt) {
        case DT_INT:
            result->v.boolV = (left->v.intV < right->v.intV);
            break;
        case DT_FLOAT:
            result->v.boolV = (left->v.floatV < right->v.floatV);
            break;
        case DT_BOOL:
            result->v.boolV = (left->v.boolV < right->v.boolV);
            break;
        case DT_STRING:
            result->v.boolV = (strcmp(left->v.stringV, right->v.stringV) < 0);
            break;
        default:
            THROW(RC_RM_EXPR_NOT_SUPPORTED, "Unsupported datatype in valueSmaller");
    }
    return RC_OK;
}

/*
 * Evaluates the boolean NOT operation.
 * Input must be a boolean value.
 */
RC boolNot(Value *input, Value *result) {
    if (input->dt != DT_BOOL)
        THROW(RC_RM_BOOLEAN_EXPR_ARG_IS_NOT_BOOLEAN, "Boolean NOT requires a boolean input");
    result->dt = DT_BOOL;
    result->v.boolV = !(input->v.boolV);
    return RC_OK;
}


/*
 * Evaluates the boolean AND operation.
 * Both operands must be booleans.
 */
RC boolAnd(Value *left, Value *right, Value *result) {
    if (left->dt != DT_BOOL || right->dt != DT_BOOL)
        THROW(RC_RM_BOOLEAN_EXPR_ARG_IS_NOT_BOOLEAN, "Boolean AND requires boolean inputs");
    result->dt = DT_BOOL;
    result->v.boolV = (left->v.boolV && right->v.boolV);
    return RC_OK;
}

/*
 * Evaluates the boolean OR operation.
 * Both operands must be booleans.
 */
RC boolOr(Value *left, Value *right, Value *result) {
    if (left->dt != DT_BOOL || right->dt != DT_BOOL)
        THROW(RC_RM_BOOLEAN_EXPR_ARG_IS_NOT_BOOLEAN, "Boolean OR requires boolean inputs");
    result->dt = DT_BOOL;
    result->v.boolV = (left->v.boolV || right->v.boolV);
    return RC_OK;
}

/*
 * Recursively evaluates an expression given a record and schema.
 * The result is returned via the provided pointer.
 */
RC evalExpr(Record *record, Schema *schema, Expr *expr, Value **result) {
    Value *lIn = NULL;
    Value *rIn = NULL;

    switch(expr->type) {
        case EXPR_OP: {
            Operator *op = expr->expr.op;
            Value *tempVal;
            MAKE_INT_VALUE(tempVal, -1);
            *result = tempVal;

            /* Evaluate left operand always; right operand only if needed */
            CHECK(evalExpr(record, schema, op->args[0], &lIn));
            if (op->type != OP_BOOL_NOT) {
                CHECK(evalExpr(record, schema, op->args[1], &rIn));
            }

            switch(op->type) {
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
                    THROW(RC_RM_EXPR_NOT_SUPPORTED, "Unknown operator");
            }
            freeVal(lIn);
            if (op->type != OP_BOOL_NOT)
                freeVal(rIn);
            break;
        }
        case EXPR_CONST:
            CPVAL(*result, expr->expr.cons);
            break;
        case EXPR_ATTRREF:
            free(*result);
            CHECK(getAttr(record, schema, expr->expr.attrRef, result));
            break;
        default:
            THROW(RC_RM_EXPR_NOT_SUPPORTED, "Unsupported expression type");
    }
    return RC_OK;
}

/*
 * Frees all memory allocated for an expression, including its subexpressions.
 */
RC freeExpr(Expr *expr) {
    if (!expr)
        return RC_OK;
    
    switch(expr->type) {
        case EXPR_OP: {
            Operator *op = expr->expr.op;
            if (op) {
                switch(op->type) {
                    case OP_BOOL_NOT:
                        freeExpr(op->args[0]);
                        break;
                    default:
                        freeExpr(op->args[0]);
                        freeExpr(op->args[1]);
                        break;
                }
                free(op->args);
                free(op);
            }
            break;
        }
        case EXPR_CONST:
            freeVal(expr->expr.cons);
            break;
        case EXPR_ATTRREF:
            /* Nothing to free in attribute reference */
            break;
        default:
            break;
    }
    free(expr);
    return RC_OK;
}

/*
 * Frees a Value structure.
 * If the value is a string, its contents are also freed.
 */
void freeVal(Value *val) {
    if (!val)
        return;
    if (val->dt == DT_STRING)
        free(val->v.stringV);
    free(val);
}