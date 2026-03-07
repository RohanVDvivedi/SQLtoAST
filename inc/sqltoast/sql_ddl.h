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
	SQL_CATALOG,
	SQL_DATABASE = SQL_CATALOG, // same as SQL_CATALOG
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
};

// hierarchially 
// catalog -> schema -> tables/views -> indices

typedef struct sql_create_schema sql_create_schema;
struct sql_create_schema
{
	dstring authorization; // if empty, then not set
};

typedef enum sql_table_element_type sql_table_element_type;
enum sql_table_element_type
{
	SQL_COLUMN,
	SQL_CONSTRAINT,
};

typedef struct sql_column_def sql_column_def;
struct sql_column_def
{
	dstring column_name;

	sql_type type;

	int is_not_null:1;
	int is_primary_key:1;
	int is_unique:1;
	int is_foreign_key:1;
	dstring foreign_table;
	dstring foreign_column; // if absent, you are bound to reference the primary key of the foreign_table
	// TODO: add on delete/update clauses
	sql_expression* default_value;
	arraylist constraint_check_exprs;
};

typedef enum sql_constraint_type sql_constraint_type;
enum sql_constraint_type
{
	SQL_UNIQUE,
	SQL_PRIMARY_KEY,
	SQL_FOREIGN_KEY,
	SQL_CONSTRAINT_CHECK,
};

typedef struct sql_constraint_def sql_constraint_def;
struct sql_constraint_def
{
	sql_constraint_type type;

	// every constarint has a constraint name
	dstring constraint_name;

	// used for SQL_PRIMARY_KEY, SQL_UNIQUE and SQL_FORIEGN_KEY
	arraylist column_list;

	// used for SQL_FORIEGN_KEY
	dstring foreign_table;
	arraylist foreign_column_list;

	// used for SQL_CONSTRAINT_CHECK
	sql_expression* constraint_check_expr;
};

typedef struct sql_table_element sql_table_element;
struct sql_table_element
{
	sql_table_element_type type;

	union
	{
		sql_column_def column_def;
		sql_constraint_def constraint_def;
	};
};

#define init_table_element(te_p, _type) {                                          \
	te_p->type = _type;                                                            \
	switch(_type)                                                                  \
	{                                                                              \
		case SQL_COLUMN :                                                          \
		{                                                                          \
			init_empty_dstring(&(te_p->column_def.column_name), 0);                \
			init_empty_dstring(&(te_p->column_def.foreign_table), 0);              \
			init_empty_dstring(&(te_p->column_def.foreign_column), 0);             \
			te_p->column_def.default_value = NULL;                                 \
			initialize_arraylist(&(te_p->column_def.constraint_check_exprs), 0);   \
			break;                                                                 \
		}                                                                          \
		case SQL_CONSTRAINT :                                                      \
		{                                                                          \
			init_empty_dstring(&(te_p->constraint_def.constraint_name), 0);        \
			initialize_arraylist(&(te_p->constraint_def.column_list), 0);          \
			init_empty_dstring(&(te_p->constraint_def.foreign_table), 0);          \
			initialize_arraylist(&(te_p->constraint_def.foreign_column_list), 0);  \
			te_p->constraint_def.constraint_check_expr = NULL;                     \
			break;                                                                 \
		}                                                                          \
	}                                                                              \
}

typedef struct sql_create_table sql_create_table;
struct sql_create_table
{
	arraylist table_elements;
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

	union
	{
		sql_create_schema create_schema_query;
		sql_create_table create_table_query;
	};

	// only used if the query context required dropping, either directly as a drop query or as a part of alter table drop column
	sql_drop_behavior drop_behavior;
};

sql_ddl* new_ddl(sql_ddl_type type, sql_object_type object_type);

void print_ddl(const sql_ddl* ddl);

void delete_ddl(sql_ddl* ddl);

#endif