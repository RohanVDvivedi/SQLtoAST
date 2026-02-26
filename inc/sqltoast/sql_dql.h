#ifndef SQL_DQL_H
#define SQL_DQL_H

typedef enum set_op_mod set_op_mod;
enum set_op_mod
{
	SQL_RESULT_SET_DISTINCT,
	SQL_RESULT_SET_ALL
};

#include<sqltoast/sql_expression.h>

#include<cutlery/arraylist.h>
#include<cutlery/dstring.h>

typedef struct sql_expression sql_expression;

typedef enum sql_dql_type sql_dql_type;
enum sql_dql_type
{
	SELECT_QUERY,
	VALUES_QUERY,
	SET_OPERATION
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
	FUNCTION_CALL,
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

	// if empty, ignore
	// valid only for type = SUB_QUERY or FUNCTION_CALL
	arraylist columns_as;
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

typedef enum join_condition_type join_condition_type;
enum join_condition_type
{
	NO_JOIN_CONDITION, // used only for CROSS_JOIN
	NATURAL_JOIN_CONDITION,
	ON_EXPR_JOIN_CONDITION, // on_expr is present
	USING_JOIN_CONDITION, // using_cols is present 
};

typedef struct join_with join_with;
struct join_with
{
	join_type type;

	int is_lateral;

	relation_input input;

	join_condition_type condition_type;

	union
	{
		sql_expression* on_expr;
		arraylist using_cols;
	};
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

typedef struct sql_select sql_select;
struct sql_select
{
	set_op_mod projection_mode;

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

typedef struct sql_values sql_values;
struct sql_values
{
	// 2D pointer arraylist of expressions, each element of values is an arraylist, pointing to sql_expression pointer
	// if any expression is NULL, it is considered to be equivalent to DEFAULT in the sql statement
	arraylist values;
};

typedef enum set_op_type set_op_type;
enum set_op_type
{
	SQL_SET_INTERSECT,
	SQL_SET_UNION,
	SQL_SET_EXCEPT,
};

typedef struct sql_dql sql_dql;
struct sql_dql
{
	sql_dql_type type;

	union
	{
		sql_select select_query;
		sql_values values_query;
		struct 
		{
			set_op_type op_type;

			set_op_mod op_mod;

			sql_dql* left;
			sql_dql* right;
		} set_operation;
	};
};

sql_dql* new_dql(sql_dql_type type);

#define init_relation_input(ri_p, relation_name_, as_)                             {ri_p->type = RELATION; ri_p->relation_name = relation_name_; ri_p->as = as_; initialize_arraylist(ri_p->columns_as, 0);}
#define init_sub_query_relation_input(ri_p, sub_query_, as_)                       {ri_p->type = SUB_QUERY; ri_p->sub_query = sub_query_; ri_p->as = as_; initialize_arraylist(ri_p->columns_as, 0);}
#define inti_function_call_relation_input(ri_p, function_call_, as_)               {ri_p->type = FUNCTION_CALL; ri_p->function_call = function_call_; ri_p->as = as_; initialize_arraylist(ri_p->columns_as, 0);}

void flatten_exprs_dql(sql_dql* dql);

void print_dql(const sql_dql* dql);

void delete_dql(sql_dql* dql);

#endif