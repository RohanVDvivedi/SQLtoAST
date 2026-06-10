#ifndef SQL_EXPRESSION_EVAL_H
#define SQL_EXPRESSION_EVAL_H

#include<sqltoast/sql_expression.h>

int has_sub_query_in_sql_exp(const sql_expression* expr);

#endif