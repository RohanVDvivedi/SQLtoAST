#ifndef SQLTOAST_H
#define SQLTOAST_H

#include<cutlery/stream.h>

#include<sqltoast/sql_expression.h>

typedef struct sql sql;
struct sql
{
	int ival;
};

sql* parsesql(stream* strm, int* error);

#endif