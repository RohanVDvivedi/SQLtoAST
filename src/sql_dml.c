#include<sqltoast/sql_dml.h>

#include<stdlib.h>

void delete_dml(sql_dml* dml)
{
	switch(dml->type)
	{
		case INSERT_QUERY :
		{
			deinit_dstring(&(dml->insert_query.table_name));

			for(cy_uint i = 0; i < get_element_count_arraylist(&(dml->insert_query.column_name_list)); i++)
				deinit_dstring((dstring*) get_from_front_of_arraylist(&(dml->insert_query.column_name_list), i));
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
			}
			deinitialize_arraylist(&(dml->insert_query.values));

			break;
		}
		case UPDATE_QUERY :
		{
			deinit_dstring(&(dml->update_query.table_name));

			for(cy_uint i = 0; i < get_element_count_arraylist(&(dml->update_query.values_to_be_set)); i++)
			{
				column_to_be_set* c = (column_to_be_set*) get_from_front_of_arraylist(&(dml->update_query.values_to_be_set), i);
				if(c->value_expr)
					delete_sql_expr(c->value_expr);
				deinit_dstring(&(c->column_name));
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