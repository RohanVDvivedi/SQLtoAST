#include<sqltoast/sql_expression.h>

#include<stdlib.h>
#include<stdio.h>

declarations_value_arraylist(sql_expression_list, sql_expression*, static inline)
#define EXPANSION_FACTOR 1.5
function_definitions_value_arraylist(sql_expression_list, sql_expression*, static inline)

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

sql_expression* new_between_sql_expr(sql_expression* input, sql_expression* bounds0, sql_expression* bounds1)
{
	sql_expression* expr = malloc(sizeof(sql_expression));
	expr->type = SQL_BTWN;
	expr->input = input;
	expr->bounds[0] = bounds0;
	expr->bounds[1] = bounds1;
	return expr;
}

sql_expression* new_flat_sql_expr(sql_expression_type type)
{
	sql_expression* expr = malloc(sizeof(sql_expression));
	expr->type = type;
	initialize_sql_expression_list(&(expr->expr_list), 5);
	return expr;
}

void insert_expr_to_flat_sql_expr(sql_expression* expr, sql_expression* from_val)
{
	if(is_full_sql_expression_list(&(expr->expr_list)) && !expand_sql_expression_list(&(expr->expr_list)))
	{
		printf("failed to insert back in expression list for flat operator\n");
		exit(-1);
	}
	push_back_in_sql_expression_list(&(expr->expr_list), &from_val);
}

sql_expression* new_valued_sql_expr(sql_expression_type type, dstring value)
{
	sql_expression* expr = malloc(sizeof(sql_expression));
	expr->type = type;
	expr->value = value;
	return expr;
}

