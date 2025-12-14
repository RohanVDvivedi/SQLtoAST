#include<sqltoast/sql_type.h>

sql_type new_sql_type(sql_type_name type_name)
{
	sql_type t;
	t.type_name = type_name;
	t.spec[0] = -1;
	t.spec[1] = -1;
	t.spec[2] = -1;
	t.spec[3] = -1;
	t.spec[4] = -1;
	t.with_time_zone = 0;
	return t;
}