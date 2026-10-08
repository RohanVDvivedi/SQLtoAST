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
			concatenate_dstring(str_p, (const dstring*) get_from_front_of_arraylist(&(cte->cte_column_names), i));
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

int are_equal_cte(const sql_cte* cte1, const sql_cte* cte2)
{
	if(cte1 == cte2)
		return 1;
	if(cte1 == NULL || cte2 == NULL)
		return 0;

	if(!compare_dstring(&(cte1->cte_name), &(cte2->cte_name)))
		return 0;
	if(get_element_count_arraylist(&(cte1->cte_column_names)) != get_element_count_arraylist(&(cte2->cte_column_names)))
		return 0;
	for(cy_uint i = 0; i < get_element_count_arraylist(&(cte1->cte_column_names)); i++)
		if(!compare_dstring(get_from_front_of_arraylist(&(cte1->cte_column_names), i), get_from_front_of_arraylist(&(cte2->cte_column_names), i)))
			return 0;
	if(cte1->cte_type != cte2->cte_type)
		return 0;
	switch(cte1->cte_type)
	{
		case CTE_DQL :
		{
			if(!are_equal_dql(cte1->dql_query, cte2->dql_query))
				return 0;
			break;
		}
		case CTE_DML :
		{
			if(!are_equal_dml(cte1->dml_query, cte2->dml_query))
				return 0;
			break;
		}
	}
	return 1;
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