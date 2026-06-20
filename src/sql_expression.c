#include<sqltoast/sql_expression.h>

#include<cutlery/cutlery_math.h>

#include<stdlib.h>
#include<stdio.h>

void initialize_expr_list(arraylist* expr_list)
{
	if(!initialize_arraylist(expr_list, 5))
	{
		printf("failed to initialize expression list\n");
		exit(-1);
	}
}

void insert_in_expr_list(arraylist* expr_list, sql_expression* expr)
{
	if(is_full_arraylist(expr_list) && !expand_arraylist(expr_list))
	{
		printf("failed to insert back in expression list\n");
		exit(-1);
	}
	push_back_to_arraylist(expr_list, expr);
}

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

sql_expression* new_compare_sql_expr(sql_expression_type type, sql_cmp_quantifier cmp_rhs_quantfier, sql_expression* left, void* right)
{
	sql_expression* expr = malloc(sizeof(sql_expression));
	expr->type = type;
	expr->cmp_rhs_quantfier = cmp_rhs_quantfier;
	expr->left = left;
	expr->right = right;
	return expr;
}

sql_expression* new_between_sql_expr(sql_expression* input, sql_expression* bounds0, sql_expression* bounds1)
{
	sql_expression* expr = malloc(sizeof(sql_expression));
	expr->type = SQL_BTWN;
	expr->btwn_input = input;
	expr->bounds[0] = bounds0;
	expr->bounds[1] = bounds1;
	return expr;
}

sql_expression* new_flat_sql_expr(sql_expression_type type, arraylist expr_list)
{
	sql_expression* expr = malloc(sizeof(sql_expression));
	expr->type = type;
	expr->expr_list = expr_list;
	return expr;
}

sql_expression* new_in_sql_expr(sql_expression* in_input, sql_dql* in_sub_query, arraylist in_expr_list)
{
	sql_expression* expr = malloc(sizeof(sql_expression));
	expr->type = SQL_IN;
	expr->in_input = in_input;
	expr->in_sub_query = in_sub_query;
	expr->in_expr_list = in_expr_list;
	return expr;
}

sql_expression* new_func_sql_expr(dstring func_name, set_op_mod aggregate_mode, arraylist param_expr_list)
{
	sql_expression* expr = malloc(sizeof(sql_expression));
	expr->type = SQL_FUNCTION_CALL;
	expr->aggregate_mode = aggregate_mode;
	expr->func_name = func_name;
	expr->param_expr_list = param_expr_list;
	return expr;
}

sql_expression* new_cast_sql_expr(sql_expression* cast_expr, sql_type* cast_type)
{
	sql_expression* expr = malloc(sizeof(sql_expression));
	expr->type = SQL_CAST;
	expr->cast_expr = cast_expr;
	expr->cast_type = cast_type;
	return expr;
}

sql_expression* new_sub_query_sql_expr(sql_expression_type type, sql_dql* sub_query)
{
	sql_expression* expr = malloc(sizeof(sql_expression));
	expr->type = type;
	expr->sub_query = sub_query;
	return expr;
}

sql_expression* new_valued_sql_expr(sql_expression_type type, dstring value)
{
	sql_expression* expr = malloc(sizeof(sql_expression));
	expr->type = type;
	expr->value = value;
	return expr;
}

sql_expression* new_const_non_valued_sql_expr(sql_expression_type type)
{
	sql_expression* expr = malloc(sizeof(sql_expression));
	expr->type = type;
	return expr;
}

sql_expression* new_case_sql_expr(sql_expression* case_expr, arraylist when_exprs, arraylist then_exprs, sql_expression* else_expr)
{
	sql_expression* expr = malloc(sizeof(sql_expression));
	expr->type = SQL_CASE;
	expr->case_expr = case_expr;
	expr->when_exprs = when_exprs;
	expr->then_exprs = then_exprs;
	expr->else_expr = else_expr;
	return expr;
}

