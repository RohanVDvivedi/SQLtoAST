#ifndef SQL_EXPRESSION_EVAL_H
#define SQL_EXPRESSION_EVAL_H

#include<sqltoast/sql_expression.h>

int has_sub_query_in_sql_exp(const sql_expression* expr);

typedef struct sql_expr_eval_context sql_expr_eval_context;

typedef void* (*sql_user_function)(void** data_params, uint32_t params_count, const sql_expr_eval_context* ec_p, int* error_code);

struct sql_expr_eval_context
{
	// to be used only by the user, not internally used
	void* context_p;

	// true is 1 and 0 is false
	int (*get_bool)(void* data, const sql_expr_eval_context* ec_p, int* error_code); // used for all logical operators

	// constants, static
	union
	{
		void* one;
		void* true_bool;
	};
	union
	{
		void* zero;
		void* false_bool;
	};
	void* unknown;
	void* minus_one;
	// NULL is just NULL

	// basic arithmetic
	void* (*add)(void* data1, void* data2, const sql_expr_eval_context* ec_p, int* error_code);
	void* (*sub)(void* data1, void* data2, const sql_expr_eval_context* ec_p, int* error_code);
	void* (*mul)(void* data1, void* data2, const sql_expr_eval_context* ec_p, int* error_code);
	void* (*div)(void* data1, void* data2, const sql_expr_eval_context* ec_p, int* error_code);
	void* (*mod)(void* data1, void* data2, const sql_expr_eval_context* ec_p, int* error_code);

	int (*compare)(void* data1, void* data2, const sql_expr_eval_context* ec_p, int* error_code); // returns sign of data1 - data2

	// left shift/right shift
	void* (*left_shift)(void* data, void* shift_amt, const sql_expr_eval_context* ec_p, int* error_code);
	void* (*right_shift)(void* data, void* shift_amt, const sql_expr_eval_context* ec_p, int* error_code);

	// bit-wise ops
	void* (*bit_and)(void* data1, void* data2, const sql_expr_eval_context* ec_p, int* error_code);
	void* (*bit_or)(void* data1, void* data2, const sql_expr_eval_context* ec_p, int* error_code);
	void* (*bit_xor)(void* data1, void* data2, const sql_expr_eval_context* ec_p, int* error_code);
	void* (*bit_not)(void* data, const sql_expr_eval_context* ec_p, int* error_code);

	void* (*cast)(void* data, const sql_type* to_type, const sql_expr_eval_context* ec_p, int* error_code);

	void* (*create_number)(const dstring* data_bytes, const sql_expr_eval_context* ec_p, int* error_code);
	void* (*create_string)(const dstring* data_bytes, const sql_expr_eval_context* ec_p, int* error_code);

	// concat
	void (*concat)(void** data1_p, void* data2, const sql_expr_eval_context* ec_p, int* error_code);

	// destroys any 1 data
	void (*delete_data)(void* data, const sql_expr_eval_context* ec_p);

	void* (*get_sub_query)(const sql_dql* dql, const sql_expr_eval_context* ec_p, int* error_code);
	void* (*next_data_from_sub_query)(void* sub_query, const sql_expr_eval_context* ec_p, int* error_code);
	void* (*delete_sub_query)(void* sub_query, const sql_expr_eval_context* ec_p);

	sql_user_function (*get_function)(const dstring* identifier_bytes, uint32_t params_count, const sql_expr_eval_context* ec_p, int* error_code);

	// get value for the variable
	void* (*get_variable)(const dstring* identifier_bytes, const sql_expr_eval_context* ec_p, int* error_code);
};

void* evaluate_sql_expr(const sql_expression* expr, const sql_expr_eval_context* ec_p, int* error_code);

#endif