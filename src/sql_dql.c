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

			dql->select_query.has_base_input = 0;
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

static void snprint_relation_input(dstring* str_p, const relation_input* ri_p)
{
	switch(ri_p->type)
	{
		case RELATION :
		{
			concatenate_dstring(str_p, &(ri_p->relation_name));
			break;
		}
		case SUB_QUERY :
		{
			snprintf_dstring(str_p, "(");
			snprint_dql(str_p, ri_p->sub_query);
			snprintf_dstring(str_p, ")");
			break;
		}
		case FUNCTION_CALL :
		{
			snprint_sql_expr(str_p, ri_p->function_call);
			break;
		}
	}
	if(!is_empty_dstring(&(ri_p->as)))
	{
		snprintf_dstring(str_p, " AS ");
		concatenate_dstring(str_p, &(ri_p->as));
		if(get_element_count_arraylist(&(ri_p->columns_as)) > 0)
		{
			snprintf_dstring(str_p, "(");
			for(cy_uint i = 0; i < get_element_count_arraylist(&(ri_p->columns_as)); i++)
			{
				if(i != 0)
					snprintf_dstring(str_p, ",");
				concatenate_dstring(str_p, (const dstring*) get_from_front_of_arraylist(&(ri_p->columns_as), i));
			}
			snprintf_dstring(str_p, ")");
		}
	}
}