sql_expression* flatten_similar_associative_operators_in_sql_expression(sql_expression* expr)
{
	switch(expr->type)
	{
		case SQL_ADD :
		case SQL_MUL :
		case SQL_LOGAND :
		case SQL_LOGOR :
		case SQL_LOGXOR :
		case SQL_CONCAT :
		{
			sql_expression* left = flatten_similar_associative_operators_in_sql_expression(expr->left);
			sql_expression* right = flatten_similar_associative_operators_in_sql_expression(expr->right);

			// the next type is always the flattenned operator
			sql_expression_type flat_type = expr->type + 1;

			arraylist expr_list;
			initialize_expr_list(&expr_list);

			// free up the, memory for the old binary version of this operator
			free(expr);

			// only similar flat expression types can be made flat
			if(left->type == flat_type)
			{
				for(cy_uint i = 0; i < get_element_count_arraylist(&(left->expr_list)); i++)
					insert_in_expr_list(&expr_list, (sql_expression*) get_from_front_of_arraylist(&(left->expr_list), i));

				// destroy just the left child
				deinitialize_arraylist(&(left->expr_list));
				free(left);
			}
			else // else insert left as is
				insert_in_expr_list(&expr_list, left);

			// only similar flat expression types can be made flat
			if(right->type == flat_type)
			{
				for(cy_uint i = 0; i < get_element_count_arraylist(&(right->expr_list)); i++)
					insert_in_expr_list(&expr_list, (sql_expression*) get_from_front_of_arraylist(&(right->expr_list), i));

				// destroy just the right child
				deinitialize_arraylist(&(right->expr_list));
				free(right);
			}
			else // else insert right as is
				insert_in_expr_list(&expr_list, right);

			return new_flat_sql_expr(flat_type, expr_list);
		}

		case SQL_SUB : // A - B to A + (-B)
		{
			sql_expression* left = flatten_similar_associative_operators_in_sql_expression(expr->left);
			sql_expression* right = flatten_similar_associative_operators_in_sql_expression(expr->right);

			// the next type is always the flattenned operator
			sql_expression_type flat_type = SQL_ADD_FLAT;

			arraylist expr_list;
			initialize_expr_list(&expr_list);

			// free up the, memory for the old binary version of this operator
			free(expr);

			// only similar flat expression types can be made flat
			if(left->type == flat_type)
			{
				for(cy_uint i = 0; i < get_element_count_arraylist(&(left->expr_list)); i++)
					insert_in_expr_list(&expr_list, (sql_expression*) get_from_front_of_arraylist(&(left->expr_list), i));

				// destroy just the left child
				deinitialize_arraylist(&(left->expr_list));
				free(left);
			}
			else // else insert left as is
				insert_in_expr_list(&expr_list, left);

			// only similar flat expression types can be made flat
			if(right->type == flat_type)
			{
				for(cy_uint i = 0; i < get_element_count_arraylist(&(right->expr_list)); i++)
				{
					sql_expression* t = (sql_expression*) get_from_front_of_arraylist(&(right->expr_list), i);
					sql_expression* neg_t = NULL;
					if(t->type == SQL_NEG) // remove (--) in sequence to just a flatenned +
					{
						neg_t = t->unary_of;
						free(t);
					}
					else
						neg_t = new_unary_sql_expr(SQL_NEG, t);
					// from here on t may not exist, and may have been deleted
					insert_in_expr_list(&expr_list, neg_t);
				}

				// destroy just the right child
				deinitialize_arraylist(&(right->expr_list));
				free(right);
			}
			else // else insert negative of right
			{
				sql_expression* neg_right = NULL;
				if(right->type == SQL_NEG)
				{
					neg_right = right->unary_of;
					free(right);
				}
				else
					neg_right = new_unary_sql_expr(SQL_NEG, right);
				insert_in_expr_list(&expr_list, neg_right);
			}

			return new_flat_sql_expr(flat_type, expr_list);
		}

#ifdef FLATTEN_SQL_DIV_INTO_SQL_MUL_INV
		case SQL_DIV : // A * B to A * (B^(-1))
		{
			sql_expression* left = flatten_similar_associative_operators_in_sql_expression(expr->left);
			sql_expression* right = flatten_similar_associative_operators_in_sql_expression(expr->right);

			// the next type is always the flattenned operator
			sql_expression_type flat_type = SQL_MUL_FLAT;

			arraylist expr_list;
			initialize_expr_list(&expr_list);

			// free up the, memory for the old binary version of this operator
			free(expr);

			// only similar flat expression types can be made flat
			if(left->type == flat_type)
			{
				for(cy_uint i = 0; i < get_element_count_arraylist(&(left->expr_list)); i++)
					insert_in_expr_list(&expr_list, (sql_expression*) get_from_front_of_arraylist(&(left->expr_list), i));

				// destroy just the left child
				deinitialize_arraylist(&(left->expr_list));
				free(left);
			}
			else // else insert left as is
				insert_in_expr_list(&expr_list, left);

			// only similar flat expression types can be made flat
			if(right->type == flat_type)
			{
				for(cy_uint i = 0; i < get_element_count_arraylist(&(right->expr_list)); i++)
				{
					sql_expression* t = (sql_expression*) get_from_front_of_arraylist(&(right->expr_list), i);
					sql_expression* inv_t = NULL;
					if(t->type == SQL_MUL_INV) // remove (--) in sequence to just a flatenned +
					{
						inv_t = t->unary_of;
						free(t);
					}
					else
						inv_t = new_unary_sql_expr(SQL_MUL_INV, t);
					// from here on t may not exist, and may have been deleted
					insert_in_expr_list(&expr_list, inv_t);
				}

				// destroy just the right child
				deinitialize_arraylist(&(right->expr_list));
				free(right);
			}
			else // else insert inverse of right
			{
				sql_expression* inv_right = NULL;
				if(right->type == SQL_MUL_INV)
				{
					inv_right = right->unary_of;
					free(right);
				}
				else
					inv_right = new_unary_sql_expr(SQL_MUL_INV, right);
				insert_in_expr_list(&expr_list, inv_right);
			}

			return new_flat_sql_expr(flat_type, expr_list);
		}
#endif

		case SQL_MUL_INV :
		case SQL_NEG :
		case SQL_BITNOT :
		case SQL_LOGNOT :
		{
			expr->unary_of = flatten_similar_associative_operators_in_sql_expression(expr->unary_of);
			if(expr->unary_of->type == expr->type && expr->unary_of->unary_of->type == expr->type)
			{
				sql_expression* flat_expr = expr->unary_of->unary_of;
				free(expr->unary_of);
				free(expr);
				return flat_expr;
			}
			else
				return expr;
		}

#ifndef FLATTEN_SQL_DIV_INTO_SQL_MUL_INV
		case SQL_DIV :
#endif
		case SQL_MOD :
		case SQL_BITAND :
		case SQL_BITOR :
		case SQL_BITXOR :
		case SQL_LSHIFT :
		case SQL_RSHIFT :
		case SQL_LIKE :
		case SQL_IS :
		{
			expr->left = flatten_similar_associative_operators_in_sql_expression(expr->left);
			expr->right = flatten_similar_associative_operators_in_sql_expression(expr->right);
			return expr;
		}

		case SQL_GT :
		case SQL_GTE :
		case SQL_LT :
		case SQL_LTE :
		case SQL_EQ :
		case SQL_NEQ :
		{
			expr->left = flatten_similar_associative_operators_in_sql_expression(expr->left);
			if(expr->cmp_rhs_quantfier == SQL_CMP_NONE)
				expr->right = flatten_similar_associative_operators_in_sql_expression(expr->right);
			else
				flatten_exprs_dql(expr->right_sub_query);
			return expr;
		}

		case SQL_BTWN :
		{
			expr->btwn_input = flatten_similar_associative_operators_in_sql_expression(expr->btwn_input);
			expr->bounds[0] = flatten_similar_associative_operators_in_sql_expression(expr->bounds[0]);
			expr->bounds[1] = flatten_similar_associative_operators_in_sql_expression(expr->bounds[1]);
			return expr;
		}

		// they are already flatenned
		case SQL_ADD_FLAT :
		case SQL_MUL_FLAT :
		case SQL_LOGAND_FLAT :
		case SQL_LOGOR_FLAT :
		case SQL_LOGXOR_FLAT :
		case SQL_CONCAT_FLAT :
		{
			return expr;
		}

		case SQL_IN :
		{
			if(expr->in_sub_query != NULL)
				;//flatten_similar_associative_operators_in_sql_dql_query(expr->in_sub_query);
			for(cy_uint i = 0; i < get_element_count_arraylist(&(expr->in_expr_list)); i++)
				set_from_front_in_arraylist(&(expr->in_expr_list), (sql_expression*) flatten_similar_associative_operators_in_sql_expression((sql_expression*)get_from_front_of_arraylist(&(expr->in_expr_list), i)), i);
			expr->in_input = flatten_similar_associative_operators_in_sql_expression(expr->in_input);
			return expr;
		}

		case SQL_NUM :
		case SQL_STR :
		case SQL_VAR :
		case SQL_TRUE :
		case SQL_FALSE :
		case SQL_UNKNOWN :
		case SQL_NULL :
		{
			return expr;
		}

		case SQL_FUNCTION_CALL :
		{
			for(cy_uint i = 0; i < get_element_count_arraylist(&(expr->param_expr_list)); i++)
				set_from_front_in_arraylist(&(expr->param_expr_list), (sql_expression*) flatten_similar_associative_operators_in_sql_expression((sql_expression*)get_from_front_of_arraylist(&(expr->param_expr_list), i)), i);
			return expr;
		}

		case SQL_CAST :
		{
			expr->cast_expr = flatten_similar_associative_operators_in_sql_expression(expr->cast_expr);
			return expr;
		}

		case SQL_SUB_QUERY :
		case SQL_EXISTS :
		{
			flatten_exprs_dql(expr->sub_query);
			return expr;
		}

		case SQL_CASE :
		{
			if(expr->case_expr)
				expr->case_expr = flatten_similar_associative_operators_in_sql_expression(expr->case_expr);
			for(cy_uint i = 0; i < get_element_count_arraylist(&(expr->when_exprs)); i++)
			{
				sql_expression* x = (sql_expression*) get_from_front_of_arraylist(&(expr->when_exprs), i);
				x = flatten_similar_associative_operators_in_sql_expression(x);
				set_from_front_in_arraylist(&(expr->when_exprs), x, i);
			}
			for(cy_uint i = 0; i < get_element_count_arraylist(&(expr->then_exprs)); i++)
			{
				sql_expression* x = (sql_expression*) get_from_front_of_arraylist(&(expr->then_exprs), i);
				x = flatten_similar_associative_operators_in_sql_expression(x);
				set_from_front_in_arraylist(&(expr->then_exprs), x, i);
			}
			if(expr->else_expr)
				expr->else_expr = flatten_similar_associative_operators_in_sql_expression(expr->else_expr);
			return expr;
		}
	}

	return NULL;
}

