#include<sqltoast/sql_dql.h>

#include<sqltoast/arraylist_deleter.h>

#include<stdio.h>
#include<stdlib.h>

sql_dql* new_dql(sql_dql_type type)
{
	sql_dql* dql = malloc(sizeof(sql_dql));

	dql->type = type;

	switch(dql->type)
	{
		case SELECT_QUERY :
		{
			initialize_arraylist(&(dql->select_query.projections), 0);

			init_relation_input(&(dql->select_query.base_input), new_copy_dstring(&get_dstring_pointing_to_cstring("")), new_copy_dstring(&get_dstring_pointing_to_cstring("")));

			initialize_arraylist(&(dql->select_query.joins_with), 0);

			dql->select_query.where_expr = NULL;
			initialize_arraylist(&(dql->select_query.group_by), 0);
			dql->select_query.having_expr = NULL;
			initialize_arraylist(&(dql->select_query.ordered_by), 0);
			dql->select_query.offset_expr = NULL;
			dql->select_query.limit_expr = NULL;

			break;
		}
		case VALUES_QUERY :
		{
			initialize_arraylist(&(dql->values_query.values), 0);
			break;
		}
		case SET_OPERATION :
		{
			dql->set_operation.left = NULL;
			dql->set_operation.right = NULL;
			break;
		}
	}

	return dql;
}

static void flatten_exprs_relation_input(relation_input* ri_p)
{
	switch(ri_p->type)
	{
		case RELATION :
		{
			break;
		}
		case SUB_QUERY :
		{
			flatten_exprs_dql(ri_p->sub_query);
			break;
		}
		case FUNCTION_CALL :
		{
			ri_p->function_call = flatten_similar_associative_operators_in_sql_expression(ri_p->function_call);
			break;
		}
	}
}

void flatten_exprs_dql(sql_dql* dql)
{
	switch(dql->type)
	{
		case SELECT_QUERY :
		{
			if(get_element_count_arraylist(&(dql->select_query.projections)) > 0)
			{
				for(cy_uint i = 0; i < get_element_count_arraylist(&(dql->select_query.projections)); i++)
				{
					projection* p = (projection*) get_from_front_of_arraylist(&(dql->select_query.projections), i);
					p->projection_expr = flatten_similar_associative_operators_in_sql_expression(p->projection_expr);
				}
			}

			flatten_exprs_relation_input(&(dql->select_query.base_input));

			if(get_element_count_arraylist(&(dql->select_query.joins_with)) > 0)
			{
				for(cy_uint i = 0; i < get_element_count_arraylist(&(dql->select_query.joins_with)); i++)
				{
					join_with* j = (join_with*) get_from_front_of_arraylist(&(dql->select_query.joins_with), i);
					flatten_exprs_relation_input(&(j->input));
					switch(j->condition_type)
					{
						case NO_JOIN_CONDITION :
							break;
						case NATURAL_JOIN_CONDITION :
							break;
						case ON_EXPR_JOIN_CONDITION :
						{
							j->on_expr = flatten_similar_associative_operators_in_sql_expression(j->on_expr);
							break;
						}
						case USING_JOIN_CONDITION :
							break;
					}
				}
			}

			if(dql->select_query.where_expr)
				dql->select_query.where_expr = flatten_similar_associative_operators_in_sql_expression(dql->select_query.where_expr);

			if(get_element_count_arraylist(&(dql->select_query.group_by)) > 0)
			{
				for(cy_uint i = 0; i < get_element_count_arraylist(&(dql->select_query.group_by)); i++)
				{
					sql_expression* g = (sql_expression*) get_from_front_of_arraylist(&(dql->select_query.group_by), i);
					g = flatten_similar_associative_operators_in_sql_expression(g);
					set_from_front_in_arraylist(&(dql->select_query.group_by), g, i);
				}
			}

			
			if(dql->select_query.having_expr)
				dql->select_query.having_expr = flatten_similar_associative_operators_in_sql_expression(dql->select_query.having_expr);

			if(get_element_count_arraylist(&(dql->select_query.ordered_by)) > 0)
			{
				for(cy_uint i = 0; i < get_element_count_arraylist(&(dql->select_query.ordered_by)); i++)
				{
					order_by* o = (order_by*) get_from_front_of_arraylist(&(dql->select_query.ordered_by), i);
					o->ordering_expr = flatten_similar_associative_operators_in_sql_expression(o->ordering_expr);
				}
			}

			if(dql->select_query.offset_expr)
				dql->select_query.offset_expr = flatten_similar_associative_operators_in_sql_expression(dql->select_query.offset_expr);

			if(dql->select_query.limit_expr)
				dql->select_query.limit_expr = flatten_similar_associative_operators_in_sql_expression(dql->select_query.limit_expr);

			break;
		}
		case VALUES_QUERY :
		{
			for(cy_uint i = 0; i < get_element_count_arraylist(&(dql->values_query.values)); i++)
			{
				arraylist* row = (arraylist*) get_from_front_of_arraylist(&(dql->values_query.values), i);
				for(cy_uint j = 0; j < get_element_count_arraylist(row); j++)
				{
					sql_expression* expr = (sql_expression*) get_from_front_of_arraylist(row, j);
					if(expr != NULL)
					{
						expr = flatten_similar_associative_operators_in_sql_expression(expr);
						set_from_front_in_arraylist(row, expr, j);
					}
				}
			}
			break;
		}
		case SET_OPERATION :
		{
			flatten_exprs_dql(dql->set_operation.left);
			flatten_exprs_dql(dql->set_operation.right);
			break;
		}
	}
}

