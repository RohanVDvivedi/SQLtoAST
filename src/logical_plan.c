#include<sqltoast/sql_logical_plan.h>

logical_operator* get_logical_plan_for_sql(sql* sql, uint32_t* result_operators_count);