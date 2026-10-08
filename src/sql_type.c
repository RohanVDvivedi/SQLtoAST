#include<sqltoast/sql_type.h>

#include<cutlery/cutlery_stds.h>

#include<inttypes.h>
#include<stdio.h>
#include<stdlib.h>

char const * const type_names[] = {
	[SQL_BOOL] = "BOOL",

	[SQL_BIT] = "BIT",

	[SQL_SMALLINT] = "SMALLINT", [SQL_INT] = "INT", [SQL_BIGINT] = "BIGINT",

	[SQL_REAL] = "REAL", [SQL_DOUBLE] = "DOUBLE", [SQL_FLOAT] = "FLOAT",

	[SQL_DECIMAL] = "DECIMAL", [SQL_NUMERIC] = "NUMERIC",

	[SQL_TEXT] = "TEXT", [SQL_CHAR] = "CHAR", [SQL_VARCHAR] = "VARCHAR", [SQL_STRING] = "STRING",

	[SQL_CLOB] = "CLOB", [SQL_BLOB] = "BLOB", [SQL_BINARY] = "BINARY",

	[SQL_DATE] = "DATE", [SQL_TIME] = "TIME", [SQL_TIMESTAMP] = "TIMESTAMP",

	[SQL_CUSTOM_TYPE] = "",
};

sql_type* new_sql_type(sql_type_name type_name)
{
	sql_type* t = malloc(sizeof(sql_type));
	t->type_name = type_name;
	if(type_name != SQL_CUSTOM_TYPE)
	{
		t->spec_size = 0;
		t->with_time_zone = 0;
	}
	else
	{
		init_empty_dstring(&(t->custom_type_name), 0);
	}

	t->for_array = 0;
	t->array_dims_size = 0;

	return t;
}

void snprint_sql_type(dstring* str_p, const sql_type* t)
{
	if(t->type_name != SQL_CUSTOM_TYPE)
	{
		snprintf_dstring(str_p, "%s", type_names[t->type_name]);
		if(t->spec_size > 0)
		{
			snprintf_dstring(str_p, "(");
			for(int i = 0; i < t->spec_size; i++)
			{
				if(i > 0)
					snprintf_dstring(str_p, ",");
				snprintf_dstring(str_p, "%"PRId64, t->spec[i]);
			}
			snprintf_dstring(str_p, ")");
		}
		if(t->type_name == SQL_TIME || t->type_name == SQL_TIMESTAMP)
		{
			if(t->with_time_zone)
				snprintf_dstring(str_p, " WITH TIME ZONE");
			else
				snprintf_dstring(str_p, " WITHOUT TIME ZONE");
		}
	}
	else
		concatenate_dstring(str_p, &(t->custom_type_name));

	if(t->for_array)
	{
		snprintf_dstring(str_p, " ARRAY");
		for(int i = 0; i < t->array_dims_size; i++)
		{
			if(t->array_dims[i] == -1)
				snprintf_dstring(str_p, "[]");
			else
				snprintf_dstring(str_p, "[%"PRId64"]", t->array_dims[i]);
		}
	}
}

int are_equal_sql_type(const sql_type* t1, const sql_type* t2)
{
	if(t1 == t2)
		return 1;
	if(t1 == NULL || t2 == NULL)
		return 0;

	if(t1->type_name != t2->type_name)
		return 0;

	if(t1->type_name != SQL_CUSTOM_TYPE)
	{
		if(t1->spec_size != t2->spec_size)
			return 0;
		for(int i = 0; i < t1->spec_size; i++)
			if(t1->spec[i] != t2->spec[i])
				return 0;
		if(t1->type_name == SQL_TIME || t1->type_name == SQL_TIMESTAMP)
			if(t1->with_time_zone != t2->with_time_zone)
				return 0;
	}
	else
	{
		if(0 != compare_dstring(&(t1->custom_type_name), &(t2->custom_type_name)))
			return 0;
	}

	if(t1->for_array != t2->for_array)
		return 0;
	if(t1->for_array)
	{
		if(t1->array_dims_size != t2->array_dims_size)
			return 0;
		for(int i = 0; i < t1->array_dims_size; i++)
			if(t1->array_dims[i] != t2->array_dims[i])
				return 0;
	}

	return 1;
}

void delete_sql_type(sql_type* t)
{
	if(t == NULL)
		return;

	if(t->type_name == SQL_CUSTOM_TYPE)
		deinit_dstring(&(t->custom_type_name));
	free(t);
}