static void print_relation_input(const relation_input* ri_p)
{
	switch(ri_p->type)
	{
		case RELATION :
		{
			printf_dstring(&(ri_p->relation_name));
			break;
		}
		case SUB_QUERY :
		{
			printf("(");
			print_dql(ri_p->sub_query);
			printf(")");
			break;
		}
		case FUNCTION_CALL :
		{			print_sql_expr(ri_p->function_call);
			break;
		}
	}
	if(!is_empty_dstring(&(ri_p->as)))
	{
		printf(" AS ");
		printf_dstring(&(ri_p->as));
		if(get_element_count_arraylist(&(ri_p->columns_as)) > 0)
		{
			printf("(");
			for(cy_uint i = 0; i < get_element_count_arraylist(&(ri_p->columns_as)); i++)
			{
				if(i != 0)
					printf(",");
				printf_dstring((const dstring*) get_from_front_of_arraylist(&(ri_p->columns_as), i));
			}
			printf(")");
		}
	}
}

void print_dql(const sql_dql* dql)
{
	switch(dql->type)
	{
		case SELECT_QUERY :
		{
			printf("SELECT ");

			if(dql->select_query.projection_mode == SQL_RESULT_SET_DISTINCT)
				printf("DISTINCT ");

			if(get_element_count_arraylist(&(dql->select_query.projections)) > 0)
			{
				for(cy_uint i = 0; i < get_element_count_arraylist(&(dql->select_query.projections)); i++)
				{
					if(i != 0)
						printf(",");
					projection* p = (projection*) get_from_front_of_arraylist(&(dql->select_query.projections), i);
					printf("(");
					print_sql_expr(p->projection_expr);
					printf(")");
					if(!is_empty_dstring(&(p->as)))
					{
						printf(" AS ");
						printf_dstring(&(p->as));
					}
					printf(" ");
				}
			}

			{
				printf("FROM ");
				print_relation_input(&(dql->select_query.base_input));
				printf(" ");
			}

			if(get_element_count_arraylist(&(dql->select_query.joins_with)) > 0)
			{
				for(cy_uint i = 0; i < get_element_count_arraylist(&(dql->select_query.joins_with)); i++)
				{
					const join_with* j = get_from_front_of_arraylist(&(dql->select_query.joins_with), i);

					if(j->condition_type == NATURAL_JOIN_CONDITION)
						printf("NATURAL ");

					switch(j->type)
					{
						case INNER_JOIN: printf("INNER JOIN "); break;
						case LEFT_JOIN:  printf("LEFT JOIN ");  break;
						case RIGHT_JOIN: printf("RIGHT JOIN "); break;
						case FULL_JOIN:  printf("FULL JOIN ");  break;
						case CROSS_JOIN: printf("CROSS JOIN "); break;
					}

					if(j->is_lateral)
						printf("LATERAL ");

					print_relation_input(&(j->input));

					switch(j->condition_type)
					{
						case NO_JOIN_CONDITION:
							break;
						case NATURAL_JOIN_CONDITION:
							break;
						case ON_EXPR_JOIN_CONDITION:
						{
							printf("ON ");
							print_sql_expr(j->on_expr);
							break;
						}
						case USING_JOIN_CONDITION:
						{
							printf("USING (");
							for(cy_uint k = 0; k < get_element_count_arraylist(&(j->using_cols)); k++)
							{
								if(k != 0)
									printf(",");
								printf_dstring(get_from_front_of_arraylist(&(j->using_cols), k));
							}
							printf(")");
							break;
						}
					}
					printf(" ");
				}
			}

			if(dql->select_query.where_expr)
			{
				printf("WHERE (");
				print_sql_expr(dql->select_query.where_expr);
				printf(") ");
			}

			if(get_element_count_arraylist(&(dql->select_query.group_by)) > 0)
			{
				printf("GROUP BY ");
				for(cy_uint i = 0; i < get_element_count_arraylist(&(dql->select_query.group_by)); i++)
				{
					if(i != 0)
						printf(",");
					sql_expression* g = (sql_expression*) get_from_front_of_arraylist(&(dql->select_query.group_by), i);
					printf("(");
					print_sql_expr(g);
					printf(") ");
				}
			}

			
			if(dql->select_query.having_expr)
			{
				printf("HAVING (");
				print_sql_expr(dql->select_query.having_expr);
				printf(") ");
			}

			if(get_element_count_arraylist(&(dql->select_query.ordered_by)) > 0)
			{
				printf("ORDER BY ");
				for(cy_uint i = 0; i < get_element_count_arraylist(&(dql->select_query.ordered_by)); i++)
				{
					if(i != 0)
						printf(",");
					order_by* o = (order_by*) get_from_front_of_arraylist(&(dql->select_query.ordered_by), i);
					printf("(");
					print_sql_expr(o->ordering_expr);
					printf(") %s ", ((o->dir == ORDER_BY_ASC) ? "ASC" : "DESC"));
				}
			}

			if(dql->select_query.offset_expr)
			{
				printf("OFFSET (");
				print_sql_expr(dql->select_query.offset_expr);
				printf(") ");
			}

			if(dql->select_query.limit_expr)
			{
				printf("LIMIT (");
				print_sql_expr(dql->select_query.limit_expr);
				printf(") ");
			}

			break;
		}
		case VALUES_QUERY :
		{
			printf("VALUES ");
			for(cy_uint i = 0; i < get_element_count_arraylist(&(dql->values_query.values)); i++)
			{
				if(i != 0)
					printf(",");
				const arraylist* row = get_from_front_of_arraylist(&(dql->values_query.values), i);
				printf("(");
				for(cy_uint j = 0; j < get_element_count_arraylist(row); j++)
				{
					if(j != 0)
						printf(",");
					const sql_expression* expr = get_from_front_of_arraylist(row, j);
					if(expr == NULL)
						printf("DEFAULT");
					else
						print_sql_expr(expr);
				}
				printf(")");
			}
			break;
		}
		case SET_OPERATION :
		{
			printf("(");print_dql(dql->set_operation.left);printf(")");
			switch(dql->set_operation.op_type)
			{
				case SQL_SET_INTERSECT :
				{
					printf(" INTERSECT ");
					break;
				}
				case SQL_SET_UNION :
				{
					printf(" UNION ");
					break;
				}
				case SQL_SET_EXCEPT :
				{
					printf(" EXCEPT ");
					break;
				}
			}
			switch(dql->set_operation.op_mod)
			{
				case SQL_RESULT_SET_DISTINCT :
				{
					printf(" DISTINCT ");
					break;
				}
				case SQL_RESULT_SET_ALL :
				{
					printf(" ALL ");
					break;
				}
			}
			printf("(");print_dql(dql->set_operation.right);printf(")");
			break;
		}
	}
}

