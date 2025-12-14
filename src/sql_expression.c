#include<sqltoast/sql_expression.h>

#include<stdlib.h>
#include<stdio.h>

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
	expr->btwn_input = input;
	expr->bounds[0] = bounds0;
	expr->bounds[1] = bounds1;
	return expr;
}

sql_expression* new_flat_sql_expr(sql_expression_type type)
{
	sql_expression* expr = malloc(sizeof(sql_expression));
	expr->type = type;
	initialize_arraylist(&(expr->expr_list), 5);
	return expr;
}

void insert_expr_to_flat_sql_expr(sql_expression* expr, sql_expression* from_val)
{
	if(is_full_arraylist(&(expr->expr_list)) && !expand_arraylist(&(expr->expr_list)))
	{
		printf("failed to insert back in expression list for flat operator\n");
		exit(-1);
	}
	push_back_to_arraylist(&(expr->expr_list), from_val);
}

void convert_flat_to_in_sql_expr(sql_expression* expr, sql_expression* input)
{
	expr->type = SQL_IN;
	arraylist temp = expr->expr_list;
	expr->in_expr_list = temp;
	expr->in_input = input;
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

			sql_expression* flat_expr = new_flat_sql_expr(expr->type + 1); // the next type is always the flattenned operator

			// free up the, memory for the old binary version of this operator
			free(expr);

			// only similar flat expression types can be made flat
			if(left->type == flat_expr->type)
			{
				for(cy_uint i = 0; i < get_element_count_arraylist(&(left->expr_list)); i++)
					insert_expr_to_flat_sql_expr(flat_expr, (sql_expression*) get_from_front_of_arraylist(&(left->expr_list), i));

				// destroy just the left child
				deinitialize_arraylist(&(left->expr_list));
				free(left);
			}
			else // else insert left as is
				insert_expr_to_flat_sql_expr(flat_expr, left);

			// only similar flat expression types can be made flat
			if(right->type == flat_expr->type)
			{
				for(cy_uint i = 0; i < get_element_count_arraylist(&(right->expr_list)); i++)
					insert_expr_to_flat_sql_expr(flat_expr, (sql_expression*) get_from_front_of_arraylist(&(right->expr_list), i));

				// destroy just the left child
				deinitialize_arraylist(&(right->expr_list));
				free(right);
			}
			else // else insert right as is
				insert_expr_to_flat_sql_expr(flat_expr, right);

			return flat_expr;
		}

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

		case SQL_SUB :
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
		case SQL_LSHIFT :
		case SQL_RSHIFT :
		case SQL_LIKE :
		case SQL_IS :
		{
			expr->left = flatten_similar_associative_operators_in_sql_expression(expr->left);
			expr->right = flatten_similar_associative_operators_in_sql_expression(expr->right);
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
	}

	return NULL;
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
			for(cy_uint i = 0; i < get_element_count_arraylist(&(expr->in_expr_list)); i++)
			{
				if(i != 0)
					printf(" , ");
				print_sql_expr(get_from_front_of_arraylist(&(expr->in_expr_list), i));
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
	}
	free(expr);
}