#include<sqltoast/sql_dql.h>

#include<sqltoast/arraylist_deleter.h>

#include<stdio.h>
#include<stdlib.h>

sql_dql* new_dql(sql_dql_type type)
{
	sql_dql* dql = malloc(sizeof(sql_dql));

	dql->with_recursive_ctes = 0;
	initialize_arraylist(&(dql->with_ctes), 0);

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
	if(get_element_count_arraylist(&(dql->with_ctes)) > 0)
	{
		snprintf_dstring(str_p, "WITH ");
		if(dql->with_recursive_ctes)
			snprintf_dstring(str_p, "RECURSIVE ");
		for(cy_uint i = 0; i < get_element_count_arraylist(&(dql->with_ctes)); i++)
		{
			if(i != 0)
				snprintf_dstring(str_p, ", ");

			const sql_cte* cte = get_from_front_of_arraylist(&(dql->with_ctes), i);
			snprint_cte(str_p, cte);
		}
		snprintf_dstring(str_p, " (");
	}

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
					const projection* p = get_from_front_of_arraylist(&(dql->select_query.projections), i);
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
					const sql_expression* g = get_from_front_of_arraylist(&(dql->select_query.group_by), i);
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
					const order_by* o = get_from_front_of_arraylist(&(dql->select_query.ordered_by), i);
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

	if(get_element_count_arraylist(&(dql->with_ctes)) > 0)
	{
		snprintf_dstring(str_p, ")");
	}
}

static int are_equal_relation_input(const relation_input* ri1_p, const relation_input* ri2_p)
{
	if(ri1_p->type != ri2_p->type)
		return 0;
	switch(ri1_p->type)
	{
		case RELATION :
		{
			if(0 != compare_dstring(&(ri1_p->relation_name), &(ri2_p->relation_name)))
				return 0;
			break;
		}
		case SUB_QUERY :
		{
			if(!are_equal_dql(ri1_p->sub_query, ri2_p->sub_query))
				return 0;
			break;
		}
		case FUNCTION_CALL :
		{
			if(!are_equal_sql_expr(ri1_p->function_call, ri2_p->function_call))
				return 0;
			break;
		}
	}
	if(0 != compare_dstring(&(ri1_p->as), &(ri2_p->as)))
		return 0;
	if(get_element_count_arraylist(&(ri1_p->columns_as)) != get_element_count_arraylist(&(ri2_p->columns_as)))
		return 0;
	for(cy_uint i = 0; i < get_element_count_arraylist(&(ri1_p->columns_as)); i++)
	{
		if(0 != compare_dstring(get_from_front_of_arraylist(&(ri1_p->columns_as), i), get_from_front_of_arraylist(&(ri2_p->columns_as), i)))
			return 0;
	}
	return 1;
}

