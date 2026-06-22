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

static void delete_data_internal(void* a, const sql_expr_eval_context* ec_p)
{
	if(a == NULL)
		return;
	if(a == ec_p->true_bool || a == ec_p->false_bool || a == ec_p->unknown_bool)
		return;
	if(a == ec_p->minus_one_number || a == ec_p->zero_number || a == ec_p->one_number)
		return;
	ec_p->delete_data(a, ec_p);
}

void* evaluate_sql_expr(const sql_expression* expr, const sql_expr_eval_context* ec_p, int* error_code)
{
	dstring str;
	dstring* str_p = &str;
	init_empty_dstring(str_p, 0);

	switch(expr->type)
	{
		case SQL_MUL_INV :
		{
			void* a = evaluate_sql_expr(expr->unary_of, ec_p, error_code);
			if(*error_code)
				return NULL;
			if(a == NULL || a == ec_p->unknown_bool)
				return ec_p->unknown_bool;

			void* res = ec_p->div(ec_p->one_number, a, ec_p, error_code);
			delete_data_internal(a, ec_p);
			if(*error_code)
				return NULL;

			return res;
		}
		case SQL_NEG :
		{
			void* a = evaluate_sql_expr(expr->unary_of, ec_p, error_code);
			if(*error_code)
				return NULL;
			if(a == NULL || a == ec_p->unknown_bool)
				return ec_p->unknown_bool;

			void* res = ec_p->sub(ec_p->zero_number, a, ec_p, error_code);
			delete_data_internal(a, ec_p);
			if(*error_code)
				return NULL;

			return res;
		}
		case SQL_BITNOT :
		{
			void* a = evaluate_sql_expr(expr->unary_of, ec_p, error_code);
			if(*error_code)
				return NULL;
			if(a == NULL || a == ec_p->unknown_bool)
				return ec_p->unknown_bool;

			void* res = ec_p->bit_not(a, ec_p, error_code);
			delete_data_internal(a, ec_p);
			if(*error_code)
				return NULL;

			return res;
		}
		case SQL_LOGNOT :
		{
			void* a = evaluate_sql_expr(expr->unary_of, ec_p, error_code);
			if(*error_code)
				return NULL;
			if(a == NULL || a == ec_p->unknown_bool)
				return ec_p->unknown_bool;

			void* res = ec_p->get_bool(a, ec_p, error_code);
			delete_data_internal(a, ec_p);
			if(*error_code)
				return NULL;

			if(res == ec_p->true_bool)
				return ec_p->false_bool;
			else if(res == ec_p->false_bool)
				return ec_p->true_bool;
			else
				return ec_p->unknown_bool;
		}

		case SQL_ADD :
		{
			void* a = evaluate_sql_expr(expr->left, ec_p, error_code);
			if(*error_code)
				return NULL;
			if(a == NULL || a == ec_p->unknown_bool)
				return ec_p->unknown_bool;

			void* b = evaluate_sql_expr(expr->right, ec_p, error_code);
			if(*error_code)
			{
				delete_data_internal(a, ec_p);
				return NULL;
			}
			if(b == NULL || b == ec_p->unknown_bool)
			{
				delete_data_internal(a, ec_p);
				return ec_p->unknown_bool;
			}

			void* res = ec_p->add(a, b, ec_p, error_code);
			delete_data_internal(a, ec_p);
			delete_data_internal(b, ec_p);
			if(*error_code)
				return NULL;

			return res;
		}
		case SQL_SUB :
		{
			void* a = evaluate_sql_expr(expr->left, ec_p, error_code);
			if(*error_code)
				return NULL;
			if(a == NULL || a == ec_p->unknown_bool)
				return ec_p->unknown_bool;

			void* b = evaluate_sql_expr(expr->right, ec_p, error_code);
			if(*error_code)
			{
				delete_data_internal(a, ec_p);
				return NULL;
			}
			if(b == NULL || b == ec_p->unknown_bool)
			{
				delete_data_internal(a, ec_p);
				return ec_p->unknown_bool;
			}

			void* res = ec_p->sub(a, b, ec_p, error_code);
			delete_data_internal(a, ec_p);
			delete_data_internal(b, ec_p);
			if(*error_code)
				return NULL;

			return res;
		}
		case SQL_MUL :
		{
			void* a = evaluate_sql_expr(expr->left, ec_p, error_code);
			if(*error_code)
				return NULL;
			if(a == NULL || a == ec_p->unknown_bool)
				return ec_p->unknown_bool;

			void* b = evaluate_sql_expr(expr->right, ec_p, error_code);
			if(*error_code)
			{
				delete_data_internal(a, ec_p);
				return NULL;
			}
			if(b == NULL || b == ec_p->unknown_bool)
			{
				delete_data_internal(a, ec_p);
				return ec_p->unknown_bool;
			}

			void* res = ec_p->mul(a, b, ec_p, error_code);
			delete_data_internal(a, ec_p);
			delete_data_internal(b, ec_p);
			if(*error_code)
				return NULL;

			return res;
		}
		case SQL_DIV :
		{
			void* a = evaluate_sql_expr(expr->left, ec_p, error_code);
			if(*error_code)
				return NULL;
			if(a == NULL || a == ec_p->unknown_bool)
				return ec_p->unknown_bool;

			void* b = evaluate_sql_expr(expr->right, ec_p, error_code);
			if(*error_code)
			{
				delete_data_internal(a, ec_p);
				return NULL;
			}
			if(b == NULL || b == ec_p->unknown_bool)
			{
				delete_data_internal(a, ec_p);
				return ec_p->unknown_bool;
			}

			void* res = ec_p->div(a, b, ec_p, error_code);
			delete_data_internal(a, ec_p);
			delete_data_internal(b, ec_p);
			if(*error_code)
				return NULL;

			return res;
		}
		case SQL_MOD :
		{
			void* a = evaluate_sql_expr(expr->left, ec_p, error_code);
			if(*error_code)
				return NULL;
			if(a == NULL || a == ec_p->unknown_bool)
				return ec_p->unknown_bool;

			void* b = evaluate_sql_expr(expr->right, ec_p, error_code);
			if(*error_code)
			{
				delete_data_internal(a, ec_p);
				return NULL;
			}
			if(b == NULL || b == ec_p->unknown_bool)
			{
				delete_data_internal(a, ec_p);
				return ec_p->unknown_bool;
			}

			void* res = ec_p->mod(a, b, ec_p, error_code);
			delete_data_internal(a, ec_p);
			delete_data_internal(b, ec_p);
			if(*error_code)
				return NULL;

			return res;
		}
		case SQL_GT :
		{
			void* a = evaluate_sql_expr(expr->left, ec_p, error_code);
			if(*error_code)
				return NULL;
			if(a == NULL || a == ec_p->unknown_bool)
				return ec_p->unknown_bool;

			switch(expr->cmp_rhs_quantfier)
			{
				default :
				case SQL_CMP_NONE :
				{
					void* b = evaluate_sql_expr(expr->right, ec_p, error_code);
					if(*error_code)
					{
						delete_data_internal(a, ec_p);
						return NULL;
					}
					if(b == NULL || b == ec_p->unknown_bool)
					{
						delete_data_internal(a, ec_p);
						return ec_p->unknown_bool;
					}

					int a_minus_b = ec_p->compare(a, b, ec_p, error_code);
					delete_data_internal(a, ec_p);
					delete_data_internal(b, ec_p);
					if(*error_code)
						return NULL;

					if(a_minus_b > 0)
						return ec_p->true_bool;
					return ec_p->false_bool;
				}
				case SQL_CMP_ANY :
				{
					void* sub_query = ec_p->get_sub_query(expr->right_sub_query, ec_p, error_code);
					if(*error_code)
					{
						delete_data_internal(a, ec_p);
						return NULL;
					}

					void* res = ec_p->false_bool;
					int end_of_results = 0;
					while(res != ec_p->true_bool)
					{
						void* b = ec_p->next_data_from_sub_query(sub_query, &end_of_results, ec_p, error_code);
						if(*error_code)
						{
							ec_p->delete_sub_query(sub_query, ec_p);
							delete_data_internal(a, ec_p);
							return NULL;
						}
						if(end_of_results)
							break;

						void* curr_res = ec_p->unknown_bool;
						if(b != NULL && b != ec_p->unknown_bool)
						{
							int a_minus_b = ec_p->compare(a, b, ec_p, error_code);
							delete_data_internal(b, ec_p);
							if(*error_code)
							{
								ec_p->delete_sub_query(sub_query, ec_p);
								delete_data_internal(a, ec_p);
								return NULL;
							}
							curr_res = (a_minus_b > 0) ? ec_p->true_bool : ec_p->false_bool;
						}

						if(res == ec_p->true_bool || curr_res == ec_p->true_bool)
						{
							res = ec_p->true_bool;
							continue;
						}

						if(res == ec_p->unknown_bool || curr_res == ec_p->unknown_bool)
						{
							res = ec_p->unknown_bool;
							continue;
						}
					}

					ec_p->delete_sub_query(sub_query, ec_p);
					delete_data_internal(a, ec_p);

					return res;
				}
				case SQL_CMP_ALL :
				{
					void* sub_query = ec_p->get_sub_query(expr->right_sub_query, ec_p, error_code);
					if(*error_code)
					{
						delete_data_internal(a, ec_p);
						return NULL;
					}

					void* res = ec_p->true_bool;
					int end_of_results = 0;
					while(res != ec_p->false_bool)
					{
						void* b = ec_p->next_data_from_sub_query(sub_query, &end_of_results, ec_p, error_code);
						if(*error_code)
						{
							ec_p->delete_sub_query(sub_query, ec_p);
							delete_data_internal(a, ec_p);
							return NULL;
						}
						if(end_of_results)
							break;

						void* curr_res = ec_p->unknown_bool;
						if(b != NULL && b != ec_p->unknown_bool)
						{
							int a_minus_b = ec_p->compare(a, b, ec_p, error_code);
							delete_data_internal(b, ec_p);
							if(*error_code)
							{
								ec_p->delete_sub_query(sub_query, ec_p);
								delete_data_internal(a, ec_p);
								return NULL;
							}
							curr_res = (a_minus_b > 0) ? ec_p->true_bool : ec_p->false_bool;
						}

						if(res == ec_p->false_bool || curr_res == ec_p->false_bool)
						{
							res = ec_p->false_bool;
							continue;
						}

						if(res == ec_p->unknown_bool || curr_res == ec_p->unknown_bool)
						{
							res = ec_p->unknown_bool;
							continue;
						}
					}

					ec_p->delete_sub_query(sub_query, ec_p);
					delete_data_internal(a, ec_p);

					return res;
				}
			}
		}
		case SQL_GTE :
		{
			void* a = evaluate_sql_expr(expr->left, ec_p, error_code);
			if(*error_code)
				return NULL;
			if(a == NULL || a == ec_p->unknown_bool)
				return ec_p->unknown_bool;

			switch(expr->cmp_rhs_quantfier)
			{
				default :
				case SQL_CMP_NONE :
				{
					void* b = evaluate_sql_expr(expr->right, ec_p, error_code);
					if(*error_code)
					{
						delete_data_internal(a, ec_p);
						return NULL;
					}
					if(b == NULL || b == ec_p->unknown_bool)
					{
						delete_data_internal(a, ec_p);
						return ec_p->unknown_bool;
					}

					int a_minus_b = ec_p->compare(a, b, ec_p, error_code);
					delete_data_internal(a, ec_p);
					delete_data_internal(b, ec_p);
					if(*error_code)
						return NULL;

					if(a_minus_b >= 0)
						return ec_p->true_bool;
					return ec_p->false_bool;
				}
				case SQL_CMP_ANY :
				{
					void* sub_query = ec_p->get_sub_query(expr->right_sub_query, ec_p, error_code);
					if(*error_code)
					{
						delete_data_internal(a, ec_p);
						return NULL;
					}

					void* res = ec_p->false_bool;
					int end_of_results = 0;
					while(res != ec_p->true_bool)
					{
						void* b = ec_p->next_data_from_sub_query(sub_query, &end_of_results, ec_p, error_code);
						if(*error_code)
						{
							ec_p->delete_sub_query(sub_query, ec_p);
							delete_data_internal(a, ec_p);
							return NULL;
						}
						if(end_of_results)
							break;

						void* curr_res = ec_p->unknown_bool;
						if(b != NULL && b != ec_p->unknown_bool)
						{
							int a_minus_b = ec_p->compare(a, b, ec_p, error_code);
							delete_data_internal(b, ec_p);
							if(*error_code)
							{
								ec_p->delete_sub_query(sub_query, ec_p);
								delete_data_internal(a, ec_p);
								return NULL;
							}
							curr_res = (a_minus_b >= 0) ? ec_p->true_bool : ec_p->false_bool;
						}

						if(res == ec_p->true_bool || curr_res == ec_p->true_bool)
						{
							res = ec_p->true_bool;
							continue;
						}

						if(res == ec_p->unknown_bool || curr_res == ec_p->unknown_bool)
						{
							res = ec_p->unknown_bool;
							continue;
						}
					}

					ec_p->delete_sub_query(sub_query, ec_p);
					delete_data_internal(a, ec_p);

					return res;
				}
				case SQL_CMP_ALL :
				{
					void* sub_query = ec_p->get_sub_query(expr->right_sub_query, ec_p, error_code);
					if(*error_code)
					{
						delete_data_internal(a, ec_p);
						return NULL;
					}

					void* res = ec_p->true_bool;
					int end_of_results = 0;
					while(res != ec_p->false_bool)
					{
						void* b = ec_p->next_data_from_sub_query(sub_query, &end_of_results, ec_p, error_code);
						if(*error_code)
						{
							ec_p->delete_sub_query(sub_query, ec_p);
							delete_data_internal(a, ec_p);
							return NULL;
						}
						if(end_of_results)
							break;

						void* curr_res = ec_p->unknown_bool;
						if(b != NULL && b != ec_p->unknown_bool)
						{
							int a_minus_b = ec_p->compare(a, b, ec_p, error_code);
							delete_data_internal(b, ec_p);
							if(*error_code)
							{
								ec_p->delete_sub_query(sub_query, ec_p);
								delete_data_internal(a, ec_p);
								return NULL;
							}
							curr_res = (a_minus_b >= 0) ? ec_p->true_bool : ec_p->false_bool;
						}

						if(res == ec_p->false_bool || curr_res == ec_p->false_bool)
						{
							res = ec_p->false_bool;
							continue;
						}

						if(res == ec_p->unknown_bool || curr_res == ec_p->unknown_bool)
						{
							res = ec_p->unknown_bool;
							continue;
						}
					}

					ec_p->delete_sub_query(sub_query, ec_p);
					delete_data_internal(a, ec_p);

					return res;
				}
			}
		}
		case SQL_LT :
		{
			void* a = evaluate_sql_expr(expr->left, ec_p, error_code);
			if(*error_code)
				return NULL;
			if(a == NULL || a == ec_p->unknown_bool)
				return ec_p->unknown_bool;

			switch(expr->cmp_rhs_quantfier)
			{
				default :
				case SQL_CMP_NONE :
				{
					void* b = evaluate_sql_expr(expr->right, ec_p, error_code);
					if(*error_code)
					{
						delete_data_internal(a, ec_p);
						return NULL;
					}
					if(b == NULL || b == ec_p->unknown_bool)
					{
						delete_data_internal(a, ec_p);
						return ec_p->unknown_bool;
					}

					int a_minus_b = ec_p->compare(a, b, ec_p, error_code);
					delete_data_internal(a, ec_p);
					delete_data_internal(b, ec_p);
					if(*error_code)
						return NULL;

					if(a_minus_b < 0)
						return ec_p->true_bool;
					return ec_p->false_bool;
				}
				case SQL_CMP_ANY :
				{
					void* sub_query = ec_p->get_sub_query(expr->right_sub_query, ec_p, error_code);
					if(*error_code)
					{
						delete_data_internal(a, ec_p);
						return NULL;
					}

					void* res = ec_p->false_bool;
					int end_of_results = 0;
					while(res != ec_p->true_bool)
					{
						void* b = ec_p->next_data_from_sub_query(sub_query, &end_of_results, ec_p, error_code);
						if(*error_code)
						{
							ec_p->delete_sub_query(sub_query, ec_p);
							delete_data_internal(a, ec_p);
							return NULL;
						}
						if(end_of_results)
							break;

						void* curr_res = ec_p->unknown_bool;
						if(b != NULL && b != ec_p->unknown_bool)
						{
							int a_minus_b = ec_p->compare(a, b, ec_p, error_code);
							delete_data_internal(b, ec_p);
							if(*error_code)
							{
								ec_p->delete_sub_query(sub_query, ec_p);
								delete_data_internal(a, ec_p);
								return NULL;
							}
							curr_res = (a_minus_b < 0) ? ec_p->true_bool : ec_p->false_bool;
						}

						if(res == ec_p->true_bool || curr_res == ec_p->true_bool)
						{
							res = ec_p->true_bool;
							continue;
						}

						if(res == ec_p->unknown_bool || curr_res == ec_p->unknown_bool)
						{
							res = ec_p->unknown_bool;
							continue;
						}
					}

					ec_p->delete_sub_query(sub_query, ec_p);
					delete_data_internal(a, ec_p);

					return res;
				}
				case SQL_CMP_ALL :
				{
					void* sub_query = ec_p->get_sub_query(expr->right_sub_query, ec_p, error_code);
					if(*error_code)
					{
						delete_data_internal(a, ec_p);
						return NULL;
					}

					void* res = ec_p->true_bool;
					int end_of_results = 0;
					while(res != ec_p->false_bool)
					{
						void* b = ec_p->next_data_from_sub_query(sub_query, &end_of_results, ec_p, error_code);
						if(*error_code)
						{
							ec_p->delete_sub_query(sub_query, ec_p);
							delete_data_internal(a, ec_p);
							return NULL;
						}
						if(end_of_results)
							break;

						void* curr_res = ec_p->unknown_bool;
						if(b != NULL && b != ec_p->unknown_bool)
						{
							int a_minus_b = ec_p->compare(a, b, ec_p, error_code);
							delete_data_internal(b, ec_p);
							if(*error_code)
							{
								ec_p->delete_sub_query(sub_query, ec_p);
								delete_data_internal(a, ec_p);
								return NULL;
							}
							curr_res = (a_minus_b < 0) ? ec_p->true_bool : ec_p->false_bool;
						}

						if(res == ec_p->false_bool || curr_res == ec_p->false_bool)
						{
							res = ec_p->false_bool;
							continue;
						}

						if(res == ec_p->unknown_bool || curr_res == ec_p->unknown_bool)
						{
							res = ec_p->unknown_bool;
							continue;
						}
					}

					ec_p->delete_sub_query(sub_query, ec_p);
					delete_data_internal(a, ec_p);

					return res;
				}
			}
		}
		case SQL_LTE :
		{
			void* a = evaluate_sql_expr(expr->left, ec_p, error_code);
			if(*error_code)
				return NULL;
			if(a == NULL || a == ec_p->unknown_bool)
				return ec_p->unknown_bool;

			switch(expr->cmp_rhs_quantfier)
			{
				default :
				case SQL_CMP_NONE :
				{
					void* b = evaluate_sql_expr(expr->right, ec_p, error_code);
					if(*error_code)
					{
						delete_data_internal(a, ec_p);
						return NULL;
					}
					if(b == NULL || b == ec_p->unknown_bool)
					{
						delete_data_internal(a, ec_p);
						return ec_p->unknown_bool;
					}

					int a_minus_b = ec_p->compare(a, b, ec_p, error_code);
					delete_data_internal(a, ec_p);
					delete_data_internal(b, ec_p);
					if(*error_code)
						return NULL;

					if(a_minus_b <= 0)
						return ec_p->true_bool;
					return ec_p->false_bool;
				}
				case SQL_CMP_ANY :
				{
					void* sub_query = ec_p->get_sub_query(expr->right_sub_query, ec_p, error_code);
					if(*error_code)
					{
						delete_data_internal(a, ec_p);
						return NULL;
					}

					void* res = ec_p->false_bool;
					int end_of_results = 0;
					while(res != ec_p->true_bool)
					{
						void* b = ec_p->next_data_from_sub_query(sub_query, &end_of_results, ec_p, error_code);
						if(*error_code)
						{
							ec_p->delete_sub_query(sub_query, ec_p);
							delete_data_internal(a, ec_p);
							return NULL;
						}
						if(end_of_results)
							break;

						void* curr_res = ec_p->unknown_bool;
						if(b != NULL && b != ec_p->unknown_bool)
						{
							int a_minus_b = ec_p->compare(a, b, ec_p, error_code);
							delete_data_internal(b, ec_p);
							if(*error_code)
							{
								ec_p->delete_sub_query(sub_query, ec_p);
								delete_data_internal(a, ec_p);
								return NULL;
							}
							curr_res = (a_minus_b <= 0) ? ec_p->true_bool : ec_p->false_bool;
						}

						if(res == ec_p->true_bool || curr_res == ec_p->true_bool)
						{
							res = ec_p->true_bool;
							continue;
						}

						if(res == ec_p->unknown_bool || curr_res == ec_p->unknown_bool)
						{
							res = ec_p->unknown_bool;
							continue;
						}
					}

					ec_p->delete_sub_query(sub_query, ec_p);
					delete_data_internal(a, ec_p);

					return res;
				}
				case SQL_CMP_ALL :
				{
					void* sub_query = ec_p->get_sub_query(expr->right_sub_query, ec_p, error_code);
					if(*error_code)
					{
						delete_data_internal(a, ec_p);
						return NULL;
					}

					void* res = ec_p->true_bool;
					int end_of_results = 0;
					while(res != ec_p->false_bool)
					{
						void* b = ec_p->next_data_from_sub_query(sub_query, &end_of_results, ec_p, error_code);
						if(*error_code)
						{
							ec_p->delete_sub_query(sub_query, ec_p);
							delete_data_internal(a, ec_p);
							return NULL;
						}
						if(end_of_results)
							break;

						void* curr_res = ec_p->unknown_bool;
						if(b != NULL && b != ec_p->unknown_bool)
						{
							int a_minus_b = ec_p->compare(a, b, ec_p, error_code);
							delete_data_internal(b, ec_p);
							if(*error_code)
							{
								ec_p->delete_sub_query(sub_query, ec_p);
								delete_data_internal(a, ec_p);
								return NULL;
							}
							curr_res = (a_minus_b <= 0) ? ec_p->true_bool : ec_p->false_bool;
						}

						if(res == ec_p->false_bool || curr_res == ec_p->false_bool)
						{
							res = ec_p->false_bool;
							continue;
						}

						if(res == ec_p->unknown_bool || curr_res == ec_p->unknown_bool)
						{
							res = ec_p->unknown_bool;
							continue;
						}
					}

					ec_p->delete_sub_query(sub_query, ec_p);
					delete_data_internal(a, ec_p);

					return res;
				}
			}
		}
		case SQL_EQ :
		{
			void* a = evaluate_sql_expr(expr->left, ec_p, error_code);
			if(*error_code)
				return NULL;
			if(a == NULL || a == ec_p->unknown_bool)
				return ec_p->unknown_bool;

			switch(expr->cmp_rhs_quantfier)
			{
				default :
				case SQL_CMP_NONE :
				{
					void* b = evaluate_sql_expr(expr->right, ec_p, error_code);
					if(*error_code)
					{
						delete_data_internal(a, ec_p);
						return NULL;
					}
					if(b == NULL || b == ec_p->unknown_bool)
					{
						delete_data_internal(a, ec_p);
						return ec_p->unknown_bool;
					}

					int a_minus_b = ec_p->compare(a, b, ec_p, error_code);
					delete_data_internal(a, ec_p);
					delete_data_internal(b, ec_p);
					if(*error_code)
						return NULL;

					if(a_minus_b == 0)
						return ec_p->true_bool;
					return ec_p->false_bool;
				}
				case SQL_CMP_ANY :
				{
					void* sub_query = ec_p->get_sub_query(expr->right_sub_query, ec_p, error_code);
					if(*error_code)
					{
						delete_data_internal(a, ec_p);
						return NULL;
					}

					void* res = ec_p->false_bool;
					int end_of_results = 0;
					while(res != ec_p->true_bool)
					{
						void* b = ec_p->next_data_from_sub_query(sub_query, &end_of_results, ec_p, error_code);
						if(*error_code)
						{
							ec_p->delete_sub_query(sub_query, ec_p);
							delete_data_internal(a, ec_p);
							return NULL;
						}
						if(end_of_results)
							break;

						void* curr_res = ec_p->unknown_bool;
						if(b != NULL && b != ec_p->unknown_bool)
						{
							int a_minus_b = ec_p->compare(a, b, ec_p, error_code);
							delete_data_internal(b, ec_p);
							if(*error_code)
							{
								ec_p->delete_sub_query(sub_query, ec_p);
								delete_data_internal(a, ec_p);
								return NULL;
							}
							curr_res = (a_minus_b == 0) ? ec_p->true_bool : ec_p->false_bool;
						}

						if(res == ec_p->true_bool || curr_res == ec_p->true_bool)
						{
							res = ec_p->true_bool;
							continue;
						}

						if(res == ec_p->unknown_bool || curr_res == ec_p->unknown_bool)
						{
							res = ec_p->unknown_bool;
							continue;
						}
					}

					ec_p->delete_sub_query(sub_query, ec_p);
					delete_data_internal(a, ec_p);

					return res;
				}
				case SQL_CMP_ALL :
				{
					void* sub_query = ec_p->get_sub_query(expr->right_sub_query, ec_p, error_code);
					if(*error_code)
					{
						delete_data_internal(a, ec_p);
						return NULL;
					}

					void* res = ec_p->true_bool;
					int end_of_results = 0;
					while(res != ec_p->false_bool)
					{
						void* b = ec_p->next_data_from_sub_query(sub_query, &end_of_results, ec_p, error_code);
						if(*error_code)
						{
							ec_p->delete_sub_query(sub_query, ec_p);
							delete_data_internal(a, ec_p);
							return NULL;
						}
						if(end_of_results)
							break;

						void* curr_res = ec_p->unknown_bool;
						if(b != NULL && b != ec_p->unknown_bool)
						{
							int a_minus_b = ec_p->compare(a, b, ec_p, error_code);
							delete_data_internal(b, ec_p);
							if(*error_code)
							{
								ec_p->delete_sub_query(sub_query, ec_p);
								delete_data_internal(a, ec_p);
								return NULL;
							}
							curr_res = (a_minus_b == 0) ? ec_p->true_bool : ec_p->false_bool;
						}

						if(res == ec_p->false_bool || curr_res == ec_p->false_bool)
						{
							res = ec_p->false_bool;
							continue;
						}

						if(res == ec_p->unknown_bool || curr_res == ec_p->unknown_bool)
						{
							res = ec_p->unknown_bool;
							continue;
						}
					}

					ec_p->delete_sub_query(sub_query, ec_p);
					delete_data_internal(a, ec_p);

					return res;
				}
			}
		}
		case SQL_NEQ :
		{
			void* a = evaluate_sql_expr(expr->left, ec_p, error_code);
			if(*error_code)
				return NULL;
			if(a == NULL || a == ec_p->unknown_bool)
				return ec_p->unknown_bool;

			switch(expr->cmp_rhs_quantfier)
			{
				default :
				case SQL_CMP_NONE :
				{
					void* b = evaluate_sql_expr(expr->right, ec_p, error_code);
					if(*error_code)
					{
						delete_data_internal(a, ec_p);
						return NULL;
					}
					if(b == NULL || b == ec_p->unknown_bool)
					{
						delete_data_internal(a, ec_p);
						return ec_p->unknown_bool;
					}

					int a_minus_b = ec_p->compare(a, b, ec_p, error_code);
					delete_data_internal(a, ec_p);
					delete_data_internal(b, ec_p);
					if(*error_code)
						return NULL;

					if(a_minus_b != 0)
						return ec_p->true_bool;
					return ec_p->false_bool;
				}
				case SQL_CMP_ANY :
				{
					void* sub_query = ec_p->get_sub_query(expr->right_sub_query, ec_p, error_code);
					if(*error_code)
					{
						delete_data_internal(a, ec_p);
						return NULL;
					}

					void* res = ec_p->false_bool;
					int end_of_results = 0;
					while(res != ec_p->true_bool)
					{
						void* b = ec_p->next_data_from_sub_query(sub_query, &end_of_results, ec_p, error_code);
						if(*error_code)
						{
							ec_p->delete_sub_query(sub_query, ec_p);
							delete_data_internal(a, ec_p);
							return NULL;
						}
						if(end_of_results)
							break;

						void* curr_res = ec_p->unknown_bool;
						if(b != NULL && b != ec_p->unknown_bool)
						{
							int a_minus_b = ec_p->compare(a, b, ec_p, error_code);
							delete_data_internal(b, ec_p);
							if(*error_code)
							{
								ec_p->delete_sub_query(sub_query, ec_p);
								delete_data_internal(a, ec_p);
								return NULL;
							}
							curr_res = (a_minus_b != 0) ? ec_p->true_bool : ec_p->false_bool;
						}

						if(res == ec_p->true_bool || curr_res == ec_p->true_bool)
						{
							res = ec_p->true_bool;
							continue;
						}

						if(res == ec_p->unknown_bool || curr_res == ec_p->unknown_bool)
						{
							res = ec_p->unknown_bool;
							continue;
						}
					}

					ec_p->delete_sub_query(sub_query, ec_p);
					delete_data_internal(a, ec_p);

					return res;
				}
				case SQL_CMP_ALL :
				{
					void* sub_query = ec_p->get_sub_query(expr->right_sub_query, ec_p, error_code);
					if(*error_code)
					{
						delete_data_internal(a, ec_p);
						return NULL;
					}

					void* res = ec_p->true_bool;
					int end_of_results = 0;
					while(res != ec_p->false_bool)
					{
						void* b = ec_p->next_data_from_sub_query(sub_query, &end_of_results, ec_p, error_code);
						if(*error_code)
						{
							ec_p->delete_sub_query(sub_query, ec_p);
							delete_data_internal(a, ec_p);
							return NULL;
						}
						if(end_of_results)
							break;

						void* curr_res = ec_p->unknown_bool;
						if(b != NULL && b != ec_p->unknown_bool)
						{
							int a_minus_b = ec_p->compare(a, b, ec_p, error_code);
							delete_data_internal(b, ec_p);
							if(*error_code)
							{
								ec_p->delete_sub_query(sub_query, ec_p);
								delete_data_internal(a, ec_p);
								return NULL;
							}
							curr_res = (a_minus_b != 0) ? ec_p->true_bool : ec_p->false_bool;
						}

						if(res == ec_p->false_bool || curr_res == ec_p->false_bool)
						{
							res = ec_p->false_bool;
							continue;
						}

						if(res == ec_p->unknown_bool || curr_res == ec_p->unknown_bool)
						{
							res = ec_p->unknown_bool;
							continue;
						}
					}

					ec_p->delete_sub_query(sub_query, ec_p);
					delete_data_internal(a, ec_p);

					return res;
				}
			}
		}
		case SQL_BITAND :
		{
			void* a = evaluate_sql_expr(expr->left, ec_p, error_code);
			if(*error_code)
				return NULL;
			if(a == NULL || a == ec_p->unknown_bool)
				return ec_p->unknown_bool;

			void* b = evaluate_sql_expr(expr->right, ec_p, error_code);
			if(*error_code)
			{
				delete_data_internal(a, ec_p);
				return NULL;
			}
			if(b == NULL || b == ec_p->unknown_bool)
			{
				delete_data_internal(a, ec_p);
				return ec_p->unknown_bool;
			}

			void* res = ec_p->bit_and(a, b, ec_p, error_code);
			delete_data_internal(a, ec_p);
			delete_data_internal(b, ec_p);
			if(*error_code)
				return NULL;

			return res;
		}
		case SQL_BITOR :
		{
			void* a = evaluate_sql_expr(expr->left, ec_p, error_code);
			if(*error_code)
				return NULL;
			if(a == NULL || a == ec_p->unknown_bool)
				return ec_p->unknown_bool;

			void* b = evaluate_sql_expr(expr->right, ec_p, error_code);
			if(*error_code)
			{
				delete_data_internal(a, ec_p);
				return NULL;
			}
			if(b == NULL || b == ec_p->unknown_bool)
			{
				delete_data_internal(a, ec_p);
				return ec_p->unknown_bool;
			}

			void* res = ec_p->bit_or(a, b, ec_p, error_code);
			delete_data_internal(a, ec_p);
			delete_data_internal(b, ec_p);
			if(*error_code)
				return NULL;

			return res;
		}
		case SQL_BITXOR :
		{
			void* a = evaluate_sql_expr(expr->left, ec_p, error_code);
			if(*error_code)
				return NULL;
			if(a == NULL || a == ec_p->unknown_bool)
				return ec_p->unknown_bool;

			void* b = evaluate_sql_expr(expr->right, ec_p, error_code);
			if(*error_code)
			{
				delete_data_internal(a, ec_p);
				return NULL;
			}
			if(b == NULL || b == ec_p->unknown_bool)
			{
				delete_data_internal(a, ec_p);
				return ec_p->unknown_bool;
			}

			void* res = ec_p->bit_xor(a, b, ec_p, error_code);
			delete_data_internal(a, ec_p);
			delete_data_internal(b, ec_p);
			if(*error_code)
				return NULL;

			return res;
		}
		case SQL_LOGAND :
		{
			void* a = evaluate_sql_expr(expr->left, ec_p, error_code);
			if(*error_code)
				return NULL;

			void* log_a = ec_p->unknown_bool;
			if(a != NULL && a != ec_p->unknown_bool)
				log_a = ec_p->get_bool(a, ec_p, error_code);
			delete_data_internal(a, ec_p);
			if(*error_code)
				return NULL;

			if(log_a == ec_p->false_bool)
				return ec_p->false_bool;

			void* b = evaluate_sql_expr(expr->right, ec_p, error_code);
			if(*error_code)
				return NULL;

			void* log_b = ec_p->unknown_bool;
			if(b != NULL && b != ec_p->unknown_bool)
				log_b = ec_p->get_bool(b, ec_p, error_code);
			delete_data_internal(b, ec_p);
			if(*error_code)
				return NULL;

			if(log_a == ec_p->false_bool || log_b == ec_p->false_bool)
				return ec_p->false_bool;

			if(log_a == ec_p->unknown_bool || log_b == ec_p->unknown_bool)
				return ec_p->unknown_bool;

			// default return true when neither is false or unknown
			// this is the result when everything is true
			return ec_p->true_bool;
		}
		case SQL_LOGOR :
		{
			void* a = evaluate_sql_expr(expr->left, ec_p, error_code);
			if(*error_code)
				return NULL;

			void* log_a = ec_p->unknown_bool;
			if(a != NULL && a != ec_p->unknown_bool)
				log_a = ec_p->get_bool(a, ec_p, error_code);
			delete_data_internal(a, ec_p);
			if(*error_code)
				return NULL;

			if(log_a == ec_p->true_bool)
				return ec_p->true_bool;

			void* b = evaluate_sql_expr(expr->right, ec_p, error_code);
			if(*error_code)
				return NULL;

			void* log_b = ec_p->unknown_bool;
			if(b != NULL && b != ec_p->unknown_bool)
				log_b = ec_p->get_bool(b, ec_p, error_code);
			delete_data_internal(b, ec_p);
			if(*error_code)
				return NULL;

			if(log_a == ec_p->true_bool || log_b == ec_p->true_bool)
				return ec_p->true_bool;

			if(log_a == ec_p->unknown_bool || log_b == ec_p->unknown_bool)
				return ec_p->unknown_bool;

			// default return false when neither is true or unknown
			// this is the result when everything is false
			return ec_p->false_bool;
		}
		case SQL_LOGXOR :
		{
			void* a = evaluate_sql_expr(expr->left, ec_p, error_code);
			if(*error_code)
				return NULL;

			void* log_a = ec_p->unknown_bool;
			if(a != NULL && a != ec_p->unknown_bool)
				log_a = ec_p->get_bool(a, ec_p, error_code);
			delete_data_internal(a, ec_p);
			if(*error_code)
				return NULL;

			if(log_a == ec_p->unknown_bool)
				return ec_p->unknown_bool;

			void* b = evaluate_sql_expr(expr->right, ec_p, error_code);
			if(*error_code)
				return NULL;

			void* log_b = ec_p->unknown_bool;
			if(b != NULL && b != ec_p->unknown_bool)
				log_b = ec_p->get_bool(b, ec_p, error_code);
			delete_data_internal(b, ec_p);
			if(*error_code)
				return NULL;

			if(log_a == ec_p->unknown_bool || log_b == ec_p->unknown_bool)
				return ec_p->unknown_bool;

			if(log_a == ec_p->false_bool)
				return log_b;
			else if(log_b == ec_p->false_bool)
				return log_a;
			else
				return ec_p->false_bool;
		}
		case SQL_LSHIFT :
		{
			void* a = evaluate_sql_expr(expr->left, ec_p, error_code);
			if(*error_code)
				return NULL;
			if(a == NULL || a == ec_p->unknown_bool)
				return ec_p->unknown_bool;

			void* b = evaluate_sql_expr(expr->right, ec_p, error_code);
			if(*error_code)
			{
				delete_data_internal(a, ec_p);
				return NULL;
			}
			if(b == NULL || b == ec_p->unknown_bool)
			{
				delete_data_internal(a, ec_p);
				return ec_p->unknown_bool;
			}

			void* res = ec_p->left_shift(a, b, ec_p, error_code);
			delete_data_internal(a, ec_p);
			delete_data_internal(b, ec_p);
			if(*error_code)
				return NULL;

			return res;
		}
		case SQL_RSHIFT :
		{
			void* a = evaluate_sql_expr(expr->left, ec_p, error_code);
			if(*error_code)
				return NULL;
			if(a == NULL || a == ec_p->unknown_bool)
				return ec_p->unknown_bool;

			void* b = evaluate_sql_expr(expr->right, ec_p, error_code);
			if(*error_code)
			{
				delete_data_internal(a, ec_p);
				return NULL;
			}
			if(b == NULL || b == ec_p->unknown_bool)
			{
				delete_data_internal(a, ec_p);
				return ec_p->unknown_bool;
			}

			void* res = ec_p->right_shift(a, b, ec_p, error_code);
			delete_data_internal(a, ec_p);
			delete_data_internal(b, ec_p);
			if(*error_code)
				return NULL;

			return res;
		}
		case SQL_CONCAT :
		{
			snprintf_dstring(str_p, "(");
			snprint_sql_expr(str_p, expr->left);
			snprintf_dstring(str_p, ")||(");
			snprint_sql_expr(str_p, expr->right);
			snprintf_dstring(str_p, ")");
			break;
		}
		case SQL_LIKE :
		{
			snprintf_dstring(str_p, "(");
			snprint_sql_expr(str_p, expr->left);
			snprintf_dstring(str_p, ")LIKE(");
			snprint_sql_expr(str_p, expr->right);
			snprintf_dstring(str_p, ")");
			break;
		}
		case SQL_IS :
		{
			void* a = evaluate_sql_expr(expr->left, ec_p, error_code);
			if(*error_code)
				return NULL;

			void* b = evaluate_sql_expr(expr->right, ec_p, error_code);
			if(*error_code)
			{
				delete_data_internal(b, ec_p);
				return NULL;
			}

			if(b == a)
			{
				delete_data_internal(a, ec_p); // b too gets deleted, it points to a
				return ec_p->true_bool;
			}

			if(a == NULL || b == NULL || a == ec_p->unknown_bool || b == ec_p->unknown_bool)
			{
				delete_data_internal(a, ec_p);
				delete_data_internal(b, ec_p);
				return ec_p->false_bool;
			}

			void* log_a = ec_p->get_bool(a, ec_p, error_code);
			delete_data_internal(a, ec_p);
			if(*error_code)
			{
				delete_data_internal(b, ec_p);
				return NULL;
			}

			if(b == log_a)
			{
				delete_data_internal(b, ec_p);
				return ec_p->true_bool;
			}
			else
			{
				delete_data_internal(b, ec_p);
				return ec_p->false_bool;
			}
		}

		case SQL_BTWN :
		{
			void* input = evaluate_sql_expr(expr->btwn_input, ec_p, error_code);
			if(*error_code)
				return NULL;
			if(input == NULL || input == ec_p->unknown_bool)
				return ec_p->unknown_bool;

			void* low = evaluate_sql_expr(expr->bounds[0], ec_p, error_code);
			if(*error_code)
			{
				delete_data_internal(input, ec_p);
				return NULL;
			}

			void* high = evaluate_sql_expr(expr->bounds[1], ec_p, error_code);
			if(*error_code)
			{
				delete_data_internal(input, ec_p);
				delete_data_internal(low, ec_p);
				return NULL;
			}

			void* lower_bound_satisfied = ec_p->unknown_bool;
			if(low != NULL && low != ec_p->unknown_bool)
			{
				int input_minus_low = ec_p->compare(input, low, ec_p, error_code);
				if(*error_code)
				{
					delete_data_internal(input, ec_p);
					delete_data_internal(low, ec_p);
					delete_data_internal(high, ec_p);
					return NULL;
				}
				lower_bound_satisfied = (input_minus_low >= 0) ? ec_p->true_bool : ec_p->false_bool;
			}

			void* upper_bound_satisfied = ec_p->unknown_bool;
			if(high != NULL && high != ec_p->unknown_bool)
			{
				int input_minus_high = ec_p->compare(input, high, ec_p, error_code);
				if(*error_code)
				{
					delete_data_internal(input, ec_p);
					delete_data_internal(low, ec_p);
					delete_data_internal(high, ec_p);
					return NULL;
				}
				upper_bound_satisfied = (input_minus_high <= 0) ? ec_p->true_bool : ec_p->false_bool;
			}

			delete_data_internal(input, ec_p);
			delete_data_internal(low, ec_p);
			delete_data_internal(high, ec_p);

			if(lower_bound_satisfied == ec_p->false_bool || upper_bound_satisfied == ec_p->false_bool)
				return ec_p->false_bool;
			if(lower_bound_satisfied == ec_p->unknown_bool || upper_bound_satisfied == ec_p->unknown_bool)
				return ec_p->unknown_bool;
			return ec_p->true_bool;
		}

		case SQL_ADD_FLAT :
		{
			void* res = ec_p->zero_number;

			for(cy_uint i = 0; i < get_element_count_arraylist(&(expr->expr_list)); i++)
			{
				void* a = evaluate_sql_expr(get_from_front_of_arraylist(&(expr->expr_list), i), ec_p, error_code);
				if(*error_code)
				{
					delete_data_internal(res, ec_p);
					return NULL;
				}
				if(a == NULL || a == ec_p->unknown_bool)
				{
					delete_data_internal(res, ec_p);
					return ec_p->unknown_bool;
				}

				void* temp = ec_p->add(res, a, ec_p, error_code);
				delete_data_internal(a, ec_p);
				delete_data_internal(res, ec_p);
				if(*error_code)
					return NULL;

				res = temp;
			}

			return res;
		}
		case SQL_MUL_FLAT :
		{
			void* res = ec_p->one_number;

			for(cy_uint i = 0; i < get_element_count_arraylist(&(expr->expr_list)); i++)
			{
				void* a = evaluate_sql_expr(get_from_front_of_arraylist(&(expr->expr_list), i), ec_p, error_code);
				if(*error_code)
				{
					delete_data_internal(res, ec_p);
					return NULL;
				}
				if(a == NULL || a == ec_p->unknown_bool)
				{
					delete_data_internal(res, ec_p);
					return ec_p->unknown_bool;
				}

				void* temp = ec_p->mul(res, a, ec_p, error_code);
				delete_data_internal(a, ec_p);
				delete_data_internal(res, ec_p);
				if(*error_code)
					return NULL;

				res = temp;
			}

			return res;
		}
		case SQL_LOGAND_FLAT :
		{
			// default value
			void* res = ec_p->true_bool;

			for(cy_uint i = 0; i < get_element_count_arraylist(&(expr->expr_list)) && res != ec_p->false_bool; i++)
			{
				void* a = evaluate_sql_expr(get_from_front_of_arraylist(&(expr->expr_list), i), ec_p, error_code);
				if(*error_code)
					return NULL;

				void* log_a = ec_p->unknown_bool;
				if(a != NULL && a != ec_p->unknown_bool)
					log_a = ec_p->get_bool(a, ec_p, error_code);
				delete_data_internal(a, ec_p);
				if(*error_code)
					return NULL;

				if(res == ec_p->false_bool || log_a == ec_p->false_bool)
				{
					res = ec_p->false_bool;
					continue;
				}

				if(res == ec_p->unknown_bool || log_a == ec_p->unknown_bool)
				{
					res = ec_p->unknown_bool;
					continue;
				}
			}

			return res;
		}
		case SQL_LOGOR_FLAT :
		{
			// default value
			void* res = ec_p->false_bool;

			for(cy_uint i = 0; i < get_element_count_arraylist(&(expr->expr_list)) && res != ec_p->true_bool; i++)
			{
				void* a = evaluate_sql_expr(get_from_front_of_arraylist(&(expr->expr_list), i), ec_p, error_code);
				if(*error_code)
					return NULL;

				void* log_a = ec_p->unknown_bool;
				if(a != NULL && a != ec_p->unknown_bool)
					log_a = ec_p->get_bool(a, ec_p, error_code);
				delete_data_internal(a, ec_p);
				if(*error_code)
					return NULL;

				if(res == ec_p->true_bool || log_a == ec_p->true_bool)
				{
					res = ec_p->true_bool;
					continue;
				}

				if(res == ec_p->unknown_bool || log_a == ec_p->unknown_bool)
				{
					res = ec_p->unknown_bool;
					continue;
				}
			}

			return res;
		}
		case SQL_LOGXOR_FLAT :
		{
			// default value
			void* res = ec_p->false_bool;

			for(cy_uint i = 0; i < get_element_count_arraylist(&(expr->expr_list)) && res != ec_p->unknown_bool; i++)
			{
				void* a = evaluate_sql_expr(get_from_front_of_arraylist(&(expr->expr_list), i), ec_p, error_code);
				if(*error_code)
					return NULL;

				void* log_a = ec_p->unknown_bool;
				if(a != NULL && a != ec_p->unknown_bool)
					log_a = ec_p->get_bool(a, ec_p, error_code);
				delete_data_internal(a, ec_p);
				if(*error_code)
					return NULL;

				if(res == ec_p->unknown_bool || log_a == ec_p->unknown_bool)
				{
					res = ec_p->unknown_bool;
					continue;
				}

				if(log_a == ec_p->true_bool)
				{
					res = (res == ec_p->false_bool) ? ec_p->true_bool : ec_p->false_bool;
					continue;
				}
			}

			return res;
		}
		case SQL_CONCAT_FLAT :
		{
			for(cy_uint i = 0; i < get_element_count_arraylist(&(expr->expr_list)); i++)
			{
				if(i != 0)
					snprintf_dstring(str_p, "||");
				snprintf_dstring(str_p, "(");
				snprint_sql_expr(str_p, get_from_front_of_arraylist(&(expr->expr_list), i));
				snprintf_dstring(str_p, ")");
			}
			break;
		}
		case SQL_IN :
		{
			snprintf_dstring(str_p, "(");
			snprint_sql_expr(str_p, expr->in_input);
			snprintf_dstring(str_p, ")IN(");
			if(expr->in_sub_query)
				snprint_dql(str_p, expr->in_sub_query);
			else
			{
				for(cy_uint i = 0; i < get_element_count_arraylist(&(expr->in_expr_list)); i++)
				{
					if(i != 0)
						snprintf_dstring(str_p, ",");
					snprint_sql_expr(str_p, get_from_front_of_arraylist(&(expr->in_expr_list), i));
				}
			}
			snprintf_dstring(str_p, ")");
			break;
		}

		case SQL_STR :
		{
			return ec_p->create_string(&(expr->value), ec_p, error_code);
		}
		case SQL_NUM :
		{
			return ec_p->create_number(&(expr->value), ec_p, error_code);
		}
		case SQL_VAR :
		{
			return ec_p->get_variable(&(expr->value), ec_p, error_code);
		}

		case SQL_TRUE :
		{
			return ec_p->true_bool;
		}
		case SQL_FALSE :
		{
			return ec_p->false_bool;
		}
		case SQL_UNKNOWN :
		{
			return ec_p->unknown_bool;
		}
		case SQL_NULL :
		{
			return NULL;
		}

		case SQL_FUNCTION_CALL :
		{
			concatenate_dstring(str_p, &(expr->func_name));
			snprintf_dstring(str_p, "(");
			if(expr->aggregate_mode == SQL_RESULT_SET_DISTINCT)
				snprintf_dstring(str_p, "DISTINCT ");
			for(cy_uint i = 0; i < get_element_count_arraylist(&(expr->param_expr_list)); i++)
			{
				if(i != 0)
					snprintf_dstring(str_p, ",");
				snprint_sql_expr(str_p, get_from_front_of_arraylist(&(expr->param_expr_list), i));
			}
			snprintf_dstring(str_p, ")");
			break;
		}

		case SQL_CAST :
		{
			void* a = evaluate_sql_expr(expr->cast_expr, ec_p, error_code);
			if(*error_code)
				return NULL;
			if(a == NULL || a == ec_p->unknown_bool)
				return a;

			void* res = ec_p->cast(a, expr->cast_type, ec_p, error_code);
			delete_data_internal(a, ec_p);
			if(*error_code)
				return NULL;

			return res;
		}

		case SQL_SUB_QUERY :
		{
			void* sub_query = ec_p->get_sub_query(expr->sub_query, ec_p, error_code);
			if(*error_code)
				return NULL;

			int end_of_results = 0;
			void* some_result = ec_p->next_data_from_sub_query(sub_query, &end_of_results, ec_p, error_code);
			ec_p->delete_sub_query(sub_query, ec_p);
			if(*error_code)
				return NULL;

			if(end_of_results)
				return NULL;

			return some_result;
		}

		case SQL_EXISTS :
		{
			void* sub_query = ec_p->get_sub_query(expr->sub_query, ec_p, error_code);
			if(*error_code)
				return NULL;

			int end_of_results = 0;
			void* some_result = ec_p->next_data_from_sub_query(sub_query, &end_of_results, ec_p, error_code);
			ec_p->delete_sub_query(sub_query, ec_p);
			if(*error_code)
				return NULL;
			delete_data_internal(some_result, ec_p);

			return (end_of_results) ? ec_p->false_bool : ec_p->true_bool;
		}

		case SQL_CASE :
		{
			if(expr->case_expr)
			{
				snprintf_dstring(str_p, "CASE(");
				snprint_sql_expr(str_p, expr->case_expr);
				snprintf_dstring(str_p, ") ");
			}
			else
				snprintf_dstring(str_p, "CASE ");
			for(cy_uint i = 0; i < get_element_count_arraylist(&(expr->when_exprs)); i++)
			{
				{
					const sql_expression* when = get_from_front_of_arraylist(&(expr->when_exprs), i);
					snprintf_dstring(str_p, "WHEN(");
					snprint_sql_expr(str_p, when);
					snprintf_dstring(str_p, ") ");
				}

				{
					const sql_expression* then = get_from_front_of_arraylist(&(expr->then_exprs), i);
					snprintf_dstring(str_p, "THEN(");
					snprint_sql_expr(str_p, then);
					snprintf_dstring(str_p, ") ");
				}
			}
			if(expr->else_expr)
			{
				snprintf_dstring(str_p, "ELSE(");
				snprint_sql_expr(str_p, expr->else_expr);
				snprintf_dstring(str_p, ") ");
			}
			snprintf_dstring(str_p, "END");
			break;
		}
	}

	deinit_dstring(str_p);

	return NULL;
}