#include<sqltoast/sql_expression_eval.h>

int has_sub_query_in_sql_exp(const sql_expression* expr)
{
	switch(expr->type)
	{
		case SQL_MUL_INV :
		case SQL_NEG :
		case SQL_BITNOT :
		case SQL_LOGNOT :
			return has_sub_query_in_sql_exp(expr->unary_of);

		case SQL_ADD :
		case SQL_SUB :
		case SQL_MUL :
		case SQL_DIV :
		case SQL_MOD :
		case SQL_BITAND :
		case SQL_BITOR :
		case SQL_BITXOR :
		case SQL_LOGAND :
		case SQL_LOGOR :
		case SQL_LOGXOR :
		case SQL_LSHIFT :
		case SQL_RSHIFT :
		case SQL_CONCAT :
		case SQL_LIKE :
		case SQL_IS :
			return has_sub_query_in_sql_exp(expr->left) || has_sub_query_in_sql_exp(expr->right);

		case SQL_GT :
		case SQL_GTE :
		case SQL_LT :
		case SQL_LTE :
		case SQL_EQ :
		case SQL_NEQ :
		{
			if(expr->cmp_rhs_quantfier == SQL_CMP_NONE)
				return has_sub_query_in_sql_exp(expr->left) || has_sub_query_in_sql_exp(expr->right);
			else
				return 1; // ANY and ALL -> RHS will always be a sub_query
		}

		case SQL_BTWN :
			return has_sub_query_in_sql_exp(expr->btwn_input) || has_sub_query_in_sql_exp(expr->bounds[0]) || has_sub_query_in_sql_exp(expr->bounds[1]);

		case SQL_ADD_FLAT :
		case SQL_MUL_FLAT :
		case SQL_LOGAND_FLAT :
		case SQL_LOGOR_FLAT :
		case SQL_LOGXOR_FLAT :
		case SQL_CONCAT_FLAT :
		{
			for(cy_uint i = 0; i < get_element_count_arraylist(&(expr->expr_list)); i++)
				if(has_sub_query_in_sql_exp((const sql_expression*)get_from_front_of_arraylist(&(expr->expr_list), i)))
					return 1;
			return 0;
		}

		case SQL_IN :
		{
			if(expr->in_sub_query) // IN follows a sub_query
				return 1;
			for(cy_uint i = 0; i < get_element_count_arraylist(&(expr->in_expr_list)); i++)
				if(has_sub_query_in_sql_exp((const sql_expression*)get_from_front_of_arraylist(&(expr->in_expr_list), i)))
					return 1;
			return has_sub_query_in_sql_exp(expr->in_input);
		}

		case SQL_NUM :
		case SQL_STR :
		case SQL_VAR :
			return 0;

		case SQL_TRUE :
		case SQL_FALSE :
		case SQL_UNKNOWN :
		case SQL_NULL :
			return 0;

		case SQL_FUNCTION_CALL :
		{
			for(cy_uint i = 0; i < get_element_count_arraylist(&(expr->param_expr_list)); i++)
				if(has_sub_query_in_sql_exp((const sql_expression*)get_from_front_of_arraylist(&(expr->param_expr_list), i)))
					return 1;
			return 0;
		}

		case SQL_CAST :
			return has_sub_query_in_sql_exp(expr->cast_expr);

		case SQL_SUB_QUERY :
		case SQL_EXISTS :
			return 1;

		case SQL_CASE :
		{
			if(expr->case_expr)
				if(has_sub_query_in_sql_exp(expr->case_expr))
					return 1;

			for(cy_uint i = 0; i < get_element_count_arraylist(&(expr->when_exprs)); i++)
			{
				if(has_sub_query_in_sql_exp((const sql_expression*) get_from_front_of_arraylist(&(expr->when_exprs), i)))
					return 1;
			}
			for(cy_uint i = 0; i < get_element_count_arraylist(&(expr->then_exprs)); i++)
			{
				if(has_sub_query_in_sql_exp((const sql_expression*) get_from_front_of_arraylist(&(expr->then_exprs), i)))
					return 1;
			}
			if(expr->else_expr)
				if(has_sub_query_in_sql_exp(expr->else_expr))
					return 1;
			return 0;
		}
	}

	return 0;
}