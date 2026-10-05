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
		case VALUES_QUERY :
		{
			return NULL;
		}
		case SELECT_QUERY :
		{
			return NULL;
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
		case DML :
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