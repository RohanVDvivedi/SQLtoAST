#include<sqltoast/sql_logical_plan.h>

#include<stdlib.h>

/*
	logical order of the sql clauses

	FROM
	JOIN
	WHERE
	GROUP BY / AGGREGATION
	HAVING
	PROJECTION
	DISTINCT
	UNION/INTERSECT/EXCEPT
	ORDER BY
	OFFSET
	LIMIT
	INSERT/DELETE - if DML
	PROJECTION if returning is present -> can be input to some other query
*/

#include<sqltoast/sql_expression_iter.h>

typedef enum expression_breaker_error expression_breaker_error;
enum expression_breaker_error
{
	NO_ERROR_FROM_EXPRESSION_BREAKER,
	TOO_DEEP_AGGREGATION_DEPTH_FROM_EXPRESSION_BREAKER, // more aggregation calls depth than expected_aggregation_depth_calls
	UNIDENTIFIED_FUNCTION_FROM_EXPRESSION_BREAKER,
};

// every sql expression in a single query without any nested queries and CTEs, is exacly expression -> aggregate-function -> expression
// it will also set the is_aggregation bit in the function calls or clear it
typedef struct expression_breaker_context expression_breaker_context;
struct expression_breaker_context
{
	// helper integrface to get schema information
	schema_query_interface* sqi;
	uint8_t expected_aggregation_depth_calls; // set to 0 for where, join conditons, limit, offset and group by clauses
	                                          // set to 1 for the rest in the expression clauses like projections, having and order by

	// every pre-order callback pushes on to this stack and analyzes the expression and increments the aggregation_calls_in_expressions_stack
	arraylist expressions_stack;
	uint32_t aggregation_calls_in_expressions_stack;

	// actual expression stack
	arraylist before_aggregation_expressions;
	arraylist aggregation_call_expressions;	    // will get an intermediate_column_name for after_aggregation_expressions to evaluate them
	arraylist after_aggregation_expressions;    // every expression processed gets a single entry here
                                                // for expressions with no aggregates, there will be no entry in before_aggregation_expressions and aggregation_call_expressions

	arraylist accessed_variable_identifiers;    // list of all column identifiers

	expression_breaker_error error;
};

static void initialize_expression_breaker_context(expression_breaker_context* ebc, schema_query_interface* sqi, uint8_t expected_aggregation_depth_calls);
static traversal_status pre_order_callback_expression_breaker(const sql_expression* expr, void* context);
static void post_order_callback_expression_breaker(const sql_expression* expr, void* context);
static void deinitialize_expression_breaker_context(expression_breaker_context* ebc);

static logical_operator* get_logical_plan_for_relation_input(const relation_input* ri_p, schema_query_interface* sqi, logical_plan_error* error);
static logical_operator* get_logical_plan_for_dql(const sql_dql* dql, schema_query_interface* sqi, logical_plan_error* error);

static logical_operator* get_logical_plan_for_relation_input(const relation_input* ri_p, schema_query_interface* sqi, logical_plan_error* error)
{
	switch(ri_p->type)
	{
		case RELATION :
		{
			void* table_handle = sqi->fetch_table_handle_by_name(&(ri_p->relation_name), sqi->schema_context);
			if(table_handle == NULL)
			{
				(*error) = LOGICAL_PLAN_UNIDENTIFIED_TABLE;
				return NULL;
			}

			logical_operator* result = malloc(sizeof(logical_operator));
			result->type = TABLE_SCAN_LOGI_OP;
			result->table_scan_info.scan_name = get_dstring_pointing_to_dstring(&(ri_p->relation_name));
			result->table_scan_info.table_handle = table_handle;
			return result;
		}
		case SUB_QUERY :
		{
			return get_logical_plan_for_dql(ri_p->sub_query, sqi, error);
		}
		case FUNCTION_CALL : // HANDLED LATER
		{
			(*error) = LOGICAL_PLAN_UNSUPPORTED_QUERY;
			return NULL;
		}
	}

	(*error) = LOGICAL_PLAN_UNSUPPORTED_QUERY;
	return NULL;
}

