#include<sqltoast/sql_type.h>

#include<cutlery/cutlery_stds.h>

sql_type new_sql_type(sql_type_name type_name)
{
	sql_type t;
	t.type_name = type_name;
	memory_set(t.spec, -1, sizeof(t.spec));
	t.with_time_zone = 0;
	return t;
}