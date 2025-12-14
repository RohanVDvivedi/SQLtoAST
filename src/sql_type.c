#include<sqltoast/sql_type.h>

#include<cutlery/cutlery_stds.h>

#include<inttypes.h>
#include<stdio.h>

char const * const type_names[] = {
	[SQL_SMALLINT] = "SMALLINT", [SQL_INT] = "INT", [SQL_BIGINT] = "BIGINT",

	[SQL_REAL] = "REAL", [SQL_DECIMAL] = "DECIMAL", [SQL_NUMERIC] = "NUMERIC",

	[SQL_TEXT] = "TEXT", [SQL_CHAR] = "CHAR", [SQL_VARCHAR] = "VARCHAR", [SQL_CLOB] = "CLOB",

	[SQL_DATE] = "DATE", [SQL_TIME] = "TIME", [SQL_TIMESTAMP] = "TIMESTAMP",
};

sql_type new_sql_type(sql_type_name type_name)
{
	sql_type t;
	t.type_name = type_name;
	t.spec_size = 0;
	t.with_time_zone = 0;
	return t;
}

void print_sql_type(sql_type* t)
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