static logical_operator* get_logical_plan_for_dql(const sql_dql* dql, schema_query_interface* sqi, logical_plan_error* error)
{
	switch(dql->type)
	{
		case SET_OPERATION :
		{
			logical_operator* input_operator[2] = {NULL, NULL};

			input_operator[0] = get_logical_plan_for_dql(dql->set_operation.left, sqi, error);
			if(*error)
				return NULL;

			input_operator[1] = get_logical_plan_for_dql(dql->set_operation.right, sqi, error);
			if(*error)
			{
				delete_logical_plan(input_operator[0]);
				return NULL;
			}

			unsigned int is_all;
			switch(dql->set_operation.op_mod)
			{
				case SQL_RESULT_SET_DISTINCT :
				{
					is_all = 0;
					break;
				}
				case SQL_RESULT_SET_ALL :
				{
					is_all = 1;
					break;
				}
				default :
					goto SET_OP_UNIDENTIFIED;
			}

			logical_operator_type type;
			switch(dql->set_operation.op_type)
			{
				case SQL_SET_UNION :
				{
					type = UNION_LOGI_OP;
					break;
				}
				case SQL_SET_INTERSECT :
				{
					type = INTERSECT_LOGI_OP;
					break;
				}
				case SQL_SET_EXCEPT :
				{
					type = EXCEPT_LOGI_OP;
					break;
				}
				default :
					goto SET_OP_UNIDENTIFIED;
			}

			logical_operator* result = malloc(sizeof(logical_operator));
			result->type = type;
			result->set_op_info.input_operator[0] = input_operator[0];
			result->set_op_info.input_operator[1] = input_operator[1];
			result->set_op_info.is_all = is_all;

			return result;

			SET_OP_UNIDENTIFIED:;
			delete_logical_plan(input_operator[0]);
			delete_logical_plan(input_operator[1]);
			(*error) = LOGICAL_PLAN_UNSUPPORTED_QUERY;
			return NULL;
		}
		case VALUES_QUERY : // HANDLED LATER
		{
			(*error) = LOGICAL_PLAN_UNSUPPORTED_QUERY;
			return NULL;
		}
		case SELECT_QUERY :
		{
			logical_operator* result = NULL;

			// scan operator
			if(dql->select_query.has_base_input)
			{
				logical_operator* scan = get_logical_plan_for_relation_input(&(dql->select_query.base_input), sqi, error);
				if(*error)
				{
					if(result != NULL)
						delete_logical_plan(result);
					return NULL;
				}
				result = scan;
			}

			// join operators

			// filter operator (where clause)

			// projection before aggregation operator

			// aggregation operator (preserved keys if it has group by) (with/without group by)

			// filter operator (having clause)

			// projection after aggregation operator

			// order by

			// offset

			// limit

			return result;
		}
		default :
		{
			(*error) = LOGICAL_PLAN_UNSUPPORTED_QUERY;
			return NULL;
		}
	}

	(*error) = LOGICAL_PLAN_UNSUPPORTED_QUERY;
	return NULL;
}

logical_operator* get_logical_plan_for_sql(const sql* sql, schema_query_interface* sqi, logical_plan_error* error)
{
	(*error) = LOGICAL_PLAN_SUCCESS;
	switch(sql->type)
	{
		case DQL :
			return get_logical_plan_for_dql(sql->dql_query, sqi, error);
		case DML : // HANDLED LATER
		{
			(*error) = LOGICAL_PLAN_UNSUPPORTED_QUERY;
			return NULL;
		}
		default :
		{
			(*error) = LOGICAL_PLAN_UNSUPPORTED_QUERY;
			return NULL;
		}
	}
}

