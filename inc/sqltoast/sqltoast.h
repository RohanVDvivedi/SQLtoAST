#ifndef SQLTOAST_H
#define SQLTOAST_H

#include<cutlery/stream.h>

#include<sqltoast/sql_dql.h>
#include<sqltoast/sql_dml.h>
#include<sqltoast/sql_ddl.h>
#include<sqltoast/sql_tcl.h>

typedef enum sql_query_type sql_query_type;
enum sql_query_type
{
	DQL,
	DML,
	DDL,
	TCL,
};

typedef struct sql sql;
struct sql
{
	sql_query_type type;

	union
	{
		sql_dql* dql_query;
		sql_dml* dml_query;
		sql_ddl* ddl_query;
		sql_tcl* tcl_cmd;
	};
};

sql* parse_sql(stream* strm, int* error);

void print_sql(const sql* sqlast);

void delete_sql(sql* sqlast);

#endif