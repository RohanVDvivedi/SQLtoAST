#include<sqltoast/sql_expression.h>

sql_expression* new_unary_sql_expr(sql_expression_type type, sql_expression* unary_val);

sql_expression* new_binary_sql_expr(sql_expression_type type, sql_expression* left, sql_expression* right);

sql_expression* new_between_sql_expr(sql_expression_type type, sql_expression* bounds0, sql_expression* bounds2);

sql_expression* new_in_sql_expr();

void insert_expr_to_in_sql_expr(sql_expression* in_expr, sql_expression* from_val);

sql_expression* new_valued_sql_expr(dstring* value);

void delete_sql_expr(sql_expression* expr);