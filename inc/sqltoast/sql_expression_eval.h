#ifndef SQL_EXPRESSION_EVAL_H
#define SQL_EXPRESSION_EVAL_H

#include<sqltoast/sql_expression.h>

int has_sub_query_in_sql_exp(const sql_expression* expr);

// any of the functions below inside sql_expr_eval_context OR evaluate_sql_expr, must return NULL on setting or receiving error_code

typedef struct sql_expr_eval_context sql_expr_eval_context;
struct sql_expr_eval_context
{
	// to be used only by the user, not internally used
	void* context_p;

	// used for all logical operators only returns true_bool, false_bool and unknown_bool
	// output of this function must never be freed
	void* (*get_bool)(void* data, const sql_expr_eval_context* ec_p, int* error_code);
	// anything should be convertible into true_bool, false_bool and unknown_bool (often for NULL and unknown)
	// whether empty string is true_bool or false_bool is up to you
	// non-zero in all number types must be true_bool and exact-zero in all number types must be false_bool, (ideally, for non-ideal cases, only god knows what you want)

	// constants, static, and needs special handling
	void* true_bool;
	void* false_bool;
	void* unknown_bool;
	void* one_number;
	void* zero_number;
	void* minus_one_number;
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

	// like
	void* (*like)(void* str_p, void* pattern_p, const sql_expr_eval_context* ec_p, int* error_code);

	// destroys any data, NO-OP if the void* constansts, static provided in this struct are passed
	void (*delete_data)(void* data, const sql_expr_eval_context* ec_p);

	void* (*get_sub_query)(const sql_dql* dql, const sql_expr_eval_context* ec_p, int* error_code);
	void* (*next_data_from_sub_query)(void* sub_query, int* end_of_results, const sql_expr_eval_context* ec_p, int* error_code);
	void* (*delete_sub_query)(void* sub_query, const sql_expr_eval_context* ec_p);

	void* (*call_function)(const dstring* identifier_bytes, void** params, uint32_t params_count, const sql_expr_eval_context* ec_p, int* error_code);

	// get value for the variable
	void* (*get_variable)(const dstring* identifier_bytes, const sql_expr_eval_context* ec_p, int* error_code);

	//
	//
	// below callbacks work with only type (may not be sql_type) dictated by the implementer of ec_p
	// the below functions may be left unimplemented if you prefer dynamic typing and may never expect to call infer_type_sql_expr() function given below
	// this (the whole interface) allows users to implement the basic functions and let these functions in this library handle the complex error-prone operator boilerplate
	//
	//

	void* bool_type; // exact static constant type for the unknown_bool, true_bool and false_bool above
	// returned by all the logical and comparison operators

	// returns non-zero if the types are comparable
	int (*can_compare_types)(void* typ1, void* typ2, const sql_expr_eval_context* ec_p, int* error_code);

	// for cast operations
	void* (*get_type_for_sql_type)(const sql_type* type, const sql_expr_eval_context* ec_p, int* error_code);
	int (*can_cast_types)(void* typ_to, void* typ_from, const sql_expr_eval_context* ec_p, int* error_code);

	void* (*get_return_type_for_op_exec_callback)(void* op_exec_func, void* typ1, void* typ2, const sql_expr_eval_context* ec_p, int* error_code);
	// op_exec_func, here is bit_and, add, sub, left_shift, concat, like etc

	void* (*get_type_for_data)(void* data, const sql_expr_eval_context* ec_p, int* error_code);
	// prelimnarily used for constant expressions to get their type

	void* (*get_type_for_sub_query)(const sql_dql* dql, const sql_expr_eval_context* ec_p, int* error_code);
	
	void* (*get_return_type_for_function)(const dstring* identifier_bytes, void** param_typs, uint32_t params_count, const sql_expr_eval_context* ec_p, int* error_code);

	void* (*get_type_for_variable)(const dstring* identifier_bytes, const sql_expr_eval_context* ec_p, int* error_code);
};

void* evaluate_sql_expr(const sql_expression* expr, const sql_expr_eval_context* ec_p, int* error_code);

void* infer_type_sql_expr(const sql_expression* expr, const sql_expr_eval_context* ec_p, int* error_code);

#endif