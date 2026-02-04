#include<sqltoast/sql_dql.h>

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

			dql->select_query.base_input = new_relation_input(new_copy_dstring(&get_dstring_pointing_to_cstring("")), new_copy_dstring(&get_dstring_pointing_to_cstring("")));

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

static void print_relation_input(const relation_input* ri_p)
{
	switch(ri_p->type)
	{
		case RELATION :
		{
			printf("relation( ( ");
			printf("\"");
			printf_dstring(&(ri_p->relation_name));
			printf("\"");
			break;
		}
		case SUB_QUERY :
		{
			printf("sub_query( ( ");
			print_dql(ri_p->sub_query);
			break;
		}
		case FUNCTION :
		{
			printf("function_call( ( ");
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
	switch(dql->type)
	{
		case SELECT_QUERY :
		{
			printf("select( ");

			int clauses_printed = 0;

			if(get_element_count_arraylist(&(dql->select_query.projections)) > 0)
			{
				if(clauses_printed != 0)
					printf(" , ");
				printf("projections( ");
				if(dql->select_query.projection_mode == SQL_RESULT_SET_DISTINCT)
					printf("DISTINCT");
				for(cy_uint i = 0; i < get_element_count_arraylist(&(dql->select_query.projections)); i++)
				{
					if(i != 0)
						printf(" , ");
					projection* p = (projection*) get_from_front_of_arraylist(&(dql->select_query.projections), i);
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
				clauses_printed++;
			}

			{
				if(clauses_printed != 0)
					printf(" , ");
				printf("from( ");
				print_relation_input(&(dql->select_query.base_input));
				printf(" )");
				clauses_printed++;
			}

			if(get_element_count_arraylist(&(dql->select_query.joins_with)) > 0)
			{
				if(clauses_printed != 0)
					printf(" , ");
				printf("joins( ");
				for(cy_uint i = 0; i < get_element_count_arraylist(&(dql->select_query.joins_with)); i++)
				{
					if(i != 0)
						printf(" , ");
					printf("( type(");
					join_with* j = (join_with*) get_from_front_of_arraylist(&(dql->select_query.joins_with), i);
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
				clauses_printed++;
			}

			if(dql->select_query.where_expr)
			{
				if(clauses_printed != 0)
					printf(" , ");
				printf("where( ");
				print_sql_expr(dql->select_query.where_expr);
				printf(" )");
				clauses_printed++;
			}

			if(get_element_count_arraylist(&(dql->select_query.group_by)) > 0)
			{
				if(clauses_printed != 0)
					printf(" , ");
				printf("group_by( ");
				for(cy_uint i = 0; i < get_element_count_arraylist(&(dql->select_query.group_by)); i++)
				{
					if(i != 0)
						printf(" , ");
					sql_expression* g = (sql_expression*) get_from_front_of_arraylist(&(dql->select_query.group_by), i);
					printf("( ");
					print_sql_expr(g);
					printf(" )");
				}
				printf(" )");
				clauses_printed++;
			}

			
			if(dql->select_query.having_expr)
			{
				if(clauses_printed != 0)
					printf(" , ");
				printf("having( ");
				print_sql_expr(dql->select_query.having_expr);
				printf(" )");
				clauses_printed++;
			}

			if(get_element_count_arraylist(&(dql->select_query.ordered_by)) > 0)
			{
				if(clauses_printed != 0)
					printf(" , ");
				printf("order_by( ");
				for(cy_uint i = 0; i < get_element_count_arraylist(&(dql->select_query.ordered_by)); i++)
				{
					if(i != 0)
						printf(" , ");
					order_by* o = (order_by*) get_from_front_of_arraylist(&(dql->select_query.ordered_by), i);
					printf("( ( ");
					print_sql_expr(o->ordering_expr);
					printf(" ) in %s order )", ((o->dir == ORDER_BY_ASC) ? "ascending" : "descending"));
				}
				clauses_printed++;
			}

			if(dql->select_query.offset_expr)
			{
				if(clauses_printed != 0)
					printf(" , ");
				printf("offset( ");
				print_sql_expr(dql->select_query.offset_expr);
				printf(" )");
				clauses_printed++;
			}

			if(dql->select_query.limit_expr)
			{
				if(clauses_printed != 0)
					printf(" , ");
				printf("limit( ");
				print_sql_expr(dql->select_query.limit_expr);
				printf(" )");
				clauses_printed++;
			}

			printf(" )");
			break;
		}
		case VALUES_QUERY :
		{
			// TODO
			break;
		}
		case SET_OPERATION :
		{
			printf("( ");
			printf("( ");print_dql(dql->set_operation.left);printf(" )");
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
			printf("( ");print_dql(dql->set_operation.right);printf(" )");
			printf(" )");
			break;
		}
	}
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
	switch(dql->type)
	{
		case SELECT_QUERY :
		{
			for(cy_uint i = 0; i < get_element_count_arraylist(&(dql->select_query.projections)); i++)
			{
				projection* p = (projection*) get_from_front_of_arraylist(&(dql->select_query.projections), i);
				delete_sql_expr(p->projection_expr);
				deinit_dstring(&(p->as));
				free(p);
			}
			deinitialize_arraylist(&(dql->select_query.projections));

			destroy_relation_input(&(dql->select_query.base_input));

			for(cy_uint i = 0; i < get_element_count_arraylist(&(dql->select_query.joins_with)); i++)
			{
				join_with* j = (join_with*) get_from_front_of_arraylist(&(dql->select_query.joins_with), i);
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
			deinitialize_arraylist(&(dql->select_query.joins_with));

			if(dql->select_query.where_expr)
				delete_sql_expr(dql->select_query.where_expr);

			for(cy_uint i = 0; i < get_element_count_arraylist(&(dql->select_query.group_by)); i++)
			{
				sql_expression* grouping_expr = (sql_expression*) get_from_front_of_arraylist(&(dql->select_query.group_by), i);
				delete_sql_expr(grouping_expr);
			}
			deinitialize_arraylist(&(dql->select_query.group_by));

			if(dql->select_query.having_expr)
				delete_sql_expr(dql->select_query.having_expr);

			for(cy_uint i = 0; i < get_element_count_arraylist(&(dql->select_query.ordered_by)); i++)
			{
				order_by* o = (order_by*) get_from_front_of_arraylist(&(dql->select_query.ordered_by), i);
				delete_sql_expr(o->ordering_expr);
				free(o);
			}
			deinitialize_arraylist(&(dql->select_query.ordered_by));

			if(dql->select_query.offset_expr)
				delete_sql_expr(dql->select_query.offset_expr);

			if(dql->select_query.limit_expr)
				delete_sql_expr(dql->select_query.limit_expr);

			break;
		}
		case VALUES_QUERY :
		{
			// TODO
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