void destroy_relation_input(relation_input* ri_p)
{
	switch(ri_p->type)
	{
		case RELATION :
		{
			deinit_dstring(&(ri_p->relation_name));
			break;
		}
		case SUB_QUERY :
		{
			delete_dql(ri_p->sub_query);
			break;
		}
		case FUNCTION_CALL :
		{
			delete_sql_expr(ri_p->function_call);
			break;
		}
	}
	deinit_dstring(&(ri_p->as));
	for(cy_uint i = 0; i < get_element_count_arraylist(&(ri_p->columns_as)); i++)
	{
		dstring* c_as = (dstring*) get_from_front_of_arraylist(&(ri_p->columns_as), i);
		deinit_dstring(c_as);
		free(c_as);
	}
	deinitialize_arraylist(&(ri_p->columns_as));
}

void delete_projection(projection* p)
{
	delete_sql_expr(p->projection_expr);
	deinit_dstring(&(p->as));
	free(p);
}

void delete_join_with(join_with* j)
{
	destroy_relation_input(&(j->input));
	switch(j->condition_type)
	{
		case ON_EXPR_JOIN_CONDITION :
		{
			delete_sql_expr(j->on_expr);
			break;
		}
		case USING_JOIN_CONDITION :
		{
			for(cy_uint i = 0; i < get_element_count_arraylist(&(j->using_cols)); i++)
			{
				dstring* col = (dstring*) get_from_front_of_arraylist(&(j->using_cols), i);
				deinit_dstring(col);
				free(col);
			}
			deinitialize_arraylist(&(j->using_cols));
			break;
		}
		default :
			break;
	}
	free(j);
}