void snprint_sql_expr(dstring* str_p, const sql_expression* expr)
{
	switch(expr->type)
	{
		case SQL_MUL_INV :
		{
			snprintf_dstring(str_p, "1/(");
			snprint_sql_expr(str_p, expr->unary_of);
			snprintf_dstring(str_p, ")");
			break;
		}
		case SQL_NEG :
		{
			snprintf_dstring(str_p, "-(");
			snprint_sql_expr(str_p, expr->unary_of);
			snprintf_dstring(str_p, ")");
			break;
		}
		case SQL_BITNOT :
		{
			snprintf_dstring(str_p, "~(");
			snprint_sql_expr(str_p, expr->unary_of);
			snprintf_dstring(str_p, ")");
			break;
		}
		case SQL_LOGNOT :
		{
			snprintf_dstring(str_p, "NOT(");
			snprint_sql_expr(str_p, expr->unary_of);
			snprintf_dstring(str_p, ")");
			break;
		}

		case SQL_ADD :
		{
			snprintf_dstring(str_p, "(");
			snprint_sql_expr(str_p, expr->left);
			snprintf_dstring(str_p, ")+(");
			snprint_sql_expr(str_p, expr->right);
			snprintf_dstring(str_p, ")");
			break;
		}
		case SQL_SUB :
		{
			snprintf_dstring(str_p, "(");
			snprint_sql_expr(str_p, expr->left);
			snprintf_dstring(str_p, ")-(");
			snprint_sql_expr(str_p, expr->right);
			snprintf_dstring(str_p, ")");
			break;
		}
		case SQL_MUL :
		{
			snprintf_dstring(str_p, "(");
			snprint_sql_expr(str_p, expr->left);
			snprintf_dstring(str_p, ")*(");
			snprint_sql_expr(str_p, expr->right);
			snprintf_dstring(str_p, ")");
			break;
		}
		case SQL_DIV :
		{
			snprintf_dstring(str_p, "(");
			snprint_sql_expr(str_p, expr->left);
			snprintf_dstring(str_p, ")/(");
			snprint_sql_expr(str_p, expr->right);
			snprintf_dstring(str_p, ")");
			break;
		}
		case SQL_MOD :
		{
			snprintf_dstring(str_p, "(");
			snprint_sql_expr(str_p, expr->left);
			snprintf_dstring(str_p, ")%%(");
			snprint_sql_expr(str_p, expr->right);
			snprintf_dstring(str_p, ")");
			break;
		}
		case SQL_GT :
		{
			snprintf_dstring(str_p, "(");
			snprint_sql_expr(str_p, expr->left);
			snprintf_dstring(str_p, ")>");
			if(expr->cmp_rhs_quantfier != SQL_CMP_NONE) snprintf_dstring(str_p, "%s", ((expr->cmp_rhs_quantfier == SQL_CMP_ANY) ? "ANY" : "ALL"));
			snprintf_dstring(str_p, "(");
			if(expr->cmp_rhs_quantfier == SQL_CMP_NONE)
				snprint_sql_expr(str_p, expr->right);
			else
				snprint_dql(str_p, expr->right_sub_query);
			snprintf_dstring(str_p, ")");
			break;
		}
		case SQL_GTE :
		{
			snprintf_dstring(str_p, "(");
			snprint_sql_expr(str_p, expr->left);
			snprintf_dstring(str_p, ")>=");
			if(expr->cmp_rhs_quantfier != SQL_CMP_NONE) snprintf_dstring(str_p, "%s", ((expr->cmp_rhs_quantfier == SQL_CMP_ANY) ? "ANY" : "ALL"));
			snprintf_dstring(str_p, "(");
			if(expr->cmp_rhs_quantfier == SQL_CMP_NONE)
				snprint_sql_expr(str_p, expr->right);
			else
				snprint_dql(str_p, expr->right_sub_query);
			snprintf_dstring(str_p, ")");
			break;
		}
		case SQL_LT :
		{
			snprintf_dstring(str_p, "(");
			snprint_sql_expr(str_p, expr->left);
			snprintf_dstring(str_p, ")<");
			if(expr->cmp_rhs_quantfier != SQL_CMP_NONE) snprintf_dstring(str_p, "%s", ((expr->cmp_rhs_quantfier == SQL_CMP_ANY) ? "ANY" : "ALL"));
			snprintf_dstring(str_p, "(");
			if(expr->cmp_rhs_quantfier == SQL_CMP_NONE)
				snprint_sql_expr(str_p, expr->right);
			else
				snprint_dql(str_p, expr->right_sub_query);
			snprintf_dstring(str_p, ")");
			break;
		}
		case SQL_LTE :
		{
			snprintf_dstring(str_p, "(");
			snprint_sql_expr(str_p, expr->left);
			snprintf_dstring(str_p, ")<=");
			if(expr->cmp_rhs_quantfier != SQL_CMP_NONE) snprintf_dstring(str_p, "%s", ((expr->cmp_rhs_quantfier == SQL_CMP_ANY) ? "ANY" : "ALL"));
			snprintf_dstring(str_p, "(");
			if(expr->cmp_rhs_quantfier == SQL_CMP_NONE)
				snprint_sql_expr(str_p, expr->right);
			else
				snprint_dql(str_p, expr->right_sub_query);
			snprintf_dstring(str_p, ")");
			break;
		}
		case SQL_EQ :
		{
			snprintf_dstring(str_p, "(");
			snprint_sql_expr(str_p, expr->left);
			snprintf_dstring(str_p, ")=");
			if(expr->cmp_rhs_quantfier != SQL_CMP_NONE) snprintf_dstring(str_p, "%s", ((expr->cmp_rhs_quantfier == SQL_CMP_ANY) ? "ANY" : "ALL"));
			snprintf_dstring(str_p, "(");
			if(expr->cmp_rhs_quantfier == SQL_CMP_NONE)
				snprint_sql_expr(str_p, expr->right);
			else
				snprint_dql(str_p, expr->right_sub_query);
			snprintf_dstring(str_p, ")");
			break;
		}
		case SQL_NEQ :
		{
			snprintf_dstring(str_p, "(");
			snprint_sql_expr(str_p, expr->left);
			snprintf_dstring(str_p, ")<>");
			if(expr->cmp_rhs_quantfier != SQL_CMP_NONE) snprintf_dstring(str_p, "%s", ((expr->cmp_rhs_quantfier == SQL_CMP_ANY) ? "ANY" : "ALL"));
			snprintf_dstring(str_p, "(");
			if(expr->cmp_rhs_quantfier == SQL_CMP_NONE)
				snprint_sql_expr(str_p, expr->right);
			else
				snprint_dql(str_p, expr->right_sub_query);
			snprintf_dstring(str_p, ")");
			break;
		}
		case SQL_BITAND :
		{
			snprintf_dstring(str_p, "(");
			snprint_sql_expr(str_p, expr->left);
			snprintf_dstring(str_p, ")&(");
			snprint_sql_expr(str_p, expr->right);
			snprintf_dstring(str_p, ")");
			break;
		}
		case SQL_BITOR :
		{
			snprintf_dstring(str_p, "(");
			snprint_sql_expr(str_p, expr->left);
			snprintf_dstring(str_p, ")|(");
			snprint_sql_expr(str_p, expr->right);
			snprintf_dstring(str_p, ")");
			break;
		}
		case SQL_BITXOR :
		{
			snprintf_dstring(str_p, "(");
			snprint_sql_expr(str_p, expr->left);
			snprintf_dstring(str_p, ")^(");
			snprint_sql_expr(str_p, expr->right);
			snprintf_dstring(str_p, ")");
			break;
		}
		case SQL_LOGAND :
		{
			snprintf_dstring(str_p, "(");
			snprint_sql_expr(str_p, expr->left);
			snprintf_dstring(str_p, ")AND(");
			snprint_sql_expr(str_p, expr->right);
			snprintf_dstring(str_p, ")");
			break;
		}
		case SQL_LOGOR :
		{
			snprintf_dstring(str_p, "(");
			snprint_sql_expr(str_p, expr->left);
			snprintf_dstring(str_p, ")OR(");
			snprint_sql_expr(str_p, expr->right);
			snprintf_dstring(str_p, ")");
			break;
		}
		case SQL_LOGXOR :
		{
			snprintf_dstring(str_p, "(");
			snprint_sql_expr(str_p, expr->left);
			snprintf_dstring(str_p, ")XOR(");
			snprint_sql_expr(str_p, expr->right);
			snprintf_dstring(str_p, ")");
			break;
		}
		case SQL_LSHIFT :
		{
			snprintf_dstring(str_p, "(");
			snprint_sql_expr(str_p, expr->left);
			snprintf_dstring(str_p, ")<<(");
			snprint_sql_expr(str_p, expr->right);
			snprintf_dstring(str_p, ")");
			break;
		}
		case SQL_RSHIFT :
		{
			snprintf_dstring(str_p, "(");
			snprint_sql_expr(str_p, expr->left);
			snprintf_dstring(str_p, ")>>(");
			snprint_sql_expr(str_p, expr->right);
			snprintf_dstring(str_p, ")");
			break;
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
			snprintf_dstring(str_p, "(");
			snprint_sql_expr(str_p, expr->left);
			snprintf_dstring(str_p, ")IS ");
			snprint_sql_expr(str_p, expr->right);
			break;
		}

		case SQL_BTWN :
		{
			snprintf_dstring(str_p, "(");
			snprint_sql_expr(str_p, expr->btwn_input);
			snprintf_dstring(str_p, ")BETWEEN(");
			snprint_sql_expr(str_p, expr->bounds[0]);
			snprintf_dstring(str_p, ")AND(");
			snprint_sql_expr(str_p, expr->bounds[1]);
			snprintf_dstring(str_p, ")");
			break;
		}

		case SQL_ADD_FLAT :
		{
			for(cy_uint i = 0; i < get_element_count_arraylist(&(expr->expr_list)); i++)
			{
				if(i != 0)
					snprintf_dstring(str_p, "+");
				snprintf_dstring(str_p, "(");
				snprint_sql_expr(str_p, get_from_front_of_arraylist(&(expr->expr_list), i));
				snprintf_dstring(str_p, ")");
			}
			break;
		}
		case SQL_MUL_FLAT :
		{
			for(cy_uint i = 0; i < get_element_count_arraylist(&(expr->expr_list)); i++)
			{
				if(i != 0)
					snprintf_dstring(str_p, "*");
				snprintf_dstring(str_p, "(");
				snprint_sql_expr(str_p, get_from_front_of_arraylist(&(expr->expr_list), i));
				snprintf_dstring(str_p, ")");
			}
			break;
		}
		case SQL_LOGAND_FLAT :
		{
			for(cy_uint i = 0; i < get_element_count_arraylist(&(expr->expr_list)); i++)
			{
				if(i != 0)
					snprintf_dstring(str_p, "AND");
				snprintf_dstring(str_p, "(");
				snprint_sql_expr(str_p, get_from_front_of_arraylist(&(expr->expr_list), i));
				snprintf_dstring(str_p, ")");
			}
			break;
		}
		case SQL_LOGOR_FLAT :
		{
			for(cy_uint i = 0; i < get_element_count_arraylist(&(expr->expr_list)); i++)
			{
				if(i != 0)
					snprintf_dstring(str_p, "OR");
				snprintf_dstring(str_p, "(");
				snprint_sql_expr(str_p, get_from_front_of_arraylist(&(expr->expr_list), i));
				snprintf_dstring(str_p, ")");
			}
			break;
		}
		case SQL_LOGXOR_FLAT :
		{
			for(cy_uint i = 0; i < get_element_count_arraylist(&(expr->expr_list)); i++)
			{
				if(i != 0)
					snprintf_dstring(str_p, "XOR");
				snprintf_dstring(str_p, "(");
				snprint_sql_expr(str_p, get_from_front_of_arraylist(&(expr->expr_list), i));
				snprintf_dstring(str_p, ")");
			}
			break;
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
			snprintf_dstring(str_p, "'");
			concatenate_dstring(str_p, &(expr->value));
			snprintf_dstring(str_p, "'");
			break;
		}
		case SQL_NUM :
		{
			concatenate_dstring(str_p, &(expr->value));
			break;
		}
		case SQL_VAR :
		{
			concatenate_dstring(str_p, &(expr->value));
			break;
		}

		case SQL_TRUE :
		{
			snprintf_dstring(str_p, "TRUE");
			break;
		}
		case SQL_FALSE :
		{
			snprintf_dstring(str_p, "FALSE");
			break;
		}
		case SQL_UNKNOWN :
		{
			snprintf_dstring(str_p, "UNKNOWN");
			break;
		}
		case SQL_NULL :
		{
			snprintf_dstring(str_p, "NULL");
			break;
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
			snprintf_dstring(str_p, "CAST((");
			snprint_sql_expr(str_p, expr->cast_expr);
			snprintf_dstring(str_p, ") AS ");
			snprint_sql_type(str_p, expr->cast_type);
			snprintf_dstring(str_p, ")");
			break;
		}

		case SQL_SUB_QUERY :
		{
			snprintf_dstring(str_p, "(");
			snprint_dql(str_p, expr->sub_query);
			snprintf_dstring(str_p, ")");
			break;
		}

		case SQL_EXISTS :
		{
			snprintf_dstring(str_p, "EXISTS(");
			snprint_dql(str_p, expr->sub_query);
			snprintf_dstring(str_p, ")");
			break;
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
			for(cy_uint i = 0; i < min(get_element_count_arraylist(&(expr->when_exprs)), get_element_count_arraylist(&(expr->then_exprs))); i++)
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
}

void delete_sql_expr(sql_expression* expr)
{
	if(expr == NULL)
		return;

	switch(expr->type)
	{
		case SQL_MUL_INV :
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
			delete_sql_expr(expr->left);
			delete_sql_expr(expr->right);
			break;
		}

		case SQL_GT :
		case SQL_GTE :
		case SQL_LT :
		case SQL_LTE :
		case SQL_EQ :
		case SQL_NEQ :
		{
			delete_sql_expr(expr->left);
			if(expr->cmp_rhs_quantfier == SQL_CMP_NONE)
				delete_sql_expr(expr->right);
			else
				delete_dql(expr->right_sub_query);
			break;
		}

		case SQL_BTWN :
		{
			delete_sql_expr(expr->btwn_input);
			delete_sql_expr(expr->bounds[0]);
			delete_sql_expr(expr->bounds[1]);
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
				delete_sql_expr((sql_expression*)get_from_front_of_arraylist(&(expr->expr_list), i));
			deinitialize_arraylist(&(expr->expr_list));
			break;
		}

		case SQL_IN :
		{
			if(expr->in_sub_query)
				delete_dql(expr->in_sub_query);
			for(cy_uint i = 0; i < get_element_count_arraylist(&(expr->in_expr_list)); i++)
				delete_sql_expr((sql_expression*)get_from_front_of_arraylist(&(expr->in_expr_list), i));
			delete_sql_expr(expr->in_input);
			deinitialize_arraylist(&(expr->in_expr_list));
			break;
		}

		case SQL_NUM :
		case SQL_STR :
		case SQL_VAR :
		{
			deinit_dstring(&(expr->value));
			break;
		}

		case SQL_TRUE :
		case SQL_FALSE :
		case SQL_UNKNOWN :
		case SQL_NULL :
		{
			break;
		}

		case SQL_FUNCTION_CALL :
		{
			deinit_dstring(&(expr->func_name));
			for(cy_uint i = 0; i < get_element_count_arraylist(&(expr->param_expr_list)); i++)
				delete_sql_expr((sql_expression*)get_from_front_of_arraylist(&(expr->param_expr_list), i));
			deinitialize_arraylist(&(expr->param_expr_list));
			break;
		}

		case SQL_CAST :
		{
			delete_sql_expr(expr->cast_expr);
			delete_sql_type(expr->cast_type);
			break;
		}

		case SQL_SUB_QUERY :
		case SQL_EXISTS :
		{
			delete_dql(expr->sub_query);
			break;
		}

		case SQL_CASE :
		{
			if(expr->case_expr)
				delete_sql_expr(expr->case_expr);
			for(cy_uint i = 0; i < get_element_count_arraylist(&(expr->when_exprs)); i++)
			{
				sql_expression* x = (sql_expression*) get_from_front_of_arraylist(&(expr->when_exprs), i);
				delete_sql_expr(x);
			}
			deinitialize_arraylist(&(expr->when_exprs));
			for(cy_uint i = 0; i < get_element_count_arraylist(&(expr->then_exprs)); i++)
			{
				sql_expression* x = (sql_expression*) get_from_front_of_arraylist(&(expr->then_exprs), i);
				delete_sql_expr(x);
			}
			deinitialize_arraylist(&(expr->then_exprs));
			if(expr->else_expr)
				delete_sql_expr(expr->else_expr);
			break;
		}
	}
	free(expr);
}