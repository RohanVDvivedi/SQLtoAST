#include<sqltoast/sql_dml.h>

#include<stdlib.h>

void delete_dml(sql_dml* dml)
{
	delete_sql_expr(dml->where_expr);

	free(dml);
}