void snprint_dql(dstring* str_p, const sql_dql* dql)
{
	switch(dql->type)
	{
		case SELECT_QUERY :
		{
			snprintf_dstring(str_p, "SELECT");

			if(dql->select_query.projection_mode == SQL_RESULT_SET_DISTINCT)
				snprintf_dstring(str_p, " DISTINCT");

			if(get_element_count_arraylist(&(dql->select_query.projections)) > 0)
			{
				snprintf_dstring(str_p, " ");
				for(cy_uint i = 0; i < get_element_count_arraylist(&(dql->select_query.projections)); i++)
				{
					if(i != 0)
						snprintf_dstring(str_p, ",");
					projection* p = (projection*) get_from_front_of_arraylist(&(dql->select_query.projections), i);
					snprintf_dstring(str_p, "(");
					snprint_sql_expr(str_p, p->projection_expr);
					snprintf_dstring(str_p, ")");
					if(!is_empty_dstring(&(p->as)))
					{
						snprintf_dstring(str_p, " AS ");
						concatenate_dstring(str_p, &(p->as));
					}
				}
			}

			if(dql->select_query.has_base_input)
			{
				snprintf_dstring(str_p, " FROM ");
				snprint_relation_input(str_p, &(dql->select_query.base_input));
			}

			if(get_element_count_arraylist(&(dql->select_query.joins_with)) > 0)
			{
				snprintf_dstring(str_p, " ");
				for(cy_uint i = 0; i < get_element_count_arraylist(&(dql->select_query.joins_with)); i++)
				{
					const join_with* j = get_from_front_of_arraylist(&(dql->select_query.joins_with), i);

					if(j->condition_type == NATURAL_JOIN_CONDITION)
						snprintf_dstring(str_p, "NATURAL ");

					switch(j->type)
					{
						case INNER_JOIN: snprintf_dstring(str_p, "INNER JOIN "); break;
						case LEFT_JOIN:  snprintf_dstring(str_p, "LEFT JOIN ");  break;
						case RIGHT_JOIN: snprintf_dstring(str_p, "RIGHT JOIN "); break;
						case FULL_JOIN:  snprintf_dstring(str_p, "FULL JOIN ");  break;
						case CROSS_JOIN: snprintf_dstring(str_p, "CROSS JOIN "); break;
					}

					if(j->is_lateral)
						snprintf_dstring(str_p, "LATERAL ");

					snprint_relation_input(str_p, &(j->input));
					snprintf_dstring(str_p, " ");

					switch(j->condition_type)
					{
						case NO_JOIN_CONDITION:
							break;
						case NATURAL_JOIN_CONDITION:
							break;
						case ON_EXPR_JOIN_CONDITION:
						{
							snprintf_dstring(str_p, "ON ");
							snprint_sql_expr(str_p, j->on_expr);
							break;
						}
						case USING_JOIN_CONDITION:
						{
							snprintf_dstring(str_p, "USING (");
							for(cy_uint k = 0; k < get_element_count_arraylist(&(j->using_cols)); k++)
							{
								if(k != 0)
									snprintf_dstring(str_p, ",");
								concatenate_dstring(str_p, get_from_front_of_arraylist(&(j->using_cols), k));
							}
							snprintf_dstring(str_p, ")");
							break;
						}
					}
					snprintf_dstring(str_p, " ");
				}
			}

			if(dql->select_query.where_expr)
			{
				snprintf_dstring(str_p, " WHERE (");
				snprint_sql_expr(str_p, dql->select_query.where_expr);
				snprintf_dstring(str_p, ")");
			}

			if(get_element_count_arraylist(&(dql->select_query.group_by)) > 0)
			{
				snprintf_dstring(str_p, " GROUP BY ");
				for(cy_uint i = 0; i < get_element_count_arraylist(&(dql->select_query.group_by)); i++)
				{
					if(i != 0)
						snprintf_dstring(str_p, ",");
					sql_expression* g = (sql_expression*) get_from_front_of_arraylist(&(dql->select_query.group_by), i);
					snprintf_dstring(str_p, "(");
					snprint_sql_expr(str_p, g);
					snprintf_dstring(str_p, ")");
				}
			}

			
			if(dql->select_query.having_expr)
			{
				snprintf_dstring(str_p, " HAVING (");
				snprint_sql_expr(str_p, dql->select_query.having_expr);
				snprintf_dstring(str_p, ")");
			}

			if(get_element_count_arraylist(&(dql->select_query.ordered_by)) > 0)
			{
				snprintf_dstring(str_p, " ORDER BY ");
				for(cy_uint i = 0; i < get_element_count_arraylist(&(dql->select_query.ordered_by)); i++)
				{
					if(i != 0)
						snprintf_dstring(str_p, ",");
					order_by* o = (order_by*) get_from_front_of_arraylist(&(dql->select_query.ordered_by), i);
					snprintf_dstring(str_p, "(");
					snprint_sql_expr(str_p, o->ordering_expr);
					snprintf_dstring(str_p, ") %s", ((o->dir == ORDER_BY_ASC) ? "ASC" : "DESC"));
				}
			}

			if(dql->select_query.offset_expr)
			{
				snprintf_dstring(str_p, " OFFSET (");
				snprint_sql_expr(str_p, dql->select_query.offset_expr);
				snprintf_dstring(str_p, ")");
			}

			if(dql->select_query.limit_expr)
			{
				snprintf_dstring(str_p, " LIMIT (");
				snprint_sql_expr(str_p, dql->select_query.limit_expr);
				snprintf_dstring(str_p, ")");
			}

			break;
		}
		case VALUES_QUERY :
		{
			snprintf_dstring(str_p, "VALUES ");
			for(cy_uint i = 0; i < get_element_count_arraylist(&(dql->values_query.values)); i++)
			{
				if(i != 0)
					snprintf_dstring(str_p, ",");
				const arraylist* row = get_from_front_of_arraylist(&(dql->values_query.values), i);
				snprintf_dstring(str_p, "(");
				for(cy_uint j = 0; j < get_element_count_arraylist(row); j++)
				{
					if(j != 0)
						snprintf_dstring(str_p, ",");
					const sql_expression* expr = get_from_front_of_arraylist(row, j);
					if(expr == NULL)
						snprintf_dstring(str_p, "DEFAULT");
					else
						snprint_sql_expr(str_p, expr);
				}
				snprintf_dstring(str_p, ")");
			}
			break;
		}
		case SET_OPERATION :
		{
			snprintf_dstring(str_p, "(");snprint_dql(str_p, dql->set_operation.left);snprintf_dstring(str_p, ")");
			switch(dql->set_operation.op_type)
			{
				case SQL_SET_INTERSECT :
				{
					snprintf_dstring(str_p, " INTERSECT");
					break;
				}
				case SQL_SET_UNION :
				{
					snprintf_dstring(str_p, " UNION");
					break;
				}
				case SQL_SET_EXCEPT :
				{
					snprintf_dstring(str_p, " EXCEPT");
					break;
				}
			}
			switch(dql->set_operation.op_mod)
			{
				case SQL_RESULT_SET_DISTINCT :
				{
					snprintf_dstring(str_p, " DISTINCT");
					break;
				}
				case SQL_RESULT_SET_ALL :
				{
					snprintf_dstring(str_p, " ALL");
					break;
				}
			}
			snprintf_dstring(str_p, " (");snprint_dql(str_p, dql->set_operation.right);snprintf_dstring(str_p, ")");
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
	if(dql == NULL)
		return;

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