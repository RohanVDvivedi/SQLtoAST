#include<sqltoast/sql_dml.h>

void destroydml(sql_dml* dml)
{
	delete_sql_expr(dml->where_expr);
}