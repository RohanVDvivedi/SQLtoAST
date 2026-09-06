#ifndef SQL_LOGICAL_PLAN_H
#define SQL_LOGICAL_PLAN_H

#include<sqltoast/sqltoast.h>

typedef struct schema_query_interface schema_query_interface;
struct schema_query_interface
{
	void* context;

	// return NULL if such a table does not exists
	void* (*fetch_table_handle_by_name)(const char* table_name);
	uint32_t (*get_column_count_for_fetched_table_handle)(void* table_handle);
	void* (*fetch_column_handle_for_fetched_table_handle_by_column_name)(void* table_handle, const char* column_name);
	void* (*fetch_column_handle_for_fetched_table_handle_by_index)(void* table_handle, uint32_t index); // index < get_column_count_*

	// return NULL if such a function does not exists
	void* (*fetch_function_handle_by_name)(const char* function_name);
	int (*is_aggregate_function_for_fetched_handle_handle)(void* function_handle);
	uint32_t (*get_param_count_for_fetched_handle_handle)(void* function_handle);
};

typedef enum logical_operator_type logical_operator_type;
enum logical_operator_type
{
	// source operator
	TABLE_SCAN_LOGI_OP,

	// unary operators
	SELECTION_LOGI_OP,
	PROJECTION_LOGI_OP,

	// aggregation operator
	AGGREGATION_LOGI_OP,

	// UNIQUE and DISTINCT are aliases, takes a stream of input and output their unique rows
	// can be implemented using a special case of aggregation operator
	DISTINCT_LOGI_OP,
	UNIQUE_LOGI_OP = DISTINCT_LOGI_OP,

	// binary operators
	// always with conditon, unless the condition is NULL, then it becomes cross join
	INNER_JOIN_LOGI_OP,
	LEFT_JOIN_LOGI_OP,
	RIGHT_JOIN_LOGI_OP,
	FULL_JOIN_LOGI_OP,

	// outputs only the left side
	SEMI_JOIN_LOGI_OP,
	ANTI_JOIN_LOGI_OP,

	// set operations

	UNION_ALL_LOGI_OP,
	INTERSECT_ALL_LOGI_OP,
	EXCEPT_ALL_LOGI_OP,

	// set operations same as above, but equivalent to UNIQUE_ALL -> DISTINCT
	UNION_DISTINCT_LOGI_OP,
	INTERSECT_DISTINCT_LOGI_OP,
	EXCEPT_DISTINCT_LOGI_OP,
};

typedef struct table_scan_info table_scan_info;
struct table_scan_info
{
	dstring scan_name;
};

typedef struct selection_info selection_info;
struct selection_info
{
	uint32_t input_operator_index;
	sql_expression* selection_expr;
};

typedef struct projection_info projection_info;
struct projection_info
{
	uint32_t input_operator_index;
	arraylist projection_exprs;
};

typedef struct aggregation_info aggregation_info;
struct aggregation_info
{
	uint32_t input_operator_index;

	uint32_t keys_count;
	uint32_t* key_positions;

	uint32_t aggregations_count;
	arraylist aggregate_functions; // has as many elements as aggregations_count
	uint32_t* aggregate_function_params_count; // aggregations_count is the number of elements here, aggregate_function_params_count[i] = number of params for ith function, i < aggregations_count
	uint32_t** aggregate_function_params_positions; // aggregate_function_params_positions[i < aggregations_count][j < aggregate_function_params_count[i]], is the position of the j-th parameter for the i-th aggregate function
};

typedef struct distinct_info distinct_info;
struct distinct_info
{
	uint32_t input_operator_index;
};

typedef struct join_info join_info;
struct join_info
{
	uint32_t input_operator_index[2];
	sql_expression* join_expr;
};

typedef struct set_op_info set_op_info;
struct set_op_info
{
	uint32_t input_operator_index[2];
};

typedef struct logical_operator logical_operator;
struct logical_operator
{
	logical_operator_type type;

	union
	{
		table_scan_info table_scan_info;
		selection_info selection_info;
		projection_info projection_info;
		aggregation_info aggregation_info;
		distinct_info distinct_info;
		join_info join_info;
		set_op_info set_op_info;
	};
};

// sql must be DQL or DML
logical_operator* get_logical_plan_for_sql(sql* sql, uint32_t* result_operators_count);

#endif