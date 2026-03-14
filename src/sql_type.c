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

	[SQL_TEXT] = "TEXT", [SQL_CHAR] = "CHAR", [SQL_VARCHAR] = "VARCHAR", [SQL_CLOB] = "CLOB", [SQL_BLOB] = "BLOB",

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
	return t;
}

void print_sql_type(const sql_type* t)
{
	if(t->type_name != SQL_CUSTOM_TYPE)
	{
		printf("%s", type_names[t->type_name]);
		if(t->spec_size > 0)
		{
			printf("(");
			for(int i = 0; i < t->spec_size; i++)
			{
				if(i > 0)
					printf(",");
				printf("%"PRId64, t->spec[i]);
			}
			printf(")");
		}
		if(t->type_name == SQL_TIME || t->type_name == SQL_TIMESTAMP)
		{
			if(t->with_time_zone)
				printf(" WITH TIME ZONE");
			else
				printf(" WITHOUT TIME ZONE");
		}
	}
	else
		printf_dstring(&(t->custom_type_name));
}

void delete_sql_type(sql_type* t)
{
	if(t->type_name == SQL_CUSTOM_TYPE)
		deinit_dstring(&(t->custom_type_name));
	free(t);
}