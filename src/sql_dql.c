#include<sqltoast/sql_dql.h>

#include<stdio.h>
#include<stdlib.h>

sql_dql* new_dql()
{
	sql_dql* dql = malloc(sizeof(sql_dql));

	dql->type = SELECT_QUERY;

	dql->base_input = new_relation_input(new_copy_dstring(&get_dstring_pointing_to_cstring("")), new_copy_dstring(&get_dstring_pointing_to_cstring("")));

	initialize_arraylist(&(dql->joins_with), 0);

	dql->where_expr = NULL;
	initialize_arraylist(&(dql->group_by), 0);
	dql->having_expr = NULL;
	initialize_arraylist(&(dql->ordered_by), 0);
	dql->offset_expr = NULL;
	dql->limit_expr = NULL;

	return dql;
}

static void print_relation_input(const relation_input* ri_p)
{
	switch(ri_p->type)
	{
		case RELATION :
		{
			printf("relation( ");
			printf_dstring(&(ri_p->relation_name));
			break;
		}
		case SUB_QUERY :
		{
			printf("sub_query( \n");
			print_dql(ri_p->sub_query);
			break;
		}
		case FUNCTION :
		{
			printf("function_call( ");
			print_sql_expr(ri_p->function_call);
			break;
		}
	}
	printf(" ) as ");
	if(is_empty_dstring(&(ri_p->as)))
		printf("no-alias");
	else
	{
		printf("\"");
		printf_dstring(&(ri_p->as));
		printf("\"");
	}
	printf(" )");
}

void print_dql(const sql_dql* dql)
{
	printf("select( ");

	if(get_element_count_arraylist(&(dql->projections)) > 0)
	{
		printf("projections( ");
		for(cy_uint i = 0; i < get_element_count_arraylist(&(dql->projections)); i++)
		{
			if(i != 0)
				printf(" , ");
			projection* p = (projection*) get_from_front_of_arraylist(&(dql->projections), i);
			printf("( ( ");
			print_sql_expr(p->projection_expr);
			printf(" ) as ");
			if(is_empty_dstring(&(p->as)))
				printf("no-alias");
			else
			{
				printf("\"");
				printf_dstring(&(p->as));
				printf("\"");
			}
			printf(" )");
		}
		printf(" )");
	}

	printf("from( ");
	print_relation_input(&(dql->base_input));
	printf(" )");

	if(get_element_count_arraylist(&(dql->joins_with)) > 0)
	{
		printf("joins( ");
		for(cy_uint i = 0; i < get_element_count_arraylist(&(dql->joins_with)); i++)
		{
			if(i != 0)
				printf(" , ");
			printf("( type(");
			join_with* j = (join_with*) get_from_front_of_arraylist(&(dql->joins_with), i);
			switch(j->type)
			{
				case INNER_JOIN : 		printf(" inner "); 		break;
				case LEFT_JOIN : 		printf(" left "); 		break;
				case RIGHT_JOIN : 		printf(" right "); 		break;
				case FULL_JOIN : 		printf(" full "); 		break;
				case CROSS_JOIN : 		printf(" cross "); 		break;
			}
			printf("), is_lateral(%d), with( ", j->is_lateral);
			print_relation_input(&(j->input));
			printf(" )");
			switch(j->condition_type)
			{
				case NO_JOIN_CONDITION :
					break;
				case NATURAL_JOIN_CONDITION :
				{
					printf(", condition(natural) ");
					break;
				}
				case ON_EXPR_JOIN_CONDITION :
				{
					printf(", on_condition( ");
					print_sql_expr(j->on_expr);
					printf(" )");
					break;
				}
				case USING_JOIN_CONDITION :
				{
					printf(", using_columns( ");
					for(cy_uint i = 0; i < get_element_count_arraylist(&(j->using_cols)); i++)
					{
						if(i > 0)
							printf(" , ");
						printf_dstring((dstring*) get_from_front_of_arraylist(&(j->using_cols), i));
					}
					printf(" )");
					break;
				}
			}
			printf(" )");
		}
		printf(" )");
	}

	if(dql->where_expr)
	{
		printf("where( ");
		print_sql_expr(dql->where_expr);
		printf(" )");
	}

	if(get_element_count_arraylist(&(dql->group_by)) > 0)
	{
		printf("group_by( ");
		for(cy_uint i = 0; i < get_element_count_arraylist(&(dql->group_by)); i++)
		{
			if(i != 0)
				printf(" , ");
			sql_expression* g = (sql_expression*) get_from_front_of_arraylist(&(dql->group_by), i);
			printf("( ");
			print_sql_expr(g);
			printf(" )");
		}
		printf(" )");
	}

	
	if(dql->having_expr)
	{
		printf("having( ");
		print_sql_expr(dql->having_expr);
		printf(" )");
	}

	if(get_element_count_arraylist(&(dql->ordered_by)) > 0)
	{
		printf("order_by( ");
		for(cy_uint i = 0; i < get_element_count_arraylist(&(dql->ordered_by)); i++)
		{
			if(i != 0)
				printf(" , ");
			order_by* o = (order_by*) get_from_front_of_arraylist(&(dql->ordered_by), i);
			printf("( ( ");
			print_sql_expr(o->ordering_expr);
			printf(" ) in %s order )", ((o->dir == ORDER_BY_ASC) ? "ascending" : "descending"));
		}
	}

	if(dql->offset_expr)
	{
		printf("offset( ");
		print_sql_expr(dql->offset_expr);
		printf(" )");
	}

	if(dql->limit_expr)
	{
		printf("limit( ");
		print_sql_expr(dql->limit_expr);
		printf(" )");
	}

	printf(" )");
}

static void destroy_relation_input(relation_input* ri_p)
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
		case FUNCTION :
		{
			delete_sql_expr(ri_p->function_call);
			break;
		}
	}
	deinit_dstring(&(ri_p->as));
}

void delete_dql(sql_dql* dql)
{
	for(cy_uint i = 0; i < get_element_count_arraylist(&(dql->projections)); i++)
	{
		projection* p = (projection*) get_from_front_of_arraylist(&(dql->projections), i);
		delete_sql_expr(p->projection_expr);
		deinit_dstring(&(p->as));
		free(p);
	}
	deinitialize_arraylist(&(dql->projections));

	destroy_relation_input(&(dql->base_input));

	for(cy_uint i = 0; i < get_element_count_arraylist(&(dql->joins_with)); i++)
	{
		join_with* j = (join_with*) get_from_front_of_arraylist(&(dql->joins_with), i);
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
	deinitialize_arraylist(&(dql->joins_with));

	if(dql->where_expr)
		delete_sql_expr(dql->where_expr);

	for(cy_uint i = 0; i < get_element_count_arraylist(&(dql->group_by)); i++)
	{
		sql_expression* grouping_expr = (sql_expression*) get_from_front_of_arraylist(&(dql->group_by), i);
		delete_sql_expr(grouping_expr);
	}
	deinitialize_arraylist(&(dql->group_by));

	if(dql->having_expr)
		delete_sql_expr(dql->having_expr);

	for(cy_uint i = 0; i < get_element_count_arraylist(&(dql->ordered_by)); i++)
	{
		order_by* o = (order_by*) get_from_front_of_arraylist(&(dql->ordered_by), i);
		delete_sql_expr(o->ordering_expr);
		free(o);
	}
	deinitialize_arraylist(&(dql->group_by));

	if(dql->offset_expr)
		delete_sql_expr(dql->offset_expr);

	if(dql->limit_expr)
		delete_sql_expr(dql->limit_expr);

	free(dql);
}