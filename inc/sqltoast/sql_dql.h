#ifndef SQL_DQL_H
#define SQL_DQL_H

#include<sqltoast/sql_expression.h>

#include<cutlery/arraylist.h>
#include<cutlery/dstring.h>

typedef enum sql_dql_type sql_dql_type;
enum sql_dql_type
{
	SELECT_QUERY,
};

typedef struct sql_dql sql_dql;

typedef struct projection projection;
struct projection
{
	sql_expression* projection_expr;

	// if empty, ignore
	dstring as;
};

typedef enum relation_input_type relation_input_type;
enum relation_input_type
{
	RELATION,
	SUB_QUERY,
	FUNCTION,
};

typedef struct relation_input relation_input;
struct relation_input
{
	relation_input_type type;

	union
	{
		dstring relation_name;

		sql_dql* sub_query;

		sql_expression* function_call;
	};

	// if empty, ignore
	dstring as;
};

typedef enum join_type join_type;
enum join_type
{
	INNER_JOIN,
	LEFT_JOIN,
	RIGHT_JOIN,
	FULL_JOIN,
	CROSS_JOIN,
};

typedef struct join_with join_with;
struct join_with
{
	join_type type;

	relation_input input;

	sql_expression* on;
};

typedef enum order_by_dir order_by_dir;
enum order_by_dir
{
	ORDER_BY_ASC = 0,
	ORDER_BY_DESC = 1,
};

typedef struct order_by order_by;
struct order_by
{
	sql_expression* ordering_expr;

	order_by_dir dir;
};

typedef struct sql_dql sql_dql;
struct sql_dql
{
	sql_dql_type type;

	// result columns (each struct is result_alias)
	arraylist projections;

	// from clause
	relation_input base_input;

	// join tables (each strcut is join_with)
	arraylist joins_with;

	// where clause
	sql_expression* where_expr;

	// expressions to group_by (each struct is sql_expression)
	arraylist group_by;

	// having clause
	sql_expression* having_expr;

	// order by clauses (each struct is order_by)
	arraylist ordered_by;

	// offset clause
	sql_expression* offset_expr;

	// limit clause
	sql_expression* limit_expr;
};

sql_dql* new_dql();

#define new_relation_input(relation_name_, as_)                             ((relation_input){.type = RELATION, .relation_name = relation_name_, .as = as_})
#define new_sub_query_relation_input(sub_query_, as_)                       ((relation_input){.type = SUB_QUERY, .sub_query = sub_query_, .as = as_})
#define new_function_call_relation_input(function_call_, as_)               ((relation_input){.type = FUNCTION, .function_call = function_call_, .as = as_})

void print_dql(const sql_dql* dql, int tabs);

void delete_dql(sql_dql* dql);

#endif