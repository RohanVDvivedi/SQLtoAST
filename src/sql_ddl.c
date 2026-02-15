#include<sqltoast/sql_ddl.h>

#include<stdlib.h>

sql_ddl* new_ddl(sql_ddl_type type, sql_object_type object_type)
{
	sql_ddl* ddl = malloc(sizeof(sql_ddl));

	// every ddl query has an object name
	init_empty_dstring(&(ddl->object_name), 0);

	switch(type)
	{
		case CREATE_QUERY :
		{
			break;
		}
		case ALTER_QUERY :
		{
			break;
		}
		case DROP_QUERY :
		{
			break;
		}
		case TRUNCATE_QUERY :
		{
			break;
		}
	}

	return ddl;
}

void delete_ddl(sql_ddl* ddl)
{
	deinit_dstring(&(ddl->object_name));
	free(ddl);
}