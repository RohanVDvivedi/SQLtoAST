#ifndef SQL_EXPRESSION_EVAL_H
#define SQL_EXPRESSION_EVAL_H

#include<sqltoast/sql_expression.h>

int has_sub_query_in_sql_exp(const sql_expression* expr);

typedef void* (*sql_user_function)();

typedef struct sql_expr_eval_context sql_expr_eval_context;
struct sql_expr_eval_context
{
	// true is 1 and 0 is false
	int (*bool)(void* data, int* error_code); // used for all logical operators

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
	void* (*add)(void* data1, void* data2, int* error_code);
	void* (*sub)(void* data1, void* data2, int* error_code);
	void* (*mul)(void* data1, void* data2, int* error_code);
	void* (*div)(void* data1, void* data2, int* error_code);
	void* (*mod)(void* data1, void* data2, int* error_code);

	int (*compare)(void* data1, void* data2, int* error_code); // returns sign of data1 - data2

	// left shift/right shift
	void* (*left_shift)(void* data, void* shift_amt, int* error);
	void* (*right_shift)(void* data, void* shift_amt, int* error);

	// bit-wise ops
	void* (*bit_and)(void* data1, void* data2, int* error_code);
	void* (*bit_or)(void* data1, void* data2, int* error_code);
	void* (*bit_xor)(void* data1, void* data2, int* error_code);
	void* (*bit_not)(void* data, int* error_code);

	void* (*cast)(void* data, const sql_type* to_type, int* error_code);

	void* (*create_number)(const char* bytes, int* error_code);
	void* (*create_string)(const char* bytes, int* error_code);

	// concat
	void (*concat)(void** data1_p, void* data2, int* error_code);

	// destroys any 1 data
	void (*destroy)(void* data);

	void* (*create_sub_query)(const char* bytes, int* error_code);
	void* (*next_data_from_sub_query)(void* sub_query);
	void* (*destroy_sub_query)(void* sub_query);

	sql_user_function (*create_function)(const char* function_name, uint32_t params_count, int* error_code);
};

#endif