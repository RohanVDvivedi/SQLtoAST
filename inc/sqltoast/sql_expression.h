#ifndef SQL_EXPRESSION_H
#define SQL_EXPRESSION_H

#include<cutlery/dstring.h>

typedef enum sql_expression_type sql_expression_type;
enum sql_expression_type
{
	SQL_NEG,

	SQL_BITNOT,

	SQL_LOGNOT,

	// all operators above are unary operators

	SQL_ADD,
	SQL_SUB,
	SQL_MUL,
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

	SQL_LOGAND,
	SQL_LOGOR,
	SQL_LOGXOR, // logical operators

	// all operators above are binary operators

	SQL_BTWN, // between operator in sql

	// all operators above are operators that take bounds as input

	SQL_IN,

	// all operators above take expression list as input

	SQL_CONST,

	// reflects a constant

	SQL_VAR,

	// reflects a variable, or a table accessible value
};

typedef struct sql_expression sql_expression;
struct sql_expression
{
	sql_expression_type type;

	union
	{
		// for all unary operators
		sql_expression* unary_val;

		// for all binary operators
		struct
		{
			sql_expression* left;
			sql_expression* right;
		};

		// for between operator
		struct
		{
			sql_expression* bounds[2];
		};

		// for in operator
		struct
		{
			int expr_list_size;
			sql_expression* expr_list;
		};

		// for constant or variable
		dstring value;
	};
};

sql_expression* new_unary_sql_expr(sql_expression_type type, sql_expression* unary_val);

sql_expression* new_binary_sql_expr(sql_expression_type type, sql_expression* left, sql_expression* right);

sql_expression* new_between_sql_expr(sql_expression_type type, sql_expression* bounds0, sql_expression* bounds2);

sql_expression* new_in_sql_expr();

void insert_expr_to_in_sql_expr(sql_expression* in_expr, sql_expression* from_val);

sql_expression* new_valued_sql_expr(dstring* value);

#endif