void delete_order_by(order_by* o)
{
	delete_sql_expr(o->ordering_expr);
	free(o);
}

void delete_dql(sql_dql* dql)
{
	switch(dql->type)
	{
		case SELECT_QUERY :
		{
			delete_all_and_deinitialize_arraylist_1d(&(dql->select_query.projections), (void(*)(void*))delete_projection);

			destroy_relation_input(&(dql->select_query.base_input));

			delete_all_and_deinitialize_arraylist_1d(&(dql->select_query.joins_with), (void(*)(void*))delete_join_with);

			if(dql->select_query.where_expr)
				delete_sql_expr(dql->select_query.where_expr);

			delete_all_and_deinitialize_arraylist_1d(&(dql->select_query.group_by), (void(*)(void*))delete_sql_expr);

			if(dql->select_query.having_expr)
				delete_sql_expr(dql->select_query.having_expr);

			delete_all_and_deinitialize_arraylist_1d(&(dql->select_query.ordered_by), (void(*)(void*))delete_order_by);

			if(dql->select_query.offset_expr)
				delete_sql_expr(dql->select_query.offset_expr);

			if(dql->select_query.limit_expr)
				delete_sql_expr(dql->select_query.limit_expr);

			break;
		}
		case VALUES_QUERY :
		{
			delete_all_and_deinitialize_arraylist_2d(&(dql->values_query.values), (void(*)(void*))delete_sql_expr);
			break;
		}
		case SET_OPERATION :
		{
			delete_dql(dql->set_operation.left);
			delete_dql(dql->set_operation.right);
			break;
		}
	}

	free(dql);
}