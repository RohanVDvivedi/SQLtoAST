#include<sqltoast/sql_ddl.h>

#include<stdlib.h>

void delete_ddl(sql_ddl* ddl)
{
	free(ddl);
}