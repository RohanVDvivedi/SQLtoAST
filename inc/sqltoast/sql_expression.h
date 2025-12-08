#ifndef SQL_EXPRESSION_H
#define SQL_EXPRESSION_H

typedef enum sql_expression_type sql_expression_type;
enum sql_expression_type
{
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
	SQL_NEQ,

	SQL_BITAND,
	SQL_BITOR,
	SQL_BITXOR,
	SQL_BITNOT,

	SQL_LOGAND,
	SQL_LOGOR,
	SQL_LOGXOR,
	SQL_LOGNOT,

	SQL_IN,
	SQL_BTWN, // between operator in sql

	SQL_CONST,

	SQL_VAR,
};

typedef struct sql_expression sql_expression;
struct sql_expression
{
	sql_expression_type type;

	union
	{
		struct
		{
			sql_expression* left;
			sql_expression* right;
		};
		struct
		{
			int expr_list_size;
			sql_expression* expr_list;
		};
	};
};

#endif