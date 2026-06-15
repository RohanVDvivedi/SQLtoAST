#include<sqltoast/sql_dml.h>

#include<sqltoast/arraylist_deleter.h>

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

void snprint_dml(dstring* str_p, const sql_dml* dml)
{
	switch(dml->type)
	{
		case INSERT_QUERY :
		{
			snprintf_dstring(str_p, "INSERT INTO ");
			concatenate_dstring(str_p, &(dml->insert_query.table_name));

			if(get_element_count_arraylist(&(dml->insert_query.column_name_list)) > 0)
			{
				snprintf_dstring(str_p, "(");
				for(cy_uint i = 0; i < get_element_count_arraylist(&(dml->insert_query.column_name_list)); i++)
				{
					if(i != 0)
						snprintf_dstring(str_p, ",");
					concatenate_dstring(str_p, (dstring*) get_from_front_of_arraylist(&(dml->insert_query.column_name_list), i));
				}
				snprintf_dstring(str_p, ")");
			}

			if(dml->insert_query.input_data_query)
			{
				snprintf_dstring(str_p, " ");
				snprint_dql(str_p, dml->insert_query.input_data_query);
			}
			else
			{
				snprintf_dstring(str_p, " DEFAULT VALUES");
			}

			break;
		}
		case UPDATE_QUERY :
		{
			snprintf_dstring(str_p, "UPDATE ");
			concatenate_dstring(str_p, &(dml->update_query.table_name));

			{
				snprintf_dstring(str_p, " SET ");
				for(cy_uint i = 0; i < get_element_count_arraylist(&(dml->update_query.values_to_be_set)); i++)
				{
					if(i != 0)
						snprintf_dstring(str_p, ", ");

					columns_to_be_set* c = (columns_to_be_set*) get_from_front_of_arraylist(&(dml->update_query.values_to_be_set), i);

					snprintf_dstring(str_p, "(");
					for(cy_uint j = 0; j < get_element_count_arraylist(&(c->column_names)); j++)
					{
						if(j != 0)
							snprintf_dstring(str_p, ",");
						concatenate_dstring(str_p, (const dstring*) get_from_front_of_arraylist(&(c->column_names), j));
					}
					snprintf_dstring(str_p, ") = (");
					for(cy_uint j = 0; j < get_element_count_arraylist(&(c->value_exprs)); j++)
					{
						if(j != 0)
							snprintf_dstring(str_p, ",");
						const sql_expression* value_expr = get_from_front_of_arraylist(&(c->value_exprs), j);
						if(value_expr)
						{
							snprintf_dstring(str_p, "(");
							snprint_sql_expr(str_p, value_expr);
							snprintf_dstring(str_p, ")");
						}
						else
							snprintf_dstring(str_p, "DEFAULT");
					}
					snprintf_dstring(str_p, ")");
				}
			}

			if(dml->update_query.where_expr)
			{
				snprintf_dstring(str_p, " WHERE (");
				snprint_sql_expr(str_p, dml->update_query.where_expr);
				snprintf_dstring(str_p, ")");
			}

			break;
		}
		case DELETE_QUERY :
		{
			snprintf_dstring(str_p, "DELETE FROM ");
			concatenate_dstring(str_p, &(dml->delete_query.table_name));

			if(dml->delete_query.where_expr)
			{
				snprintf_dstring(str_p, " WHERE (");
				snprint_sql_expr(str_p, dml->delete_query.where_expr);
				snprintf_dstring(str_p, ")");
			}

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

void delete_columns_to_be_set(columns_to_be_set* c)
{
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

void delete_dstring(dstring* d)
{
	deinit_dstring(d);
	free(d);
}

void delete_dml(sql_dml* dml)
{
	if(dml == NULL)
		return;

	switch(dml->type)
	{
		case INSERT_QUERY :
		{
			deinit_dstring(&(dml->insert_query.table_name));

			delete_all_and_deinitialize_arraylist_1d(&(dml->insert_query.column_name_list), (void(*)(void*))delete_dstring);

			if(dml->insert_query.input_data_query)
				delete_dql(dml->insert_query.input_data_query);

			break;
		}
		case UPDATE_QUERY :
		{
			deinit_dstring(&(dml->update_query.table_name));

			delete_all_and_deinitialize_arraylist_1d(&(dml->update_query.values_to_be_set), (void(*)(void*))delete_columns_to_be_set);

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
