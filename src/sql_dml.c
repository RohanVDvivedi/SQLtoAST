#include<sqltoast/sql_dml.h>

#include<stdlib.h>
#include<stdio.h>

columns_to_be_set* new_columns_to_be_set()
{
	columns_to_be_set* c = malloc(sizeof(columns_to_be_set));
	initialize_arraylist(&(c->column_names), 1);
	initialize_arraylist(&(c->value_exprs), 1);
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

			initialize_arraylist(&(dml->insert_query.values), 0);

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
			else if(get_element_count_arraylist(&(dml->insert_query.values)) > 0)
			{
				if(clauses_printed != 0)
					printf(" , ");
				printf("data( ");
				for(cy_uint i = 0; i < get_element_count_arraylist(&(dml->insert_query.values)); i++)
				{
					if(i != 0)
						printf(" , ");
					printf("( ");
					arraylist* row = (arraylist*) get_from_front_of_arraylist(&(dml->insert_query.values), i);
					for(cy_uint j = 0; j < get_element_count_arraylist(row); j++)
					{
						if(j != 0)
							printf(" , ");
						sql_expression* cell = (sql_expression*) get_from_front_of_arraylist(row, j);
						if(cell)
						{
							printf("( ");
							print_sql_expr(cell);
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
						dstring* column_name = (dstring*) get_from_front_of_arraylist(&(c->column_names), j);
						printf("\"");
						printf_dstring(column_name);
						printf("\"");
					}
					printf(" ) = ( ");
					for(cy_uint j = 0; j < get_element_count_arraylist(&(c->value_exprs)); j++)
					{
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

			for(cy_uint i = 0; i < get_element_count_arraylist(&(dml->insert_query.values)); i++)
			{
				arraylist* row = (arraylist*) get_from_front_of_arraylist(&(dml->insert_query.values), i);
				for(cy_uint j = 0; j < get_element_count_arraylist(row); j++)
				{
					sql_expression* cell = (sql_expression*) get_from_front_of_arraylist(row, j);
					if(cell)
						delete_sql_expr(cell);
				}
				deinitialize_arraylist(row);
				free(row);
			}
			deinitialize_arraylist(&(dml->insert_query.values));

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
				for(cy_uint j = 0; j < get_element_count_arraylist(&(c->value_exprs)); j++)
				{
					sql_expression* value_expr = (sql_expression*) get_from_front_of_arraylist(&(c->value_exprs), j);
					if(value_expr)
						delete_sql_expr(value_expr);
				}
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