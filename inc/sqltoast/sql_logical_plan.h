#ifndef SQL_LOGICAL_PLAN_H
#define SQL_LOGICAL_PLAN_H

#include<sqltoast/sqltoast.h>

typedef struct schema_query_interface schema_query_interface;
struct schema_query_interface
{
	void* schema_context;

	// return NULL if such a table does not exists
	void* (*fetch_table_handle_by_name)(const char* table_name, void* schema_context);
	uint32_t (*get_column_count_for_fetched_table_handle)(void* table_handle, void* schema_context);
	void* (*fetch_column_handle_for_fetched_table_handle_by_column_name)(void* table_handle, const char* column_name, void* schema_context);
	void* (*fetch_column_handle_for_fetched_table_handle_by_index)(void* table_handle, uint32_t index, void* schema_context); // index < get_column_count_*

	// return NULL if such a function does not exists
	void* (*fetch_function_handle_by_name)(const char* function_name, void* schema_context);
	int (*is_aggregate_function_for_fetched_function_handle)(void* function_handle, void* schema_context);
	uint32_t (*get_param_count_for_fetched_function_handle)(void* function_handle, void* schema_context);
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

	// sort operator
	SORT_LOGI_OP,

	// UNIQUE and DISTINCT are aliases, takes a stream of input and output their unique rows
	// can be implemented using a special case of aggregation operator
	DISTINCT_LOGI_OP,

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
	UNION_LOGI_OP,
	INTERSECT_LOGI_OP,
	EXCEPT_LOGI_OP,

	// output control operators
	OFFSET_LOGI_OP,
	LIMIT_LOGI_OP,
};

typedef struct table_scan_info table_scan_info;
struct table_scan_info
{
	dstring scan_name;
	void* table_handle;
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

typedef struct aggregate_call aggregate_call;
struct aggregate_call
{
	void* function_handle;

	uint32_t parameter_count;
	uint32_t* parameter_positions;
};

typedef struct aggregation_info aggregation_info;
struct aggregation_info
{
	uint32_t input_operator_index;

	uint32_t keys_count;
	uint32_t* key_positions;

	arraylist aggregate_function_calls; // has pointers for aggregate_call structs
};

typedef struct sort_info sort_info;
struct sort_info
{
	uint32_t input_operator_index;

	uint32_t keys_count;
	uint32_t* key_positions;
	unsigned int* is_key_dir_desc;
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
	unsigned int is_all:1; // if is_all == 0, implies it must have a distinct output
};

typedef struct output_control_op_info output_control_op_info;
struct output_control_op_info
{
	uint32_t input_operator_index;
	uint64_t count;
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
		sort_info sort_info;
		distinct_info distinct_info;
		join_info join_info;
		set_op_info set_op_info;
		output_control_op_info output_control_op_info;
	};
};

// sql must be DQL or DML
logical_operator* get_logical_plan_for_sql(sql* sql, uint32_t* result_operators_count, schema_query_interface* sqi);

#endif