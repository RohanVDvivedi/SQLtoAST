#ifndef SQL_DDL_H
#define SQL_DDL_H

#include<sqltoast/sql_dql.h>
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
	SQL_UNIQUE_KEY,
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

	// used for SQL_PRIMARY_KEY, SQL_UNIQUE_KEY and SQL_FORIEGN_KEY
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

#define init_table_element(te_p, _type) {                                            \
	(te_p)->type = _type;                                                            \
	switch(_type)                                                                    \
	{                                                                                \
		case SQL_COLUMN :                                                            \
		{                                                                            \
			init_empty_dstring(&((te_p)->column_def.column_name), 0);                \
			(te_p)->column_def.is_not_null = 0;                                      \
			(te_p)->column_def.is_primary_key = 0;                                   \
			(te_p)->column_def.is_unique = 0;                                        \
			(te_p)->column_def.is_foreign_key = 0;                                   \
			init_empty_dstring(&((te_p)->column_def.foreign_table), 0);              \
			init_empty_dstring(&((te_p)->column_def.foreign_column), 0);             \
			(te_p)->column_def.default_value = NULL;                                 \
			initialize_arraylist(&((te_p)->column_def.constraint_check_exprs), 0);   \
			break;                                                                   \
		}                                                                            \
		case SQL_CONSTRAINT :                                                        \
		{                                                                            \
			init_empty_dstring(&((te_p)->constraint_def.constraint_name), 0);        \
			initialize_arraylist(&((te_p)->constraint_def.column_list), 0);          \
			init_empty_dstring(&((te_p)->constraint_def.foreign_table), 0);          \
			initialize_arraylist(&((te_p)->constraint_def.foreign_column_list), 0);  \
			(te_p)->constraint_def.constraint_check_expr = NULL;                     \
			break;                                                                   \
		}                                                                            \
	}                                                                                \
}

typedef struct sql_create_table sql_create_table;
struct sql_create_table
{
	arraylist table_elements;
};

typedef enum sql_view_check_option sql_view_check_option;
enum sql_view_check_option
{
	CHECK_OPTION_NONE,

	// Ensures that INSERT or UPDATE performed through the view
	// produces rows that satisfy this view's WHERE condition.
	CHECK_OPTION_LOCAL,
	CHECK_OPTION_CASCADED // implicit is cascaded, if the check option does not have local or cascaded, specified
};

typedef struct sql_create_view sql_create_view;
struct sql_create_view
{
	// dstring containing the names of columns
	// if empty none was passed
	arraylist column_list;

	// initialized to NULL, but must be present in successfully parsed query
	sql_dql* view_query;

	sql_view_check_option check_option;
};

typedef order_by index_key_expr;

typedef struct sql_create_index sql_create_index;
struct sql_create_index
{
	int is_unique; // if it is a unique for the key_exprs

	dstring table_name; // on clause

	arraylist key_exprs; // array of index_key_expr structs, same as order_by struct in sql_dql.h

	arraylist include_expr; // additional columns that get stored as value for the index

	sql_expression* where_expr; // for partial indexing, index column only if the where_expr resolved into tre

	dstring using_index_type; // btree, hash, rtree, inverted
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
		sql_create_view create_view_query;
		sql_create_index create_index_query;
	};

	// only used if the query context required dropping, either directly as a drop query or as a part of alter table drop column
	sql_drop_behavior drop_behavior;
};

sql_ddl* new_ddl(sql_ddl_type type, sql_object_type object_type);

void print_ddl(const sql_ddl* ddl);

void delete_ddl(sql_ddl* ddl);

#endif