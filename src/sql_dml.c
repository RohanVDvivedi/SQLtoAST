#include<sqltoast/sql_dml.h>

#include<stdlib.h>
#include<stdio.h>

columns_to_be_set* new_columns_to_be_set(cy_uint capacity)
{
	columns_to_be_set* c = malloc(sizeof(columns_to_be_set));
	initialize_arraylist(&(c->column_names), capacity);
	initialize_arraylist(&(c->value_exprs), capacity);
	return c;
};

sql_dml* new_dml(sql_dml_type type)
{
	sql_dml* dml = malloc(sizeof(sql_dml));

	dml->type = type;

	switch(dml->type)
	{
		case INSERT_QUERY :
		{
			init_empty_dstring(&(dml->insert_query.table_name), 0);

			initialize_arraylist(&(dml->insert_query.column_name_list), 0);

			dml->insert_query.input_data_query = NULL;

			break;
		}
		case UPDATE_QUERY :
		{
			init_empty_dstring(&(dml->update_query.table_name), 0);

			initialize_arraylist(&(dml->update_query.values_to_be_set), 0);

			dml->update_query.where_expr = NULL;

			break;
		}
		case DELETE_QUERY :
		{
			init_empty_dstring(&(dml->delete_query.table_name), 0);

			dml->delete_query.where_expr = NULL;

			break;
		}
	}

	return dml;
}

void print_dml(const sql_dml* dml)
{
	switch(dml->type)
	{
		case INSERT_QUERY :
		{
			printf("insert( ");

			int clauses_printed = 0;

			{
				if(clauses_printed != 0)
					printf(" , ");
				printf("table( \"");
				printf_dstring(&(dml->insert_query.table_name));
				printf("\" )");
				clauses_printed++;
			}

			if(get_element_count_arraylist(&(dml->insert_query.column_name_list)) > 0)
			{
				if(clauses_printed != 0)
					printf(" , ");
				printf("columns( ");
				for(cy_uint i = 0; i < get_element_count_arraylist(&(dml->insert_query.column_name_list)); i++)
				{
					if(i != 0)
						printf(" , ");
					printf("\"");
					printf_dstring((dstring*) get_from_front_of_arraylist(&(dml->insert_query.column_name_list), i));
					printf("\"");
				}
				printf(" )");
				clauses_printed++;
			}

			if(dml->insert_query.input_data_query)
			{
				if(clauses_printed != 0)
					printf(" , ");
				printf("data( ");
				print_dql(dml->insert_query.input_data_query);
				printf(" )");
				clauses_printed++;
			}
			else
			{
				if(clauses_printed != 0)
					printf(" , ");
				printf("data( DEFAULT )");
				clauses_printed++;
			}

			printf(" )");

			break;
		}
		case UPDATE_QUERY :
		{
			printf("update( ");

			int clauses_printed = 0;

			{
				if(clauses_printed != 0)
					printf(" , ");
				printf("table( \"");
				printf_dstring(&(dml->update_query.table_name));
				printf("\" )");
				clauses_printed++;
			}

			{
				if(clauses_printed != 0)
					printf(" , ");
				printf("set( ");
				for(cy_uint i = 0; i < get_element_count_arraylist(&(dml->update_query.values_to_be_set)); i++)
				{
					if(i != 0)
						printf(" , ");

					columns_to_be_set* c = (columns_to_be_set*) get_from_front_of_arraylist(&(dml->update_query.values_to_be_set), i);

					printf("( ");
					for(cy_uint j = 0; j < get_element_count_arraylist(&(c->column_names)); j++)
					{
						if(j != 0)
							printf(" , ");
						dstring* column_name = (dstring*) get_from_front_of_arraylist(&(c->column_names), j);
						printf("\"");
						printf_dstring(column_name);
						printf("\"");
					}
					printf(" ) = ( ");
					for(cy_uint j = 0; j < get_element_count_arraylist(&(c->value_exprs)); j++)
					{
						if(j != 0)
							printf(" , ");
						sql_expression* value_expr = (sql_expression*) get_from_front_of_arraylist(&(c->value_exprs), j);
						if(value_expr)
						{
							printf("( ");
							print_sql_expr(value_expr);
							printf(" )");
						}
						else
							printf("DEFAULT");
					}
					printf(" )");
				}
				printf(" )");
				clauses_printed++;
			}

			if(dml->update_query.where_expr)
			{
				if(clauses_printed != 0)
					printf(" , ");
				printf("where( ");
				print_sql_expr(dml->update_query.where_expr);
				printf(" )");
				clauses_printed++;
			}

			printf(" )");

			break;
		}
		case DELETE_QUERY :
		{
			printf("delete( ");

			int clauses_printed = 0;

			{
				if(clauses_printed != 0)
					printf(" , ");
				printf("table( \"");
				printf_dstring(&(dml->delete_query.table_name));
				printf("\" )");
				clauses_printed++;
			}

			if(dml->delete_query.where_expr)
			{
				if(clauses_printed != 0)
					printf(" , ");
				printf("where( ");
				print_sql_expr(dml->delete_query.where_expr);
				printf(" )");
				clauses_printed++;
			}

			printf(" )");

			break;
		}
	}
}

