#ifndef SQL_DML_H
#define SQL_DML_H

#include<sqltoast/sql_expression.h>

typedef enum sql_dml_type sql_dml_type;
enum sql_dml_type
{
	INSERT_QUERY,
	UPDATE_QUERY,
	DELETE_QUERY,
};

typedef struct sql_dml sql_dml;
struct sql_dml
{
	sql_dml_type type;

	// where clause
	sql_expression* where_expr;
};

void destroydml(sql_dml* dml);

#endif