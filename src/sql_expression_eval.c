#include<sqltoast/sql_expression_eval.h>

#include<stdlib.h> // for alloca

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

static void* get_bool_internal(void* a, const sql_expr_eval_context* ec_p, int* error_code)
{
	if(a == NULL)
		return ec_p->unknown_bool;
	if(a == ec_p->true_bool || a == ec_p->false_bool || a == ec_p->unknown_bool)
		return a;
	return ec_p->get_bool(a, ec_p, error_code);
}

void* evaluate_sql_expr(const sql_expression* expr, const sql_expr_eval_context* ec_p, int* error_code)
{
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

			void* res = get_bool_internal(a, ec_p, error_code);
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
				log_a = get_bool_internal(a, ec_p, error_code);
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
				log_b = get_bool_internal(b, ec_p, error_code);
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
				log_a = get_bool_internal(a, ec_p, error_code);
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
				log_b = get_bool_internal(b, ec_p, error_code);
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
				log_a = get_bool_internal(a, ec_p, error_code);
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
				log_b = get_bool_internal(b, ec_p, error_code);
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

			ec_p->concat(&a, b, ec_p, error_code);
			delete_data_internal(b, ec_p);
			if(*error_code)
			{
				delete_data_internal(a, ec_p);
				return NULL;
			}

			return a;
		}
		case SQL_LIKE :
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

			void* res = ec_p->like(a, b, ec_p, error_code);
			delete_data_internal(a, ec_p);
			delete_data_internal(b, ec_p);
			if(*error_code)
				return NULL;

			return res;
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

			void* log_a = get_bool_internal(a, ec_p, error_code);
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

				if(i == 0)
				{
					delete_data_internal(res, ec_p);
					res = a;
					continue;
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

				if(i == 0)
				{
					delete_data_internal(res, ec_p);
					res = a;
					continue;
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
					log_a = get_bool_internal(a, ec_p, error_code);
				delete_data_internal(a, ec_p);
				if(*error_code)
					return NULL;

				if(i == 0)
				{
					delete_data_internal(res, ec_p);
					res = log_a;
					continue;
				}

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
					log_a = get_bool_internal(a, ec_p, error_code);
				delete_data_internal(a, ec_p);
				if(*error_code)
					return NULL;

				if(i == 0)
				{
					delete_data_internal(res, ec_p);
					res = log_a;
					continue;
				}

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
					log_a = get_bool_internal(a, ec_p, error_code);
				delete_data_internal(a, ec_p);
				if(*error_code)
					return NULL;

				if(i == 0)
				{
					delete_data_internal(res, ec_p);
					res = log_a;
					continue;
				}

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
			void* res = ec_p->create_string(&get_dstring_pointing_to_literal_cstring(""), ec_p, error_code);
			if(*error_code)
				return NULL;

			for(cy_uint i = 0; i < get_element_count_arraylist(&(expr->expr_list)); i++)
			{
				void* s = evaluate_sql_expr(get_from_front_of_arraylist(&(expr->expr_list), i), ec_p, error_code);
				if(*error_code)
				{
					delete_data_internal(res, ec_p);
					return NULL;
				}

				if(s == NULL || s == ec_p->unknown_bool)
				{
					delete_data_internal(res, ec_p);
					return ec_p->unknown_bool;
				}

				if(i == 0)
				{
					delete_data_internal(res, ec_p);
					res = s;
					continue;
				}

				ec_p->concat(&res, s, ec_p, error_code);
				delete_data_internal(s, ec_p);
				if(*error_code)
				{
					delete_data_internal(res, ec_p);
					return NULL;
				}
			}

			return res;
		}
		case SQL_IN :
		{
			void* a = evaluate_sql_expr(expr->in_input, ec_p, error_code);
			if(*error_code)
				return NULL;
			if(a == NULL || a == ec_p->unknown_bool)
				return ec_p->unknown_bool;

			if(expr->in_sub_query)
			{
				void* sub_query = ec_p->get_sub_query(expr->in_sub_query, ec_p, error_code);
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
			else
			{
				void* res = ec_p->false_bool;
				for(cy_uint i = 0; i < get_element_count_arraylist(&(expr->in_expr_list)) && res != ec_p->true_bool; i++)
				{
					void* b = evaluate_sql_expr(get_from_front_of_arraylist(&(expr->in_expr_list), i), ec_p, error_code);
					if(*error_code)
					{
						delete_data_internal(a, ec_p);
						return NULL;
					}

					void* curr_res = ec_p->unknown_bool;
					if(b != NULL && b != ec_p->unknown_bool)
					{
						int a_minus_b = ec_p->compare(a, b, ec_p, error_code);
						delete_data_internal(b, ec_p);
						if(*error_code)
						{
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

				delete_data_internal(a, ec_p);
				return res;
			}
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
			void** param_values = alloca(sizeof(void*) * get_element_count_arraylist(&(expr->param_expr_list)));

			for(cy_uint i = 0; i < get_element_count_arraylist(&(expr->param_expr_list)); i++)
			{
				param_values[i] = evaluate_sql_expr(get_from_front_of_arraylist(&(expr->param_expr_list), i), ec_p, error_code);
				if(*error_code)
				{
					for(cy_uint j = 0; j < i; j++)
						delete_data_internal(param_values[j], ec_p);
					return NULL;
				}
				if(param_values[i] == NULL || param_values[i] == ec_p->unknown_bool)
					param_values[i] = NULL;
			}

			void* res = ec_p->call_function(&(expr->func_name), param_values, get_element_count_arraylist(&(expr->param_expr_list)), ec_p, error_code);
			for(cy_uint i = 0; i < get_element_count_arraylist(&(expr->param_expr_list)); i++)
				delete_data_internal(param_values[i], ec_p);
			if(*error_code)
				return NULL;

			return res;
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
			int has_a = 0;
			void* a = NULL;
			if(expr->case_expr != NULL)
			{
				a = evaluate_sql_expr(expr->case_expr, ec_p, error_code);
				if(*error_code)
					return NULL;
				has_a = 1;
			}

			if((has_a && a != NULL && a != ec_p->unknown_bool) || (!has_a))
			{
				for(cy_uint i = 0; i < get_element_count_arraylist(&(expr->when_exprs)); i++)
				{
					void* when = evaluate_sql_expr(get_from_front_of_arraylist(&(expr->when_exprs), i), ec_p, error_code);
					if(*error_code)
					{
						delete_data_internal(a, ec_p);
						return NULL;
					}
					if(when == NULL || when == ec_p->unknown_bool)
						continue;

					int produce_output = 0;

					if(!has_a)
					{
						void* log_when = get_bool_internal(when, ec_p, error_code);
						delete_data_internal(when, ec_p);
						if(*error_code)
						{
							delete_data_internal(a, ec_p);
							return NULL;
						}
						produce_output = (log_when == ec_p->true_bool);
					}
					else
					{
						int a_minus_when = ec_p->compare(a, when, ec_p, error_code);
						delete_data_internal(when, ec_p);
						if(*error_code)
						{
							delete_data_internal(a, ec_p);
							return NULL;
						}
						produce_output = (a_minus_when == 0);
					}

					if(produce_output)
					{
						delete_data_internal(a, ec_p);
						void* result = evaluate_sql_expr(get_from_front_of_arraylist(&(expr->then_exprs), i), ec_p, error_code);
						if(*error_code)
							return NULL;
						return result;
					}
				}
			}

			if(expr->else_expr)
			{
				delete_data_internal(a, ec_p);
				void* result = evaluate_sql_expr(expr->else_expr, ec_p, error_code);
				if(*error_code)
					return NULL;
				return result;
			}

			delete_data_internal(a, ec_p);
			return NULL;
		}
	}

	return NULL;
}

static void delete_type_internal(void* t, const sql_expr_eval_context* ec_p)
{
	if(t == NULL)
		return;
	if(t == ec_p->bool_type)
		return;
	ec_p->delete_type(t, ec_p);
}

void* infer_type_sql_expr(const sql_expression* expr, const sql_expr_eval_context* ec_p, int* error_code)
{
	switch(expr->type)
	{
		case SQL_MUL_INV :
		{
			void* c = ec_p->get_type_for_data(ec_p->one_number, ec_p, error_code);;
			if(*error_code)
				return NULL;

			void* t = infer_type_sql_expr(expr->unary_of, ec_p, error_code);
			if(*error_code)
			{
				delete_type_internal(c, ec_p);
				return NULL;
			}

			void* res = ec_p->get_return_type_for_op_exec_callback(ec_p->div, c, t, ec_p, error_code);
			delete_type_internal(c, ec_p);
			delete_type_internal(t, ec_p);
			if(*error_code)
				return NULL;

			return res;
		}
		case SQL_NEG :
		{
			void* c = ec_p->get_type_for_data(ec_p->zero_number, ec_p, error_code);;
			if(*error_code)
				return NULL;

			void* t = infer_type_sql_expr(expr->unary_of, ec_p, error_code);
			if(*error_code)
			{
				delete_type_internal(c, ec_p);
				return NULL;
			}

			void* res = ec_p->get_return_type_for_op_exec_callback(ec_p->sub, c, t, ec_p, error_code);
			delete_type_internal(c, ec_p);
			delete_type_internal(t, ec_p);
			if(*error_code)
				return NULL;

			return res;
		}
		case SQL_BITNOT :
		{
			void* t = infer_type_sql_expr(expr->unary_of, ec_p, error_code);
			if(*error_code)
				return NULL;

			void* res = ec_p->get_return_type_for_op_exec_callback(ec_p->bit_not, t, t, ec_p, error_code); // let the user decide which param type to use of the two
			delete_type_internal(t, ec_p);
			if(*error_code)
				return NULL;

			return res;
		}

		case SQL_LOGNOT :
			return ec_p->bool_type;

		case SQL_ADD :
		case SQL_SUB :
		case SQL_MUL :
		case SQL_DIV :
		case SQL_MOD :
		case SQL_BITAND :
		case SQL_BITOR :
		case SQL_BITXOR :
		case SQL_LSHIFT :
		case SQL_RSHIFT :
		case SQL_CONCAT :
		case SQL_LIKE :
		{
			void* op_exec_func = NULL;
			switch(expr->type)
			{
				case SQL_ADD :
					op_exec_func = ec_p->add;
					break;
				case SQL_SUB :
					op_exec_func = ec_p->sub;
					break;
				case SQL_MUL :
					op_exec_func = ec_p->mul;
					break;
				case SQL_DIV :
					op_exec_func = ec_p->div;
					break;
				case SQL_MOD :
					op_exec_func = ec_p->mod;
					break;
				case SQL_BITAND :
					op_exec_func = ec_p->bit_and;
					break;
				case SQL_BITOR :
					op_exec_func = ec_p->bit_or;
					break;
				case SQL_BITXOR :
					op_exec_func = ec_p->bit_xor;
					break;
				case SQL_LSHIFT :
					op_exec_func = ec_p->left_shift;
					break;
				case SQL_RSHIFT :
					op_exec_func = ec_p->right_shift;
					break;
				case SQL_CONCAT :
					op_exec_func = ec_p->concat;
					break;
				case SQL_LIKE :
					op_exec_func = ec_p->like;
					break;
				default :
					break;
			}

			void* t1 = infer_type_sql_expr(expr->left, ec_p, error_code);
			if(*error_code)
				return NULL;

			void* t2 = infer_type_sql_expr(expr->right, ec_p, error_code);
			if(*error_code)
			{
				delete_type_internal(t1, ec_p);
				return NULL;
			}

			void* res = ec_p->get_return_type_for_op_exec_callback(op_exec_func, t1, t2, ec_p, error_code);
			delete_type_internal(t1, ec_p);
			delete_type_internal(t2, ec_p);
			if(*error_code)
				return NULL;

			return res;
		}

		case SQL_LOGAND :
		case SQL_LOGOR :
		case SQL_LOGXOR :
		case SQL_IS :
			return ec_p->bool_type;

		case SQL_GT :
		case SQL_GTE :
		case SQL_LT :
		case SQL_LTE :
		case SQL_EQ :
		case SQL_NEQ :
		{
			void* t1 = infer_type_sql_expr(expr->left, ec_p, error_code);
			if(*error_code)
				return NULL;

			void* t2 = NULL;
			if(expr->cmp_rhs_quantfier == SQL_CMP_NONE)
			{
				t2 = infer_type_sql_expr(expr->right, ec_p, error_code);
				if(*error_code)
				{
					delete_type_internal(t1, ec_p);
					return NULL;
				}
			}
			else
			{
				t2 = ec_p->get_type_for_sub_query(expr->right_sub_query, ec_p, error_code);
				if(*error_code)
				{
					delete_type_internal(t1, ec_p);
					return NULL;
				}
			}

			int cc = ec_p->can_compare_types(t1, t2, ec_p, error_code);
			delete_type_internal(t1, ec_p);
			delete_type_internal(t2, ec_p);
			if(*error_code)
				return NULL;

			if(cc)
				return ec_p->bool_type;

			(*error_code) = 1;
			return NULL;
		}

		case SQL_BTWN :
		{
			void* b0 = infer_type_sql_expr(expr->bounds[0], ec_p, error_code);
			if(*error_code)
				return NULL;

			void* b1 = infer_type_sql_expr(expr->bounds[1], ec_p, error_code);
			if(*error_code)
			{
				delete_type_internal(b0, ec_p);
				return NULL;
			}

			void* i = infer_type_sql_expr(expr->btwn_input, ec_p, error_code);
			if(*error_code)
			{
				delete_type_internal(b0, ec_p);
				delete_type_internal(b1, ec_p);
				return NULL;
			}

			int cc1 = ec_p->can_compare_types(b0, i, ec_p, error_code);
			if(*error_code)
			{
				delete_type_internal(b0, ec_p);
				delete_type_internal(b1, ec_p);
				delete_type_internal(i, ec_p);
				if(*error_code)
					return NULL;
			}
			int cc2 = ec_p->can_compare_types(b1, i, ec_p, error_code);
			delete_type_internal(b0, ec_p);
			delete_type_internal(b1, ec_p);
			delete_type_internal(i, ec_p);
			if(*error_code)
				return NULL;

			if(cc1 && cc2)
				return ec_p->bool_type;

			(*error_code) = 1;
			return NULL;
		}

		case SQL_ADD_FLAT :
		case SQL_MUL_FLAT :
		case SQL_CONCAT_FLAT :
		{
			void* res = NULL;
			void* op_exec_func = NULL;
			switch(expr->type)
			{
				case SQL_ADD_FLAT :
					res = ec_p->get_type_for_data(ec_p->zero_number, ec_p, error_code);
					if(*error_code)
						return NULL;
					op_exec_func = ec_p->add;
					break;
				case SQL_MUL_FLAT :
					res = ec_p->get_type_for_data(ec_p->one_number, ec_p, error_code);
					if(*error_code)
						return NULL;
					op_exec_func = ec_p->mul;
					break;
				case SQL_CONCAT_FLAT :
					void* res_data = ec_p->create_string(&get_dstring_pointing_to_literal_cstring(""), ec_p, error_code);
					if(*error_code)
						return NULL;
					res = ec_p->get_type_for_data(res_data, ec_p, error_code);
					delete_data_internal(res_data, ec_p);
					if(*error_code)
						return NULL;
					op_exec_func = ec_p->concat;
					break;
				default :
					break;
			}

			for(cy_uint i = 0; i < get_element_count_arraylist(&(expr->expr_list)); i++)
			{
				void* t = infer_type_sql_expr((const sql_expression*)get_from_front_of_arraylist(&(expr->expr_list), i), ec_p, error_code);
				if(*error_code)
				{
					delete_type_internal(res, ec_p);
					return NULL;
				}

				void* new_res = ec_p->get_return_type_for_op_exec_callback(op_exec_func, res, t, ec_p, error_code);
				delete_type_internal(res, ec_p);
				delete_type_internal(t, ec_p);
				if(*error_code)
					return NULL;

				res = new_res;
			}

			return res;
		}

		case SQL_LOGAND_FLAT :
		case SQL_LOGOR_FLAT :
		case SQL_LOGXOR_FLAT :
			return ec_p->bool_type;

		case SQL_IN :
		{
			void* t1 = infer_type_sql_expr(expr->in_input, ec_p, error_code);
			if(*error_code)
				return NULL;

			if(expr->in_sub_query)
			{
				void* t2 = ec_p->get_type_for_sub_query(expr->in_sub_query, ec_p, error_code);
				if(*error_code)
				{
					delete_type_internal(t1, ec_p);
					return NULL;
				}

				int cc = ec_p->can_compare_types(t1, t2, ec_p, error_code);
				delete_type_internal(t1, ec_p);
				delete_type_internal(t2, ec_p);
				if(*error_code)
					return NULL;

				if(cc)
					return ec_p->bool_type;

				(*error_code) = 1;
				return NULL;
			}
			else
			{
				for(cy_uint i = 0; i < get_element_count_arraylist(&(expr->in_expr_list)); i++)
				{
					void* t2 = infer_type_sql_expr((const sql_expression*)get_from_front_of_arraylist(&(expr->in_expr_list), i), ec_p, error_code);
					if(*error_code)
					{
						delete_type_internal(t1, ec_p);
						return NULL;
					}

					int cc = ec_p->can_compare_types(t1, t2, ec_p, error_code);
					delete_type_internal(t2, ec_p);
					if(*error_code)
					{
						delete_type_internal(t1, ec_p);
						return NULL;
					}

					if(cc)
						continue;

					if(!cc)
					{
						(*error_code) = 1;
						delete_type_internal(t1, ec_p);
						return NULL;
					}
				}

				delete_type_internal(t1, ec_p);
				return ec_p->bool_type;
			}
		}

		case SQL_NUM :
		{
			void* d = ec_p->create_number(&(expr->value), ec_p, error_code);
			if(*error_code)
				return NULL;

			void* t = ec_p->get_type_for_data(d, ec_p, error_code);
			delete_data_internal(d, ec_p);
			if(*error_code)
				return NULL;

			return t;
		}
		case SQL_STR :
		{
			void* d = ec_p->create_string(&(expr->value), ec_p, error_code);
			if(*error_code)
				return NULL;

			void* t = ec_p->get_type_for_data(d, ec_p, error_code);
			delete_data_internal(d, ec_p);
			if(*error_code)
				return NULL;

			return t;
		}

		case SQL_VAR :
			return ec_p->get_type_for_variable(&(expr->value), ec_p, error_code);

		case SQL_TRUE :
		case SQL_FALSE :
		case SQL_UNKNOWN :
			return ec_p->bool_type;

		case SQL_NULL :
			return NULL;

		case SQL_FUNCTION_CALL : // TODO
		{
			for(cy_uint i = 0; i < get_element_count_arraylist(&(expr->param_expr_list)); i++)
				if(has_sub_query_in_sql_exp((const sql_expression*)get_from_front_of_arraylist(&(expr->param_expr_list), i)))
					return 1;
			return 0;
		}

		case SQL_CAST :
		{
			void* t = ec_p->get_type_for_sql_type(expr->cast_type, ec_p, error_code);
			if(*error_code)
				return NULL;

			void* d = infer_type_sql_expr(expr->cast_expr, ec_p, error_code);
			if(*error_code)
			{
				delete_type_internal(t, ec_p);
				return NULL;
			}

			int cc = ec_p->can_cast_types(t, d, ec_p, error_code);
			delete_type_internal(d, ec_p);
			if(*error_code)
			{
				delete_type_internal(t, ec_p);
				return NULL;
			}

			if(cc)
				return t;

			(*error_code) = 1;
			delete_type_internal(t, ec_p);
			return NULL;
		}

		case SQL_SUB_QUERY :
			return ec_p->get_type_for_sub_query(expr->sub_query, ec_p, error_code);

		case SQL_EXISTS :
			return ec_p->bool_type;

		case SQL_CASE : // TODO
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

	return NULL;
}