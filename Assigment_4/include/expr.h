/************************************************************
 *     File name:                expr.h
 *     CS 525 Advanced Database Organization (Spring 2025)
 *     Harlee Ramos, Jisun Yun, Baozhu Xie
 ************************************************************/

#ifndef EXPR_H
#define EXPR_H

#include "dberror.h"
#include "tables.h"

/* Expression types */
typedef enum ExprType {
    EXPR_OP,
    EXPR_CONST,
    EXPR_ATTRREF
} ExprType;

/* Expression structure */
typedef struct Expr {
    ExprType type;
    union expr {
        Value *cons;      /* Constant value */
        int attrRef;      /* Attribute reference (by index) */
        struct Operator *op; /* Operator expression */
    } expr;
} Expr;

/* Supported operator types */
typedef enum OpType {
    OP_BOOL_AND,
    OP_BOOL_OR,
    OP_BOOL_NOT,
    OP_COMP_EQUAL,
    OP_COMP_SMALLER
} OpType;

/* Operator structure: holds operator type and its arguments */
typedef struct Operator {
    OpType type;
    Expr **args; /* Array of expression pointers */
} Operator;

/* Expression evaluation functions */
extern RC valueEquals(Value *left, Value *right, Value *result);
extern RC valueSmaller(Value *left, Value *right, Value *result);
extern RC boolNot(Value *input, Value *result);
extern RC boolAnd(Value *left, Value *right, Value *result);
extern RC boolOr(Value *left, Value *right, Value *result);
extern RC evalExpr(Record *record, Schema *schema, Expr *expr, Value **result);
extern RC freeExpr(Expr *expr);
extern void freeVal(Value *val);

/* Utility functions for constructing expressions via macros */
extern Value *stringToValue(char *value);
extern char *serializeValue(Value *val);
extern char *serializeRecord(Record *record, Schema *schema);

/* Macros for convenience */
#define CPVAL(_result,_input)                      \
  do {                                             \
    (_result)->dt = _input->dt;                    \
    switch(_input->dt) {                           \
      case DT_INT:                               \
        (_result)->v.intV = _input->v.intV;        \
        break;                                   \
      case DT_STRING:                            \
        (_result)->v.stringV = (char *)malloc(strlen(_input->v.stringV) + 1); \
        strcpy((_result)->v.stringV, _input->v.stringV); \
        break;                                   \
      case DT_FLOAT:                             \
        (_result)->v.floatV = _input->v.floatV;     \
        break;                                   \
      case DT_BOOL:                              \
        (_result)->v.boolV = _input->v.boolV;       \
        break;                                   \
    }                                            \
  } while(0)

#define MAKE_BINOP_EXPR(_result,_left,_right,_optype)  \
    do {                                               \
      Operator *_op = (Operator *)malloc(sizeof(Operator)); \
      _result = (Expr *)malloc(sizeof(Expr));          \
      _result->type = EXPR_OP;                         \
      _result->expr.op = _op;                          \
      _op->type = _optype;                             \
      _op->args = (Expr **)malloc(2 * sizeof(Expr*));    \
      _op->args[0] = _left;                            \
      _op->args[1] = _right;                           \
    } while (0)

#define MAKE_UNOP_EXPR(_result,_input,_optype)         \
  do {                                                 \
    Operator *_op = (Operator *)malloc(sizeof(Operator)); \
    _result = (Expr *)malloc(sizeof(Expr));            \
    _result->type = EXPR_OP;                           \
    _result->expr.op = _op;                            \
    _op->type = _optype;                               \
    _op->args = (Expr **)malloc(sizeof(Expr*));         \
    _op->args[0] = _input;                             \
  } while (0)

#define MAKE_ATTRREF(_result,_attr)                    \
  do {                                                 \
    _result = (Expr *)malloc(sizeof(Expr));            \
    _result->type = EXPR_ATTRREF;                      \
    _result->expr.attrRef = _attr;                     \
  } while(0)

#define MAKE_CONS(_result,_value)                      \
  do {                                                 \
    _result = (Expr *)malloc(sizeof(Expr));            \
    _result->type = EXPR_CONST;                        \
    _result->expr.cons = _value;                       \
  } while(0)

#endif // EXPR_H