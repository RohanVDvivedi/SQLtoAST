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

sql_expression* new_cast_sql_expr(sql_expression* cast_expr, sql_type cast_type)
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
			else // else insert right as is
				insert_in_expr_list(&expr_list, new_unary_sql_expr(SQL_NEG, right));

			return new_flat_sql_expr(flat_type, expr_list);
		}

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
					sql_expression* neg_t = NULL;
					if(t->type == SQL_MUL_INV) // remove (--) in sequence to just a flatenned +
					{
						neg_t = t->unary_of;
						free(t);
					}
					else
						neg_t = new_unary_sql_expr(SQL_MUL_INV, t);
					// from here on t may not exist, and may have been deleted
					insert_in_expr_list(&expr_list, neg_t);
				}

				// destroy just the right child
				deinitialize_arraylist(&(right->expr_list));
				free(right);
			}
			else // else insert right as is
				insert_in_expr_list(&expr_list, new_unary_sql_expr(SQL_MUL_INV, right));

			return new_flat_sql_expr(flat_type, expr_list);
		}

		case SQL_MUL_INV :
		case SQL_NEG :
		case SQL_BITNOT :
		case SQL_LOGNOT :
		{
			expr->unary_of = flatten_similar_associative_operators_in_sql_expression(expr->unary_of);
			if(expr->unary_of->type == expr->type)
			{
				sql_expression* flat_expr = expr->unary_of->unary_of;
				free(expr->unary_of);
				free(expr);
				return flat_expr;
			}
			else
				return expr;
		}

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

