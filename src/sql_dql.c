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
	dql->offset_expr = NULL;
	dql->limit_expr = NULL;

	return dql;
}

static void print_tabs(int tabs)
{
	while(tabs--)
		printf("\t");
}

static void print_relation_input(const relation_input* ri_p, int tabs)
{
	switch(ri_p->type)
	{
		case RELATION :
		{
			print_tabs(tabs);printf("( relation : ( ");
			printf_dstring(&(ri_p->relation_name));
			break;
		}
		case SUB_QUERY :
		{
			print_tabs(tabs);printf("( sub_query : ( \n");
			print_dql(ri_p->sub_query, tabs + 1);
			break;
		}
		case FUNCTION :
		{
			print_tabs(tabs);printf("( function_call : ( ");
			print_sql_expr(ri_p->function_call);
			break;
		}
	}
	printf(" ) AS ");
	printf_dstring(&(ri_p->as));
	printf(")");
}

void print_dql(const sql_dql* dql, int tabs)
{
	print_tabs(tabs);printf("SELECT : \n\n");

	print_tabs(tabs);printf("PROJECTION_LIST : \n");
	for(cy_uint i = 0; i < get_element_count_arraylist(&(dql->projections)); i++)
	{
		projection* p = (projection*) get_from_front_of_arraylist(&(dql->projections), i);
		print_tabs(tabs+1);printf("(");print_sql_expr(p->projection_expr);printf(" AS ");printf_dstring(&(p->as));printf(")");
		printf("\n");
	}
	printf("\n");

	print_tabs(tabs);printf("FROM : \n");
	print_relation_input(&(dql->base_input), tabs+1);
	printf("\n\n");

	print_tabs(tabs);printf("JOINS : \n");
	for(cy_uint i = 0; i < get_element_count_arraylist(&(dql->joins_with)); i++)
	{
		join_with* j = (join_with*) get_from_front_of_arraylist(&(dql->joins_with), i);
		print_tabs(tabs+1);
		switch(j->type)
		{
			case INNER_JOIN : 		printf(" INNER "); 		break;
			case LEFT_JOIN : 		printf(" LEFT "); 		break;
			case RIGHT_JOIN : 		printf(" RIGHT "); 		break;
			case FULL_JOIN : 		printf(" FULL "); 		break;
			case CROSS_JOIN : 		printf(" CROSS "); 		break;
		}
		printf(" is_lateral = %d ", j->is_lateral);
		print_relation_input(&(j->input), tabs+2);
		switch(j->condition_type)
		{
			case NO_JOIN_CONDITION :
			{
				printf(" NO CONDITION ");
				break;
			}
			case NATURAL_JOIN_CONDITION :
			{
				printf(" NATURAL CONDITION ");
				break;
			}
			case ON_EXPR_JOIN_CONDITION :
			{
				printf(" ON CONDITION ");
				print_sql_expr(j->on_expr);
				break;
			}
			case USING_JOIN_CONDITION :
			{
				printf(" USING ( ");
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
		printf("\n");
	}
	printf("\n");

	print_tabs(tabs);printf("WHERE : \n");
	print_tabs(tabs+1);
	if(dql->where_expr)
		print_sql_expr(dql->where_expr);
	else
		printf("NULL");
	printf("\n\n");

	print_tabs(tabs);printf("GROUP_BY : \n");
	for(cy_uint i = 0; i < get_element_count_arraylist(&(dql->group_by)); i++)
	{
		sql_expression* grouping_expr = (sql_expression*) get_from_front_of_arraylist(&(dql->joins_with), i);
		print_tabs(tabs+1);print_sql_expr(grouping_expr);printf("\n");
	}
	printf("\n");

	print_tabs(tabs);printf("HAVING : \n");
	print_tabs(tabs+1);
	if(dql->having_expr)
		print_sql_expr(dql->having_expr);
	else
		printf("NULL");
	printf("\n\n");

	print_tabs(tabs);printf("OFFSET : \n");
	print_tabs(tabs+1);
	if(dql->offset_expr)
		print_sql_expr(dql->offset_expr);
	else
		printf("NULL");
	printf("\n\n");

	print_tabs(tabs);printf("LIMIT : \n");
	print_tabs(tabs+1);
	if(dql->limit_expr)
		print_sql_expr(dql->limit_expr);
	else
		printf("NULL");
	printf("\n\n");
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
		sql_expression* grouping_expr = (sql_expression*) get_from_front_of_arraylist(&(dql->joins_with), i);
		delete_sql_expr(grouping_expr);
	}
	deinitialize_arraylist(&(dql->group_by));

	if(dql->having_expr)
		delete_sql_expr(dql->having_expr);

	if(dql->offset_expr)
		delete_sql_expr(dql->offset_expr);

	if(dql->limit_expr)
		delete_sql_expr(dql->limit_expr);

	free(dql);
}