#include<sqltoast/sql_expression.h>

#include<stdlib.h>

sql_expression* new_unary_sql_expr(sql_expression_type type, sql_expression* unary_of)
{
	sql_expression* expr = malloc(sizeof(sql_expression));
	expr->type = type;
	expr->unary_of = unary_of;
	return expr;
}

sql_expression* new_binary_sql_expr(sql_expression_type type, sql_expression* left, sql_expression* right)
{
	sql_expression* expr = malloc(sizeof(sql_expression));
	expr->type = type;
	expr->left = left;
	expr->right = right;
	return expr;
}

sql_expression* new_between_sql_expr(sql_expression* bounds0, sql_expression* bounds1)
{
	sql_expression* expr = malloc(sizeof(sql_expression));
	expr->type = SQL_BTWN;
	expr->bounds[0] = bounds0;
	expr->bounds[1] = bounds1;
	return expr;
}

sql_expression* new_in_sql_expr()
{
	sql_expression* expr = malloc(sizeof(sql_expression));
	expr->type = SQL_IN;
	expr->expr_list_size = 0;
	expr->expr_list = NULL;
	return expr;
}

void insert_expr_to_in_sql_expr(sql_expression* in_expr, sql_expression* from_val)
{
	in_expr->expr_list_size++;
	in_expr->expr_list = realloc(in_expr->expr_list, in_expr->expr_list_size * sizeof(sql_expression*));
	in_expr->expr_list[in_expr->expr_list_size-1] = from_val;
}

sql_expression* new_valued_sql_expr(sql_expression_type type, dstring value)
{
	sql_expression* expr = malloc(sizeof(sql_expression));
	expr->type = type;
	expr->value = value;
	return expr;
}

void delete_sql_expr(sql_expression* expr);