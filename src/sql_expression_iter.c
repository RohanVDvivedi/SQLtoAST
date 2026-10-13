#include<sqltoast/sql_expression_iter.h>

int iterate_over_sql_expr(const sql_expression* expr, traversal_status (*pre_order_callback)(const sql_expression* expr, void* context), void (*post_order_callback)(const sql_expression* expr, void* context), void* context)
{
	traversal_status status = (pre_order_callback != NULL) ? pre_order_callback(expr, context) : CONTINUE_ITERATION;
	if(status == EXIT_ITERATION)
		goto EXIT_ITERATION_CASE;
	if(status == SKIP_CHILD_ITERATION)
		goto POST_ORDER_CALLBACK_WITH_SUCCESS;

	// if the expression for some reason is NULL, skip iterating it's children
	if(expr == NULL)
		goto POST_ORDER_CALLBACK_WITH_SUCCESS;

	switch(expr->type)
	{
		case SQL_MUL_INV :
		case SQL_NEG :
		case SQL_BITNOT :
		case SQL_LOGNOT :
		{
			if(!iterate_over_sql_expr(expr->unary_of, pre_order_callback, post_order_callback, context))
				goto EXIT_ITERATION_CASE;
			break;
		}

		case SQL_ADD :
		case SQL_SUB :
		case SQL_MUL :
		case SQL_DIV :
		case SQL_MOD :
		{
			if(!iterate_over_sql_expr(expr->left, pre_order_callback, post_order_callback, context))
				goto EXIT_ITERATION_CASE;
			if(!iterate_over_sql_expr(expr->right, pre_order_callback, post_order_callback, context))
				goto EXIT_ITERATION_CASE;
			break;
		}

		case SQL_GT :
		case SQL_GTE :
		case SQL_LT :
		case SQL_LTE :
		case SQL_EQ :
		case SQL_NEQ :
		{
			if(!iterate_over_sql_expr(expr->left, pre_order_callback, post_order_callback, context))
				goto EXIT_ITERATION_CASE;
			if(expr->cmp_rhs_quantfier == SQL_CMP_NONE)
			{
				if(!iterate_over_sql_expr(expr->right, pre_order_callback, post_order_callback, context))
					goto EXIT_ITERATION_CASE;
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
			if(!iterate_over_sql_expr(expr->left, pre_order_callback, post_order_callback, context))
				goto EXIT_ITERATION_CASE;
			if(!iterate_over_sql_expr(expr->right, pre_order_callback, post_order_callback, context))
				goto EXIT_ITERATION_CASE;
			break;
		}

		case SQL_BTWN :
		{
			if(!iterate_over_sql_expr(expr->btwn_input, pre_order_callback, post_order_callback, context))
				goto EXIT_ITERATION_CASE;
			if(!iterate_over_sql_expr(expr->bounds[0], pre_order_callback, post_order_callback, context))
				goto EXIT_ITERATION_CASE;
			if(!iterate_over_sql_expr(expr->bounds[1], pre_order_callback, post_order_callback, context))
				goto EXIT_ITERATION_CASE;
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
				if(!iterate_over_sql_expr(get_from_front_of_arraylist(&(expr->expr_list), i), pre_order_callback, post_order_callback, context))
					goto EXIT_ITERATION_CASE;
			}
			break;
		}

		case SQL_IN :
		{
			if(!iterate_over_sql_expr(expr->in_input, pre_order_callback, post_order_callback, context))
				goto EXIT_ITERATION_CASE;

			for(cy_uint i = 0; i < get_element_count_arraylist(&(expr->in_expr_list)); i++)
			{
				if(!iterate_over_sql_expr(get_from_front_of_arraylist(&(expr->in_expr_list), i), pre_order_callback, post_order_callback, context))
					goto EXIT_ITERATION_CASE;
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
			if(expr->parameter_resolution != NULL) // only when paramerter is resolved
				if(!iterate_over_sql_expr(expr->parameter_resolution, pre_order_callback, post_order_callback, context))
					goto EXIT_ITERATION_CASE;
			break;
		}

		case SQL_FUNCTION_CALL :
		{
			for(cy_uint i = 0; i < get_element_count_arraylist(&(expr->param_expr_list)); i++)
				if(!iterate_over_sql_expr(get_from_front_of_arraylist(&(expr->param_expr_list), i), pre_order_callback, post_order_callback, context))
					goto EXIT_ITERATION_CASE;
			break;
		}

		case SQL_CAST :
		{
			if(!iterate_over_sql_expr(expr->cast_expr, pre_order_callback, post_order_callback, context))
				goto EXIT_ITERATION_CASE;
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
				if(!iterate_over_sql_expr(expr->case_expr, pre_order_callback, post_order_callback, context))
					goto EXIT_ITERATION_CASE;

			for(cy_uint i = 0; i < max(get_element_count_arraylist(&(expr->when_exprs)), get_element_count_arraylist(&(expr->then_exprs))); i++)
			{
				if(i < get_element_count_arraylist(&(expr->when_exprs)))
					if(!iterate_over_sql_expr(get_from_front_of_arraylist(&(expr->when_exprs), i), pre_order_callback, post_order_callback, context))
						goto EXIT_ITERATION_CASE;
				if(i < get_element_count_arraylist(&(expr->then_exprs)))
					if(!iterate_over_sql_expr(get_from_front_of_arraylist(&(expr->then_exprs), i), pre_order_callback, post_order_callback, context))
						goto EXIT_ITERATION_CASE;
			}

			if(expr->else_expr != NULL)
				if(!iterate_over_sql_expr(expr->else_expr, pre_order_callback, post_order_callback, context))
					goto EXIT_ITERATION_CASE;
			break;
		}
	}

	POST_ORDER_CALLBACK_WITH_SUCCESS:;
	if(post_order_callback != NULL)
		post_order_callback(expr, context);
	return 1;

	EXIT_ITERATION_CASE:;
	if(post_order_callback != NULL)
		post_order_callback(expr, context);
	return 0;
}