/************************************************************
 * File name:      expr.h
 * Course:         CS 525 Advanced Database Organization (Spring 2025)
 * Authors:        Harlee Ramos, Jisun Yun, Baozhu Xie
 *
 * Description:
 *   This header file defines the types, macros, and function
 *   prototypes necessary for expression evaluation within the
 *   Record Manager project. Expressions are used to define conditions,
 *   for instance during record scans. They can represent operators,
 *   constants, or attribute references. In addition, helper macros are
 *   provided to ease the creation of various expression types.
 ************************************************************/

#ifndef EXPR_H
#define EXPR_H

#include "dberror.h"   // For error codes and error handling
#include "tables.h"    // For table schema definitions and value types

/*
 * Enumeration: ExprType
 * ---------------------
 * Defines the type of expression:
 *   EXPR_OP      : Expression is an operator with arguments.
 *   EXPR_CONST   : Expression is a constant value.
 *   EXPR_ATTRREF : Expression is a reference to an attribute.
 */
typedef enum ExprType {
  EXPR_OP,
  EXPR_CONST,
  EXPR_ATTRREF
} ExprType;

/*
 * Structure: Expr
 * ---------------
 * Represents an expression used in conditions.
 * Depending on the type, it may store:
 *   - A constant value (cons)
 *   - An attribute reference (attrRef)
 *   - A pointer to an operator structure (op)
 */
typedef struct Expr {
  ExprType type;
  union expr {
    Value *cons;         // Pointer to a constant value
    int attrRef;         // Index or identifier for an attribute reference
    struct Operator *op; // Pointer to an operator structure (for composite expressions)
  } expr;
} Expr;

/*
 * Enumeration: OpType
 * -------------------
 * Defines the types of comparison and boolean operators.
 *   OP_BOOL_AND   : Logical AND operator.
 *   OP_BOOL_OR    : Logical OR operator.
 *   OP_BOOL_NOT   : Logical NOT operator.
 *   OP_COMP_EQUAL : Equality comparison operator.
 *   OP_COMP_SMALLER: Comparison operator to test if left operand is smaller.
 */
typedef enum OpType {
  OP_BOOL_AND,
  OP_BOOL_OR,
  OP_BOOL_NOT,
  OP_COMP_EQUAL,
  OP_COMP_SMALLER
} OpType;

/*
 * Structure: Operator
 * -------------------
 * Represents an operator in an expression. This structure holds:
 *   - The type of the operator.
 *   - An array of expression pointers as its arguments.
 */
typedef struct Operator {
  OpType type;
  Expr **args;  // Array of pointers to operand expressions
} Operator;

/*
 * Expression Evaluation Function Prototypes:
 *
 * These functions evaluate expressions or perform operations on Value
 * structures. They are used during record scans and other conditional
 * evaluations.
 */

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
 */
extern RC valueEquals (Value *left, Value *right, Value *result);

/*
 * Function: valueSmaller
 * ----------------------
 * Checks if the left Value is smaller than the right Value.
 *
 * Parameters:
 *   left   - Pointer to the left operand Value.
 *   right  - Pointer to the right operand Value.
 *   result - Pointer to a Value where the boolean result is stored.
 *
 * Returns:
 *   RC_OK on success or an appropriate error code.
 */
extern RC valueSmaller (Value *left, Value *right, Value *result);

/*
 * Function: boolNot
 * -----------------
 * Computes the boolean NOT of the input Value.
 *
 * Parameters:
 *   input  - Pointer to the input Value.
 *   result - Pointer to a Value where the negated boolean is stored.
 *
 * Returns:
 *   RC_OK on success or an appropriate error code.
 */
extern RC boolNot (Value *input, Value *result);

/*
 * Function: boolAnd
 * -----------------
 * Computes the logical AND of two boolean Values.
 *
 * Parameters:
 *   left   - Pointer to the left operand Value.
 *   right  - Pointer to the right operand Value.
 *   result - Pointer to a Value where the result is stored.
 *
 * Returns:
 *   RC_OK on success or an appropriate error code.
 */
extern RC boolAnd (Value *left, Value *right, Value *result);

/*
 * Function: boolOr
 * ----------------
 * Computes the logical OR of two boolean Values.
 *
 * Parameters:
 *   left   - Pointer to the left operand Value.
 *   right  - Pointer to the right operand Value.
 *   result - Pointer to a Value where the result is stored.
 *
 * Returns:
 *   RC_OK on success or an appropriate error code.
 */
extern RC boolOr (Value *left, Value *right, Value *result);

