#ifndef SQL_DQL_H
#define SQL_DQL_H

#include<sqltoast/sql_expression.h>

typedef enum sql_dql_type sql_dql_type;
enum sql_dql_type
{
	SELECT_QUERY,
};

typedef struct sql_dql sql_dql;
struct sql_dql
{
	sql_dql_type type;

	// where clause
	sql_expression* where_expr;
};

void destroydql(sql_dql* dql);

#endif