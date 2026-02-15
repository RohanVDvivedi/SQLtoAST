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

typedef enum sql_object_type sql_object_type;
enum sql_object_type
{
	SQL_DATABASE,
	SQL_SCHEMA,
	SQL_TABLE,
	SQL_VIEW,
	SQL_INDEX,
	SQL_FUNCTION,
	SQL_PROCEDURE,
	SQL_TYPE,
	SQL_DOMAIN,
	SQL_SEQUENCE,
	SQL_TRIGGER,
	SQL_ASSERTION,
};

typedef enum sql_drop_behavior sql_drop_behavior;
enum sql_drop_behavior
{
	DROP_RESTRICT,
	DROP_CASCADE,
};

typedef struct sql_ddl sql_ddl;
struct sql_ddl
{
	sql_ddl_type type;

	sql_object_type object_type;

	dstring object_name;

	// only used if the query context required dropping, ither directly as a drop query or as a part of alter table
	sql_drop_behavior drop_behavior;
};

sql_ddl* new_ddl(sql_ddl_type type, sql_object_type object_type);

void print_ddl(const sql_ddl* ddl);

void delete_ddl(sql_ddl* ddl);

#endif