void flatten_exprs_dml(sql_dml* dml)
{
	switch(dml->type)
	{
		case INSERT_QUERY :
		{
			if(dml->insert_query.input_data_query)
				flatten_exprs_dql(dml->insert_query.input_data_query);

			break;
		}
		case UPDATE_QUERY :
		{
			for(cy_uint i = 0; i < get_element_count_arraylist(&(dml->update_query.values_to_be_set)); i++)
			{
				columns_to_be_set* c = (columns_to_be_set*) get_from_front_of_arraylist(&(dml->update_query.values_to_be_set), i);
				for(cy_uint j = 0; j < get_element_count_arraylist(&(c->value_exprs)); j++)
				{
					sql_expression* value_expr = (sql_expression*) get_from_front_of_arraylist(&(c->value_exprs), j);
					if(value_expr)
					{
						value_expr = flatten_similar_associative_operators_in_sql_expression(value_expr);
						set_from_front_in_arraylist(&(c->value_exprs), value_expr, j);
					}
				}
			}

			if(dml->update_query.where_expr)
				dml->update_query.where_expr = flatten_similar_associative_operators_in_sql_expression(dml->update_query.where_expr);

			break;
		}
		case DELETE_QUERY :
		{
			if(dml->delete_query.where_expr)
				dml->delete_query.where_expr = flatten_similar_associative_operators_in_sql_expression(dml->delete_query.where_expr);

			break;
		}
	}
}

void delete_dml(sql_dml* dml)
{
	switch(dml->type)
	{
		case INSERT_QUERY :
		{
			deinit_dstring(&(dml->insert_query.table_name));

			for(cy_uint i = 0; i < get_element_count_arraylist(&(dml->insert_query.column_name_list)); i++)
			{
				dstring* col_name = (dstring*) get_from_front_of_arraylist(&(dml->insert_query.column_name_list), i);
				deinit_dstring(col_name);
				free(col_name);
			}
			deinitialize_arraylist(&(dml->insert_query.column_name_list));

			if(dml->insert_query.input_data_query)
				delete_dql(dml->insert_query.input_data_query);

			break;
		}
		case UPDATE_QUERY :
		{
			deinit_dstring(&(dml->update_query.table_name));

			for(cy_uint i = 0; i < get_element_count_arraylist(&(dml->update_query.values_to_be_set)); i++)
			{
				columns_to_be_set* c = (columns_to_be_set*) get_from_front_of_arraylist(&(dml->update_query.values_to_be_set), i);
				for(cy_uint j = 0; j < get_element_count_arraylist(&(c->column_names)); j++)
				{
					dstring* column_name = (dstring*) get_from_front_of_arraylist(&(c->column_names), j);
					deinit_dstring(column_name);
					free(column_name);
				}
				deinitialize_arraylist(&(c->column_names));
				for(cy_uint j = 0; j < get_element_count_arraylist(&(c->value_exprs)); j++)
				{
					sql_expression* value_expr = (sql_expression*) get_from_front_of_arraylist(&(c->value_exprs), j);
					if(value_expr)
						delete_sql_expr(value_expr);
				}
				deinitialize_arraylist(&(c->value_exprs));
				free(c);
			}
			deinitialize_arraylist(&(dml->update_query.values_to_be_set));

			if(dml->update_query.where_expr)
				delete_sql_expr(dml->update_query.where_expr);

			break;
		}
		case DELETE_QUERY :
		{
			deinit_dstring(&(dml->delete_query.table_name));

			if(dml->delete_query.where_expr)
				delete_sql_expr(dml->delete_query.where_expr);

			break;
		}
	}

	free(dml);
}