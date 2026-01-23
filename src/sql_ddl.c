#include<sqltoast/sql_ddl.h>

void destroyddl(sql_ddl* ddl)
{
	free(ddl);
}