void print_sql_expr(const sql_expression* expr)
{
	switch(expr->type)
	{
		case SQL_MUL_INV :
		{
			printf("( 1/( ");
			print_sql_expr(expr->unary_of);
			printf(" ) )");
			break;
		}
		case SQL_NEG :
		{
			printf("( -( ");
			print_sql_expr(expr->unary_of);
			printf(" ) )");
			break;
		}
		case SQL_BITNOT :
		{
			printf("( ~( ");
			print_sql_expr(expr->unary_of);
			printf(" ) )");
			break;
		}
		case SQL_LOGNOT :
		{
			printf("( !( ");
			print_sql_expr(expr->unary_of);
			printf(" ) )");
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
			printf(" %% ");
			print_sql_expr(expr->right);
			printf(" )");
			break;
		}
		case SQL_GT :
		{
			printf("( ");
			print_sql_expr(expr->left);
			printf(" > ");
			if(expr->cmp_rhs_quantfier != SQL_CMP_NONE) printf("%s", ((expr->cmp_rhs_quantfier == SQL_CMP_ANY) ? "ANY" : "ALL"));
			if(expr->cmp_rhs_quantfier == SQL_CMP_NONE)
				print_sql_expr(expr->right);
			else
			{
				printf("( ");
				print_dql(expr->right_sub_query);
				printf(" )");
			}
			printf(" )");
			break;
		}
		case SQL_GTE :
		{
			printf("( ");
			print_sql_expr(expr->left);
			printf(" >= ");
			if(expr->cmp_rhs_quantfier != SQL_CMP_NONE) printf("%s", ((expr->cmp_rhs_quantfier == SQL_CMP_ANY) ? "ANY" : "ALL"));
			if(expr->cmp_rhs_quantfier == SQL_CMP_NONE)
				print_sql_expr(expr->right);
			else
			{
				printf("( ");
				print_dql(expr->right_sub_query);
				printf(" )");
			}
			printf(" )");
			break;
		}
		case SQL_LT :
		{
			printf("( ");
			print_sql_expr(expr->left);
			printf(" < ");
			if(expr->cmp_rhs_quantfier != SQL_CMP_NONE) printf("%s", ((expr->cmp_rhs_quantfier == SQL_CMP_ANY) ? "ANY" : "ALL"));
			if(expr->cmp_rhs_quantfier == SQL_CMP_NONE)
				print_sql_expr(expr->right);
			else
			{
				printf("( ");
				print_dql(expr->right_sub_query);
				printf(" )");
			}
			printf(" )");
			break;
		}
		case SQL_LTE :
		{
			printf("( ");
			print_sql_expr(expr->left);
			printf(" <= ");
			if(expr->cmp_rhs_quantfier != SQL_CMP_NONE) printf("%s", ((expr->cmp_rhs_quantfier == SQL_CMP_ANY) ? "ANY" : "ALL"));
			if(expr->cmp_rhs_quantfier == SQL_CMP_NONE)
				print_sql_expr(expr->right);
			else
			{
				printf("( ");
				print_dql(expr->right_sub_query);
				printf(" )");
			}
			printf(" )");
			break;
		}
		case SQL_EQ :
		{
			printf("( ");
			print_sql_expr(expr->left);
			printf(" = ");
			if(expr->cmp_rhs_quantfier != SQL_CMP_NONE) printf("%s", ((expr->cmp_rhs_quantfier == SQL_CMP_ANY) ? "ANY" : "ALL"));
			if(expr->cmp_rhs_quantfier == SQL_CMP_NONE)
				print_sql_expr(expr->right);
			else
			{
				printf("( ");
				print_dql(expr->right_sub_query);
				printf(" )");
			}
			printf(" )");
			break;
		}
		case SQL_NEQ :
		{
			printf("( ");
			print_sql_expr(expr->left);
			printf(" <> ");
			if(expr->cmp_rhs_quantfier != SQL_CMP_NONE) printf("%s", ((expr->cmp_rhs_quantfier == SQL_CMP_ANY) ? "ANY" : "ALL"));
			if(expr->cmp_rhs_quantfier == SQL_CMP_NONE)
				print_sql_expr(expr->right);
			else
			{
				printf("( ");
				print_dql(expr->right_sub_query);
				printf(" )");
			}
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
		case SQL_LSHIFT :
		{
			printf("( ");
			print_sql_expr(expr->left);
			printf(" << ");
			print_sql_expr(expr->right);
			printf(" )");
			break;
		}
		case SQL_RSHIFT :
		{
			printf("( ");
			print_sql_expr(expr->left);
			printf(" >> ");
			print_sql_expr(expr->right);
			printf(" )");
			break;
		}
		case SQL_CONCAT :
		{
			printf("( ");
			print_sql_expr(expr->left);
			printf(" || ");
			print_sql_expr(expr->right);
			printf(" )");
			break;
		}
		case SQL_LIKE :
		{
			printf("( ");
			print_sql_expr(expr->left);
			printf(" LIKE ");
			print_sql_expr(expr->right);
			printf(" )");
			break;
		}
		case SQL_IS :
		{
			printf("( ");
			print_sql_expr(expr->left);
			printf(" IS ");
			print_sql_expr(expr->right);
			printf(" )");
			break;
		}

		case SQL_BTWN :
		{
			printf("( ");
			print_sql_expr(expr->btwn_input);
			printf(" BETWEEN [ ");
			print_sql_expr(expr->bounds[0]);
			printf(" , ");
			print_sql_expr(expr->bounds[1]);
			printf(" ] )");
			break;
		}

		case SQL_ADD_FLAT :
		{
			printf("( ");
			for(cy_uint i = 0; i < get_element_count_arraylist(&(expr->expr_list)); i++)
			{
				if(i != 0)
					printf(" + ");
				print_sql_expr(get_from_front_of_arraylist(&(expr->expr_list), i));
			}
			printf(" )");
			break;
		}
		case SQL_MUL_FLAT :
		{
			printf("( ");
			for(cy_uint i = 0; i < get_element_count_arraylist(&(expr->expr_list)); i++)
			{
				if(i != 0)
					printf(" * ");
				print_sql_expr(get_from_front_of_arraylist(&(expr->expr_list), i));
			}
			printf(" )");
			break;
		}
		case SQL_LOGAND_FLAT :
		{
			printf("( ");
			for(cy_uint i = 0; i < get_element_count_arraylist(&(expr->expr_list)); i++)
			{
				if(i != 0)
					printf(" AND ");
				print_sql_expr(get_from_front_of_arraylist(&(expr->expr_list), i));
			}
			printf(" )");
			break;
		}
		case SQL_LOGOR_FLAT :
		{
			printf("( ");
			for(cy_uint i = 0; i < get_element_count_arraylist(&(expr->expr_list)); i++)
			{
				if(i != 0)
					printf(" OR ");
				print_sql_expr(get_from_front_of_arraylist(&(expr->expr_list), i));
			}
			printf(" )");
			break;
		}
		case SQL_LOGXOR_FLAT :
		{
			printf("( ");
			for(cy_uint i = 0; i < get_element_count_arraylist(&(expr->expr_list)); i++)
			{
				if(i != 0)
					printf(" XOR ");
				print_sql_expr(get_from_front_of_arraylist(&(expr->expr_list), i));
			}
			printf(" )");
			break;
		}
		case SQL_CONCAT_FLAT :
		{
			printf("( ");
			for(cy_uint i = 0; i < get_element_count_arraylist(&(expr->expr_list)); i++)
			{
				if(i != 0)
					printf(" || ");
				print_sql_expr(get_from_front_of_arraylist(&(expr->expr_list), i));
			}
			printf(" )");
			break;
		}
		case SQL_IN :
		{
			printf("( ");
			print_sql_expr(expr->in_input);
			printf(" IN ( ");
			if(expr->in_sub_query)
				print_dql(expr->in_sub_query);
			else
			{
				for(cy_uint i = 0; i < get_element_count_arraylist(&(expr->in_expr_list)); i++)
				{
					if(i != 0)
						printf(" , ");
					print_sql_expr(get_from_front_of_arraylist(&(expr->in_expr_list), i));
				}
			}
			printf(" ) )");
			break;
		}

		case SQL_STR :
		{
			printf("'");
			printf_dstring(&(expr->value));
			printf("'");
			break;
		}
		case SQL_NUM :
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

		case SQL_TRUE :
		{
			printf("true");
			break;
		}
		case SQL_FALSE :
		{
			printf("false");
			break;
		}
		case SQL_UNKNOWN :
		{
			printf("unknown");
			break;
		}
		case SQL_NULL :
		{
			printf("null");
			break;
		}

		case SQL_FUNCTION_CALL :
		{
			printf("( ");
			printf_dstring(&(expr->func_name));
			printf("( ");
			if(expr->aggregate_mode == SQL_RESULT_SET_DISTINCT)
				printf("DISTINCT ");
			for(cy_uint i = 0; i < get_element_count_arraylist(&(expr->param_expr_list)); i++)
			{
				if(i != 0)
					printf(" , ");
				print_sql_expr(get_from_front_of_arraylist(&(expr->param_expr_list), i));
			}
			printf(" ) )");
			break;
		}

		case SQL_CAST :
		{
			printf("( CAST ( ");
			print_sql_expr(expr->cast_expr);
			printf(" ) AS ( ");
			print_sql_type(&(expr->cast_type));
			printf(" ) )");
			break;
		}

		case SQL_SUB_QUERY :
		{
			printf("( ");
			print_dql(expr->sub_query);
			printf(" )");
			break;
		}

		case SQL_EXISTS :
		{
			printf("( EXISTS( ");
			print_dql(expr->sub_query);
			printf(" ) )");
			break;
		}

		case SQL_CASE :
		{
			int clauses_printed = 0;
			printf("( CASE( ");
			if(expr->case_expr)
			{
				if(clauses_printed != 0)
					printf(" , ");
				print_sql_expr(expr->case_expr);
				clauses_printed++;
			}
			for(cy_uint i = 0; i < min(get_element_count_arraylist(&(expr->when_exprs)), get_element_count_arraylist(&(expr->then_exprs))); i++)
			{
				{
					const sql_expression* when = get_from_front_of_arraylist(&(expr->when_exprs), i);
					if(clauses_printed != 0)
						printf(" , ");
					printf("WHEN( ");
					print_sql_expr(when);
					printf(" )");
					clauses_printed++;
				}

				{
					const sql_expression* then = get_from_front_of_arraylist(&(expr->then_exprs), i);
					if(clauses_printed != 0)
						printf(" , ");
					printf("THEN( ");
					print_sql_expr(then);
					printf(" )");
					clauses_printed++;
				}
			}
			if(expr->else_expr)
			{
				if(clauses_printed != 0)
					printf(" , ");
				printf("ELSE( ");
				print_sql_expr(expr->else_expr);
				printf(" )");
				clauses_printed++;
			}
			printf(" ) )");
			break;
		}
	}
}

void delete_sql_expr(sql_expression* expr)
{
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