#ifndef SQL_TYPE_H
#define SQL_TYPE_H

#include<stdint.h>

typedef enum sql_type_name sql_type_name;
enum sql_type_name
{
	SQL_SMALLINT, SQL_INT, SQL_BIGINT,

	SQL_REAL, SQL_DECIMAL, SQL_NUMERIC,

	SQL_TEXT, SQL_CHAR, SQL_VARCHAR, SQL_CLOB,

	SQL_DATE, SQL_TIME, SQL_TIMESTAMP,
};

extern char const * const type_names[];

typedef struct sql_type sql_type;
struct sql_type
{
	sql_type_name type_name;

	// for length of char and varchar types
	// for precision and scale of real, decimal and numeric
	int64_t spec[8]; // 8 here is definitely an overkill
	uint8_t spec_size;

	// valid only for time and timestamp
	int with_time_zone:1;
};

sql_type new_sql_type(sql_type_name type_name);

void print_sql_type(sql_type* t);

#endif