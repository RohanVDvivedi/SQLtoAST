#ifndef SQL_DDL_H
#define SQL_DDL_H

#include<sqltoast/sql_expression.h>

typedef enum sql_ddl_type sql_ddl_type;
enum sql_ddl_type
{
	CREATE_QUERY,
	DROP_QUERY,
	ALTER_QUERY,
	TRUNCATE_QUERY,
};

typedef struct sql_ddl sql_ddl;
struct sql_ddl
{
	sql_ddl_type type;
};

void delete_ddl(sql_ddl* ddl);

#endif