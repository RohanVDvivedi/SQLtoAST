#include<sqltoast/sql_cte.h>

#include<sqltoast/sql_dql.h>
#include<sqltoast/sql_dml.h>

#include<sqltoast/arraylist_deleter.h>

#include<stdlib.h>

sql_cte* new_cte(sql_cte_type cte_type)
{
	sql_cte* cte = malloc(sizeof(sql_cte));

	init_empty_dstring(&(cte->cte_name), 0);

	initialize_arraylist(&(cte->cte_column_names), 0);

	cte->cte_type = cte_type;
	cte->dql_query = NULL;
	cte->dml_query = NULL;

	return cte;
}

void snprint_cte(dstring* str_p, const sql_cte* cte)
{
	concatenate_dstring(str_p, &(cte->cte_name));

	if(get_element_count_arraylist(&(cte->cte_column_names)) > 0)
	{
		snprintf_dstring(str_p, "(");
		for(cy_uint i = 0; i < get_element_count_arraylist(&(cte->cte_column_names)); i++)
		{
			if(i != 0)
				snprintf_dstring(str_p, ",");
			concatenate_dstring(str_p, (dstring*) get_from_front_of_arraylist(&(cte->cte_column_names), i));
		}
		snprintf_dstring(str_p, ")");
	}

	snprintf_dstring(str_p, " AS (");

	switch(cte->cte_type)
	{
		case CTE_DQL :
		{
			snprint_dql(str_p, cte->dql_query);
			break;
		}
		case CTE_DML :
		{
			snprint_dml(str_p, cte->dml_query);
			break;
		}
	}

	snprintf_dstring(str_p, ")");
}

void delete_dstring(dstring* d);

void delete_cte(sql_cte* cte)
{
	deinit_dstring(&(cte->cte_name));

	delete_all_and_deinitialize_arraylist_1d(&(cte->cte_column_names), (void(*)(void*))delete_dstring);

	switch(cte->cte_type)
	{
		case CTE_DQL :
		{
			if(cte->dql_query)
				delete_dql(cte->dql_query);
			break;
		}
		case CTE_DML :
		{
			if(cte->dml_query)
				delete_dml(cte->dml_query);
			break;
		}
	}

	free(cte);
}