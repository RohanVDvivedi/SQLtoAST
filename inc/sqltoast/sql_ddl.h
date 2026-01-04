#ifndef SQL_DQL_H
#define SQL_DQL_H

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

#endif