#include<sqltoast/sql_logical_plan.h>

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

logical_operator* get_logical_plan_for_sql(sql* sql, schema_query_interface* sqi);