/*
 * Function: evalExpr
 * ------------------
 * Evaluates an expression for a given record and schema.
 *
 * Parameters:
 *   record - Pointer to the record from which to evaluate attribute values.
 *   schema - Pointer to the schema associated with the record.
 *   expr   - Pointer to the expression to evaluate.
 *   result - Double pointer to a Value where the evaluation result will be stored.
 *
 * Returns:
 *   RC_OK on success or an appropriate error code.
 */
extern RC evalExpr (Record *record, Schema *schema, Expr *expr, Value **result);

/*
 * Function: freeExpr
 * ------------------
 * Frees the memory allocated for an expression.
 *
 * Parameters:
 *   expr - Pointer to the expression to be freed.
 *
 * Returns:
 *   RC_OK on success or an appropriate error code.
 */
extern RC freeExpr (Expr *expr);

/*
 * Function: freeVal
 * -----------------
 * Frees the memory allocated for a Value.
 *
 * Parameters:
 *   val - Pointer to the Value to be freed.
 */
extern void freeVal(Value *val);

/*
 * Macros for Expression Construction:
 *
 * These helper macros simplify the creation of expressions by wrapping
 * the necessary memory allocation and initialization routines.
 */

/*
 * Macro: CPVAL
 * ------------
 * Copies the content of one Value to another.
 *
 * Parameters:
 *   _result - Destination Value pointer.
 *   _input  - Source Value pointer.
 */
#define CPVAL(_result, _input)                      \
  do {                                              \
    (_result)->dt = _input->dt;                     \
    switch(_input->dt) {                            \
      case DT_INT:                                \
        (_result)->v.intV = _input->v.intV;         \
        break;                                    \
      case DT_STRING:                             \
        (_result)->v.stringV = (char *) malloc(strlen(_input->v.stringV) + 1); \
        strcpy((_result)->v.stringV, _input->v.stringV);  \
        break;                                    \
      case DT_FLOAT:                              \
        (_result)->v.floatV = _input->v.floatV;     \
        break;                                    \
      case DT_BOOL:                               \
        (_result)->v.boolV = _input->v.boolV;       \
        break;                                    \
    }                                             \
  } while(0)

/*
 * Macro: MAKE_BINOP_EXPR
 * ----------------------
 * Creates a binary operator expression with two operands.
 *
 * Parameters:
 *   _result  - Variable to hold the resulting expression pointer.
 *   _left    - Left operand expression.
 *   _right   - Right operand expression.
 *   _optype  - Operator type (OpType).
 */
#define MAKE_BINOP_EXPR(_result, _left, _right, _optype)  \
    do {                                                \
      Operator *_op = (Operator *) malloc(sizeof(Operator));   \
      _result = (Expr *) malloc(sizeof(Expr));          \
      _result->type = EXPR_OP;                          \
      _result->expr.op = _op;                           \
      _op->type = _optype;                              \
      _op->args = (Expr **) malloc(2 * sizeof(Expr*));  \
      _op->args[0] = _left;                             \
      _op->args[1] = _right;                            \
    } while (0)

/*
 * Macro: MAKE_UNOP_EXPR
 * ---------------------
 * Creates a unary operator expression with a single operand.
 *
 * Parameters:
 *   _result  - Variable to hold the resulting expression pointer.
 *   _input   - Operand expression.
 *   _optype  - Operator type (OpType).
 */
#define MAKE_UNOP_EXPR(_result, _input, _optype)       \
  do {                                                 \
    Operator *_op = (Operator *) malloc(sizeof(Operator));   \
    _result = (Expr *) malloc(sizeof(Expr));           \
    _result->type = EXPR_OP;                           \
    _result->expr.op = _op;                            \
    _op->type = _optype;                               \
    _op->args = (Expr **) malloc(sizeof(Expr*));       \
    _op->args[0] = _input;                             \
  } while (0)

/*
 * Macro: MAKE_ATTRREF
 * -------------------
 * Creates an attribute reference expression.
 *
 * Parameters:
 *   _result - Variable to hold the resulting expression pointer.
 *   _attr   - Attribute index or identifier.
 */
#define MAKE_ATTRREF(_result, _attr)                   \
  do {                                                 \
    _result = (Expr *) malloc(sizeof(Expr));           \
    _result->type = EXPR_ATTRREF;                      \
    _result->expr.attrRef = _attr;                     \
  } while(0)

/*
 * Macro: MAKE_CONS
 * ----------------
 * Creates a constant expression.
 *
 * Parameters:
 *   _result - Variable to hold the resulting expression pointer.
 *   _value  - Pointer to the constant Value.
 */
#define MAKE_CONS(_result, _value)                     \
  do {                                                 \
    _result = (Expr *) malloc(sizeof(Expr));           \
    _result->type = EXPR_CONST;                        \
    _result->expr.cons = _value;                       \
  } while(0)

#endif // EXPR_H