static void initialize_expression_breaker_context(expression_breaker_context* ebc, schema_query_interface* sqi, uint8_t expected_aggregation_depth_calls)
{
	ebc->sqi = sqi;
	ebc->expected_aggregation_depth_calls = expected_aggregation_depth_calls;

	initialize_arraylist(&(ebc->expressions_stack), 5);
	ebc->aggregation_calls_in_expressions_stack = 0;

	initialize_arraylist(&(ebc->before_aggregation_expressions), 5);
	initialize_arraylist(&(ebc->aggregation_call_expressions), 5);
	initialize_arraylist(&(ebc->after_aggregation_expressions), 5);

	initialize_arraylist(&(ebc->accessed_variable_identifiers), 5);

	ebc->error = NO_ERROR_FROM_EXPRESSION_BREAKER;
}

static traversal_status pre_order_callback_expression_breaker(const sql_expression* expr, void* context)
{
	expression_breaker_context* ebc = context;

	if(expr->type == SQL_FUNCTION_CALL)
	{
		void* function_handle = ebc->sqi->fetch_function_handle_by_name(&(expr->func_name), ebc->sqi->schema_context);
		if(function_handle == NULL)
		{
			ebc->error = UNIDENTIFIED_FUNCTION_FROM_EXPRESSION_BREAKER;
			return EXIT_ITERATION;
		}
		if(ebc->sqi->is_aggregate_function_for_fetched_function_handle(function_handle, ebc->sqi->schema_context))
		{
			((sql_expression*)expr)->is_aggregate = 1;
		}

		if(expr->is_aggregate)
		{
			ebc->aggregation_calls_in_expressions_stack++;
			if(is_full_arraylist(&(ebc->expressions_stack)) && !expand_arraylist(&(ebc->expressions_stack)))
				exit(-1);
			push_back_to_arraylist(&(ebc->expressions_stack), expr);
		}

		if(ebc->aggregation_calls_in_expressions_stack > ebc->expected_aggregation_depth_calls)
		{
			ebc->error = TOO_DEEP_AGGREGATION_DEPTH_FROM_EXPRESSION_BREAKER;
			return EXIT_ITERATION;
		}

		if(expr->is_aggregate)
		{
			if(is_full_arraylist(&(ebc->aggregation_call_expressions)) && !expand_arraylist(&(ebc->aggregation_call_expressions)))
				exit(-1);
			push_back_to_arraylist(&(ebc->aggregation_call_expressions), expr);

			for(uint32_t i = 0; i < get_element_count_arraylist(&(expr->param_expr_list)); i++)
			{
				if(is_full_arraylist(&(ebc->before_aggregation_expressions)) && !expand_arraylist(&(ebc->before_aggregation_expressions)))
					exit(-1);
				push_back_to_arraylist(&(ebc->before_aggregation_expressions), get_from_front_of_arraylist(&(expr->param_expr_list), i));
			}
		}
	}
	else if(expr->type == SQL_VAR)
	{
		if(is_full_arraylist(&(ebc->accessed_variable_identifiers)) && !expand_arraylist(&(ebc->accessed_variable_identifiers)))
			exit(-1);
		push_back_to_arraylist(&(ebc->accessed_variable_identifiers), expr);
	}

	return CONTINUE_ITERATION;
}

static void post_order_callback_expression_breaker(const sql_expression* expr, void* context)
{
	expression_breaker_context* ebc = context;

	if(expr == get_back_of_arraylist(&(ebc->expressions_stack)))
	{
		if(expr->type == SQL_FUNCTION_CALL)
		{
			if(expr->is_aggregate)
			{
				ebc->aggregation_calls_in_expressions_stack--;
				pop_back_from_arraylist(&(ebc->expressions_stack));
			}
		}
	}
}

static void deinitialize_expression_breaker_context(expression_breaker_context* ebc)
{
	deinitialize_arraylist(&(ebc->expressions_stack));
	deinitialize_arraylist(&(ebc->before_aggregation_expressions));
	deinitialize_arraylist(&(ebc->aggregation_call_expressions));
	deinitialize_arraylist(&(ebc->after_aggregation_expressions));
	deinitialize_arraylist(&(ebc->accessed_variable_identifiers));
}