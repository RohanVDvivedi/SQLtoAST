#include<sqltoast/sql_expression_iter.h>

// if returns 0 exit immediately
int iterate_over_sql_expr_recurse(const sql_expression* expr, traversal_status (*pre_order_callback)(const sql_expression* expr, void* context), void (*post_order_callback)(const sql_expression* expr, void* context), void* context)
{
	traversal_status status = (pre_order_callback != NULL) ? pre_order_callback(expr, context) : CONTINUE_ITERATION;
	if(status == EXIT_ITERATION)
		return 0;
	if(status == EXIT_EXPRESSION)
		return 1;
	if(status == SKIP_CHILD_ITERATION)
		goto POST_ORDER_CALL;

	switch(expr->type)
	{
		case SQL_MUL_INV :
		case SQL_NEG :
		case SQL_BITNOT :
		case SQL_LOGNOT :
		{
			if(!iterate_over_sql_expr_recurse(expr->unary_of, pre_order_callback, post_order_callback, context))
				return 0;
			break;
		}

		case SQL_ADD :
		case SQL_SUB :
		case SQL_MUL :
		case SQL_DIV :
		case SQL_MOD :
		{
			if(!iterate_over_sql_expr_recurse(expr->left, pre_order_callback, post_order_callback, context))
				return 0;
			if(!iterate_over_sql_expr_recurse(expr->right, pre_order_callback, post_order_callback, context))
				return 0;
			break;
		}

		case SQL_GT :
		case SQL_GTE :
		case SQL_LT :
		case SQL_LTE :
		case SQL_EQ :
		case SQL_NEQ :
		{
			if(!iterate_over_sql_expr_recurse(expr->left, pre_order_callback, post_order_callback, context))
				return 0;
			if(expr->cmp_rhs_quantfier == SQL_CMP_NONE)
			{
				if(!iterate_over_sql_expr_recurse(expr->right, pre_order_callback, post_order_callback, context))
					return 0;
			}
			break;
		}

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
		{
			if(!iterate_over_sql_expr_recurse(expr->left, pre_order_callback, post_order_callback, context))
				return 0;
			if(!iterate_over_sql_expr_recurse(expr->right, pre_order_callback, post_order_callback, context))
				return 0;
			break;
		}

		case SQL_BTWN :
		{
			if(!iterate_over_sql_expr_recurse(expr->btwn_input, pre_order_callback, post_order_callback, context))
				return 0;
			if(!iterate_over_sql_expr_recurse(expr->bounds[0], pre_order_callback, post_order_callback, context))
				return 0;
			if(!iterate_over_sql_expr_recurse(expr->bounds[1], pre_order_callback, post_order_callback, context))
				return 0;
			break;
		}

		case SQL_ADD_FLAT :
		case SQL_MUL_FLAT :
		case SQL_LOGAND_FLAT :
		case SQL_LOGOR_FLAT :
		case SQL_LOGXOR_FLAT :
		case SQL_CONCAT_FLAT :
		{
			for(cy_uint i = 0; i < get_element_count_arraylist(&(expr->expr_list)); i++)
			{
				if(!iterate_over_sql_expr_recurse(get_from_front_of_arraylist(&(expr->expr_list), i), pre_order_callback, post_order_callback, context))
					return 0;
			}
			break;
		}

		case SQL_IN :
		{
			if(!iterate_over_sql_expr_recurse(expr->in_input, pre_order_callback, post_order_callback, context))
				return 0;

			for(cy_uint i = 0; i < get_element_count_arraylist(&(expr->in_expr_list)); i++)
			{
				if(!iterate_over_sql_expr_recurse(get_from_front_of_arraylist(&(expr->in_expr_list), i), pre_order_callback, post_order_callback, context))
					return 0;
			}
			break;
		}

		case SQL_STR :
		case SQL_NUM :
		case SQL_VAR :
		{
			break;
		}

		case SQL_TRUE :
		case SQL_FALSE :
		case SQL_UNKNOWN :
		case SQL_NULL :
		{
			break;
		}

		case SQL_PARAMETER :
		{
			if(expr->parameter_resolution != NULL) // only when both paramerters is resolved
				if(!iterate_over_sql_expr_recurse(expr->parameter_resolution, pre_order_callback, post_order_callback, context))
					return 0;
			break;
		}

		case SQL_FUNCTION_CALL :
		{
			for(cy_uint i = 0; i < get_element_count_arraylist(&(expr->param_expr_list)); i++)
				if(!iterate_over_sql_expr_recurse(get_from_front_of_arraylist(&(expr->param_expr_list), i), pre_order_callback, post_order_callback, context))
					return 0;
			break;
		}

		case SQL_CAST :
		{
			if(!iterate_over_sql_expr_recurse(expr->cast_expr, pre_order_callback, post_order_callback, context))
				return 0;
			break;
		}

		case SQL_SUB_QUERY :
		case SQL_EXISTS :
		{
			break;
		}

		case SQL_CASE :
		{
			if(expr->case_expr != NULL)
				if(!iterate_over_sql_expr_recurse(expr->case_expr, pre_order_callback, post_order_callback, context))
					return 0;

			for(cy_uint i = 0; i < get_element_count_arraylist(&(expr->when_exprs)); i++)
				if(!iterate_over_sql_expr_recurse(get_from_front_of_arraylist(&(expr->when_exprs), i), pre_order_callback, post_order_callback, context))
					return 0;

			for(cy_uint i = 0; i < get_element_count_arraylist(&(expr->then_exprs)); i++)
				if(!iterate_over_sql_expr_recurse(get_from_front_of_arraylist(&(expr->then_exprs), i), pre_order_callback, post_order_callback, context))
					return 0;

			if(!iterate_over_sql_expr_recurse(expr->else_expr, pre_order_callback, post_order_callback, context))
				return 0;
			break;
		}
	}

	POST_ORDER_CALL:;
	if(post_order_callback != NULL)
		post_order_callback(expr, context);

	return 1;
}

void iterate_over_sql_expr(const sql_expression* expr, traversal_status (*pre_order_callback)(const sql_expression* expr, void* context), void (*post_order_callback)(const sql_expression* expr, void* context), void* context)
{
	iterate_over_sql_expr_recurse(expr, pre_order_callback, post_order_callback, context);
}