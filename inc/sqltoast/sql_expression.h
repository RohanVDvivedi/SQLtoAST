#ifndef SQL_EXPRESSION_H
#define SQL_EXPRESSION_H

#include<cutlery/dstring.h>
#include<cutlery/arraylist.h>

#include<sqltoast/sql_type.h>

#include<sqltoast/sql_dql.h>

typedef struct sql_dql sql_dql;

typedef enum sql_expression_type sql_expression_type;
enum sql_expression_type
{
	SQL_NEG,

	SQL_BITNOT,

	SQL_LOGNOT,

	// all operators above are unary operators

	SQL_ADD,	SQL_ADD_FLAT,
	SQL_SUB,
	SQL_MUL,	SQL_MUL_FLAT,
	SQL_DIV,
	SQL_MOD,

	SQL_GT,
	SQL_GTE,
	SQL_LT,
	SQL_LTE,
	SQL_EQ,
	SQL_NEQ, // relational operators, always binary

	SQL_BITAND,
	SQL_BITOR,
	SQL_BITXOR, // bitwise operators

	SQL_LOGAND,		SQL_LOGAND_FLAT,
	SQL_LOGOR,		SQL_LOGOR_FLAT,
	SQL_LOGXOR,		SQL_LOGXOR_FLAT, // logical operators

	SQL_LSHIFT,
	SQL_RSHIFT, // left shift and right shift on integers

	SQL_CONCAT, SQL_CONCAT_FLAT,// concatenate strings

	SQL_LIKE,

	SQL_IS,

	// all operators above are binary operators (except their flat counter parts)

	SQL_BTWN, // between operator in sql

	// all operators above are operators that take bounds as input

	SQL_IN,

	// all operators above take expression list as input

	SQL_NUM,
	SQL_STR,

	// reflects a constant

	SQL_VAR,

	// reflects a variable, or a table accessible value

	SQL_TRUE,
	SQL_FALSE,
	// boolean sql tree nodes

	SQL_UNKNOWN,
	// unknown node

	SQL_NULL,
	// null node

	SQL_FUNCTION,

	SQL_CAST,

	SQL_SUB_QUERY,
};

typedef enum sql_cmp_quantifier sql_cmp_quantifier;
enum sql_cmp_quantifier
{
	SQL_CMP_NONE,
	SQL_CMP_ANY,
	SQL_CMP_ALL,
};

/*
	int the above enum, please make sure that the next of the binary is always possibly it's flattenned version
*/

typedef struct sql_expression sql_expression;
struct sql_expression
{
	sql_expression_type type;

	// only valid if the type is in (SQL_GT, SQL_GTE, SQL_LT, SQL_LTE, SQL_EQ, SQL_NEQ)
	// this attribute makes sense only if the right is an operator of sub_query type
	sql_cmp_quantifier cmp_rhs_quantfier;

	union
	{
		// for all unary operators
		sql_expression* unary_of;

		// for all binary operators
		struct
		{
			sql_expression* left;
			sql_expression* right; // if the right side is sub_query with more than 1 result, then use the struct below to resolve the operator
		};

		// for between operator
		struct
		{
			sql_expression* btwn_input;
			sql_expression* bounds[2];
		};

		// for in operator
		struct
		{
			sql_expression* in_input;

			sql_dql* in_sub_query; // if this attribute is NULL, only then check the in_expr_list
			arraylist in_expr_list;
		};

		// for SQL_*_FLAT types
		arraylist expr_list;

		// for constant or variable
		dstring value;

		// for sql_function
		struct
		{
			dstring func_name; // name of the function
			arraylist param_expr_list;
		};

		// for sql_cast
		struct
		{
			sql_expression* cast_expr;
			sql_type cast_type;
		};

		// for sql_sub_query
		struct
		{
			sql_dql* sub_query;
		};
	};
};

void initialize_expr_list(arraylist* expr_list);
void insert_in_expr_list(arraylist* expr_list, sql_expression* expr);

sql_expression* new_unary_sql_expr(sql_expression_type type, sql_expression* unary_of);

sql_expression* new_binary_sql_expr(sql_expression_type type, sql_expression* left, sql_expression* right);

sql_expression* new_compare_sql_expr(sql_expression_type type, sql_cmp_quantifier cmp_rhs_quantfier, sql_expression* left, sql_expression* right);

sql_expression* new_between_sql_expr(sql_expression* input, sql_expression* bounds0, sql_expression* bounds1);

sql_expression* new_flat_sql_expr(sql_expression_type type, arraylist expr_list);

sql_expression* new_in_sql_expr(sql_expression* in_input, sql_dql* in_sub_query, arraylist in_expr_list);

sql_expression* new_func_sql_expr(dstring func_name, arraylist param_expr_list);

sql_expression* new_cast_sql_expr(sql_expression* cast_expr, sql_type cast_type);

sql_expression* new_sub_query_sql_expr(sql_dql* sub_query);

// for NUM, STR and VAR
sql_expression* new_valued_sql_expr(sql_expression_type type, dstring value);

// for unknown, true, false and null
sql_expression* new_const_non_valued_sql_expr(sql_expression_type type);

// the below function destroys the old tree and returns a new one
sql_expression* flatten_similar_associative_operators_in_sql_expression(sql_expression* expr);

void print_sql_expr(const sql_expression* expr);

void delete_sql_expr(sql_expression* expr);

#endif