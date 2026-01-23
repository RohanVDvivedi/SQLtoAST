#include<sqltoast/sql_ddl.h>

#include<stdlib.h>

void destroyddl(sql_ddl* ddl)
{
	free(ddl);
}