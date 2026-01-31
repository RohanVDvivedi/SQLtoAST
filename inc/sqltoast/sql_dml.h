#ifndef SQL_DML_H
#define SQL_DML_H

#include<sqltoast/sql_expression.h>
#include<sqltoast/sql_dql.h>

typedef enum sql_dml_type sql_dml_type;
enum sql_dml_type
{
	INSERT_QUERY,
	UPDATE_QUERY,
	DELETE_QUERY,
};

typedef struct sql_insert sql_insert;
struct sql_insert
{
	// into clause
	dstring table_name;

	arraylist column_name_list;

	// if NULL, check the values attribute
	sql_dql* input_data_query;

	// if input_data_query = NULL, then it instead has values expression

	// 2D pointer arraylist of expressions, element element of values is an arraylist, pointing to sql_expression pointer
	// if any expression is NULL, it is considered to be equivalent to DEFAULT in the sql statement
	arraylist values;
};

typedef struct columns_to_be_set columns_to_be_set;
struct columns_to_be_set
{
	// set columns with the value from value_expr or each one indivdually from the value_expr_list
	arraylist column_names;

	// if any of the the expressions is NULL, it is considered to be equivalent to DEFAULT in the sql statement
	arraylist value_exprs;
};

typedef struct sql_update sql_update;
struct sql_update
{
	// table to be updated
	dstring table_name;

	// each element in values_to_be_set is a pointer to the columns_to_be_set
	arraylist values_to_be_set;

	// where clause
	sql_expression* where_expr;
};

typedef struct sql_delete sql_delete;
struct sql_delete
{
	// from clause
	dstring table_name;

	// where clause
	sql_expression* where_expr;
};

typedef struct sql_dml sql_dml;
struct sql_dml
{
	sql_dml_type type;

	union
	{
		sql_insert insert_query;
		sql_update update_query;
		sql_delete delete_query;
	};
};

columns_to_be_set* new_columns_to_be_set();

sql_dml* new_dml(sql_dml_type type);

void print_dml(const sql_dml* dml);

void delete_dml(sql_dml* dml);

#endif