int are_equal_dql(const sql_dql* dql1, const sql_dql* dql2)
{
	if(dql1 == dql2)
		return 1;
	if(dql1 == NULL || dql2 == NULL) // both NULL, is fine handled above
		return 0;

	{
		if(get_element_count_arraylist(&(dql1->with_ctes)) != get_element_count_arraylist(&(dql2->with_ctes)))
			return 0;
		if(get_element_count_arraylist(&(dql1->with_ctes)) >= 0 && dql1->with_recursive_ctes != dql2->with_recursive_ctes)
			return 0;
		for(cy_uint i = 0; i < get_element_count_arraylist(&(dql1->with_ctes)); i++)
			if(!are_equal_cte(get_from_front_of_arraylist(&(dql1->with_ctes), i), get_from_front_of_arraylist(&(dql2->with_ctes), i)))
				return 0;
	}

	if(dql1->type != dql2->type)
		return 0;
	switch(dql1->type)
	{
		case SELECT_QUERY :
		{
			if(dql1->select_query.projection_mode != dql2->select_query.projection_mode)
				return 0;

			{
				if(get_element_count_arraylist(&(dql1->select_query.projections)) != get_element_count_arraylist(&(dql2->select_query.projections)))
					return 0;
				for(cy_uint i = 0; i < get_element_count_arraylist(&(dql1->select_query.projections)); i++)
				{
					const projection* p1 = get_from_front_of_arraylist(&(dql1->select_query.projections), i);
					const projection* p2 = get_from_front_of_arraylist(&(dql2->select_query.projections), i);
					if(!are_equal_sql_expr(p1->projection_expr, p2->projection_expr))
						return 0;
					if(0 != compare_dstring(&(p1->as), &(p2->as)))
						return 0;
				}
			}

			{
				if(dql1->select_query.has_base_input != dql2->select_query.has_base_input)
					return 0;
				if(dql1->select_query.has_base_input)
				{
					if(!are_equal_relation_input(&(dql1->select_query.base_input), &(dql2->select_query.base_input)))
						return 0;
				}
			}

			{
				if(get_element_count_arraylist(&(dql1->select_query.joins_with)) != get_element_count_arraylist(&(dql2->select_query.joins_with)))
					return 0;
				for(cy_uint i = 0; i < get_element_count_arraylist(&(dql1->select_query.joins_with)); i++)
				{
					const join_with* j1 = get_from_front_of_arraylist(&(dql1->select_query.joins_with), i);
					const join_with* j2 = get_from_front_of_arraylist(&(dql2->select_query.joins_with), i);

					if(j1->condition_type != j2->condition_type)
						return 0;

					if(j1->is_lateral != j2->is_lateral)
						return 0;

					if(!are_equal_relation_input(&(j1->input), &(j2->input)))
						return 0;

					switch(j1->condition_type)
					{
						case NO_JOIN_CONDITION:
							break;
						case NATURAL_JOIN_CONDITION:
							break;
						case ON_EXPR_JOIN_CONDITION:
						{
							if(!are_equal_sql_expr(j1->on_expr, j2->on_expr))
								return 0;
							break;
						}
						case USING_JOIN_CONDITION:
						{
							if(get_element_count_arraylist(&(j1->using_cols)) != get_element_count_arraylist(&(j2->using_cols)))
								return 0;
							for(cy_uint k = 0; k < get_element_count_arraylist(&(j1->using_cols)); k++)
							{
								if(0 != compare_dstring(get_from_front_of_arraylist(&(j1->using_cols), k), get_from_front_of_arraylist(&(j2->using_cols), k)))
									return 0;
							}
							break;
						}
					}
				}
			}

			if(!are_equal_sql_expr(dql1->select_query.where_expr, dql2->select_query.where_expr))
				return 0;

			{
				if(get_element_count_arraylist(&(dql1->select_query.group_by)) != get_element_count_arraylist(&(dql2->select_query.group_by)))
					return 0;
				for(cy_uint i = 0; i < get_element_count_arraylist(&(dql1->select_query.group_by)); i++)
					if(!are_equal_sql_expr(get_from_front_of_arraylist(&(dql1->select_query.group_by), i), get_from_front_of_arraylist(&(dql2->select_query.group_by), i)))
						return 0;
			}

			if(!are_equal_sql_expr(dql1->select_query.having_expr, dql2->select_query.having_expr))
				return 0;

			{
				if(get_element_count_arraylist(&(dql1->select_query.ordered_by)) != get_element_count_arraylist(&(dql2->select_query.ordered_by)))
					return 0;
				for(cy_uint i = 0; i < get_element_count_arraylist(&(dql1->select_query.ordered_by)); i++)
				{
					const order_by* o1 = get_from_front_of_arraylist(&(dql1->select_query.ordered_by), i);
					const order_by* o2 = get_from_front_of_arraylist(&(dql2->select_query.ordered_by), i);
					if(o1->dir != o2->dir)
						return 0;
					if(!are_equal_sql_expr(o1->ordering_expr, o2->ordering_expr))
						return 0;
				}
			}

			if(!are_equal_sql_expr(dql1->select_query.offset_expr, dql2->select_query.offset_expr))
				return 0;

			if(!are_equal_sql_expr(dql1->select_query.limit_expr, dql2->select_query.limit_expr))
				return 0;

			break;
		}
		case VALUES_QUERY :
		{
			if(get_element_count_arraylist(&(dql1->values_query.values)) != get_element_count_arraylist(&(dql2->values_query.values)))
				return 0;
			for(cy_uint i = 0; i < get_element_count_arraylist(&(dql1->values_query.values)); i++)
			{
				const arraylist* row1 = get_from_front_of_arraylist(&(dql1->values_query.values), i);
				const arraylist* row2 = get_from_front_of_arraylist(&(dql2->values_query.values), i);
				if(get_element_count_arraylist(row1) != get_element_count_arraylist(row2))
					return 0;
				for(cy_uint j = 0; j < get_element_count_arraylist(row1); j++)
					if(!are_equal_sql_expr(get_from_front_of_arraylist(row1, j), get_from_front_of_arraylist(row2, j)))
						return 0;
			}
			break;
		}
		case SET_OPERATION :
		{
			if(!are_equal_dql(dql1->set_operation.left, dql2->set_operation.left))
				return 0;
			if(dql1->set_operation.op_type != dql2->set_operation.op_type)
				return 0;
			if(dql1->set_operation.op_mod != dql2->set_operation.op_mod)
				return 0;
			if(!are_equal_dql(dql1->set_operation.right, dql2->set_operation.right))
				return 0;
			break;
		}
	}

	return 1;
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

	delete_all_and_deinitialize_arraylist_1d(&(dql->with_ctes), (void(*)(void*))delete_cte);

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