void print_sql_expr(const sql_expression* expr)
{
	switch(expr->type)
	{
		case SQL_NEG :
		{
			printf("-( ");
			print_sql_expr(expr->unary_of);
			printf(" )");
			break;
		}
		case SQL_BITNOT :
		{
			printf("~( ");
			print_sql_expr(expr->unary_of);
			printf(" )");
			break;
		}
		case SQL_LOGNOT :
		{
			printf("!( ");
			print_sql_expr(expr->unary_of);
			printf(" )");
			break;
		}

		case SQL_ADD :
		{
			printf("( ");
			print_sql_expr(expr->left);
			printf(" + ");
			print_sql_expr(expr->right);
			printf(" )");
			break;
		}
		case SQL_SUB :
		{
			printf("( ");
			print_sql_expr(expr->left);
			printf(" - ");
			print_sql_expr(expr->right);
			printf(" )");
			break;
		}
		case SQL_MUL :
		{
			printf("( ");
			print_sql_expr(expr->left);
			printf(" * ");
			print_sql_expr(expr->right);
			printf(" )");
			break;
		}
		case SQL_DIV :
		{
			printf("( ");
			print_sql_expr(expr->left);
			printf(" / ");
			print_sql_expr(expr->right);
			printf(" )");
			break;
		}
		case SQL_MOD :
		{
			printf("( ");
			print_sql_expr(expr->left);
			printf(" % ");
			print_sql_expr(expr->right);
			printf(" )");
			break;
		}
		case SQL_GT :
		{
			printf("( ");
			print_sql_expr(expr->left);
			printf(" > ");
			print_sql_expr(expr->right);
			printf(" )");
			break;
		}
		case SQL_GTE :
		{
			printf("( ");
			print_sql_expr(expr->left);
			printf(" >= ");
			print_sql_expr(expr->right);
			printf(" )");
			break;
		}
		case SQL_LT :
		{
			printf("( ");
			print_sql_expr(expr->left);
			printf(" < ");
			print_sql_expr(expr->right);
			printf(" )");
			break;
		}
		case SQL_LTE :
		{
			printf("( ");
			print_sql_expr(expr->left);
			printf(" <= ");
			print_sql_expr(expr->right);
			printf(" )");
			break;
		}
		case SQL_EQ :
		{
			printf("( ");
			print_sql_expr(expr->left);
			printf(" = ");
			print_sql_expr(expr->right);
			printf(" )");
			break;
		}
		case SQL_NEQ :
		{
			printf("( ");
			print_sql_expr(expr->left);
			printf(" <> ");
			print_sql_expr(expr->right);
			printf(" )");
			break;
		}
		case SQL_BITAND :
		{
			printf("( ");
			print_sql_expr(expr->left);
			printf(" & ");
			print_sql_expr(expr->right);
			printf(" )");
			break;
		}
		case SQL_BITOR :
		{
			printf("( ");
			print_sql_expr(expr->left);
			printf(" | ");
			print_sql_expr(expr->right);
			printf(" )");
			break;
		}
		case SQL_BITXOR :
		{
			printf("( ");
			print_sql_expr(expr->left);
			printf(" ^ ");
			print_sql_expr(expr->right);
			printf(" )");
			break;
		}
		case SQL_LOGAND :
		{
			printf("( ");
			print_sql_expr(expr->left);
			printf(" AND ");
			print_sql_expr(expr->right);
			printf(" )");
			break;
		}
		case SQL_LOGOR :
		{
			printf("( ");
			print_sql_expr(expr->left);
			printf(" OR ");
			print_sql_expr(expr->right);
			printf(" )");
			break;
		}
		case SQL_LOGXOR :
		{
			printf("( ");
			print_sql_expr(expr->left);
			printf(" XOR ");
			print_sql_expr(expr->right);
			printf(" )");
			break;
		}

		case SQL_BTWN :
		{
			printf("( ");
			print_sql_expr(expr->input);
			printf(" BETWEEN [ ");
			print_sql_expr(expr->bounds[0]);
			printf(" , ");
			print_sql_expr(expr->bounds[1]);
			printf(" ] )");
			break;
		}

		case SQL_IN :
		{
			printf("( IN : (");
			for(cy_uint i = 0; i < get_element_count_for_sql_expression_list(&(expr->expr_list)); i++)
			{
				if(i != 0)
					printf(" , ");
				print_sql_expr(get_element_from_sql_expression_list(&(expr->expr_list), i));
			}
			printf(") )");
			break;
		}

		case SQL_CONST :
		{
			printf_dstring(&(expr->value));
			break;
		}
		case SQL_VAR :
		{
			printf("\"");
			printf_dstring(&(expr->value));
			printf("\"");
			break;
		}
	}
}

void delete_sql_expr(sql_expression* expr)
{
	switch(expr->type)
	{
		case SQL_NEG :
		case SQL_BITNOT :
		case SQL_LOGNOT :
		{
			delete_sql_expr(expr->unary_of);
			break;
		}

		case SQL_ADD :
		case SQL_SUB :
		case SQL_MUL :
		case SQL_DIV :
		case SQL_MOD :
		case SQL_GT :
		case SQL_GTE :
		case SQL_LT :
		case SQL_LTE :
		case SQL_EQ :
		case SQL_NEQ :
		case SQL_BITAND :
		case SQL_BITOR :
		case SQL_BITXOR :
		case SQL_LOGAND :
		case SQL_LOGOR :
		case SQL_LOGXOR :
		{
			delete_sql_expr(expr->left);
			delete_sql_expr(expr->right);
			break;
		}

		case SQL_BTWN :
		{
			delete_sql_expr(expr->bounds[0]);
			delete_sql_expr(expr->bounds[1]);
			break;
		}

		case SQL_IN :
		{
			for(cy_uint i = 0; i < get_element_count_for_sql_expression_list(&(expr->expr_list)); i++)
				delete_sql_expr(get_element_from_sql_expression_list(&(expr->expr_list), i));
			deinitialize_sql_expression_list(&(expr->expr_list));
			break;
		}

		case SQL_CONST :
		case SQL_VAR :
		{
			deinit_dstring(&(expr->value));
			break;
		}
	}
	free(expr);
}