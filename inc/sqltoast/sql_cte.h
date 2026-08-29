#ifndef SQL_CTE_H
#define SQL_CTE_H

#include<cutlery/arraylist.h>
#include<cutlery/dstring.h>

typedef enum sql_cte_type sql_cte_type;
enum sql_cte_type
{
	CTE_DQL,
	CTE_DML,
};

typedef struct sql_dql sql_dql;
typedef struct sql_dml sql_dml;

typedef struct sql_cte sql_cte;
struct sql_cte
{
	dstring cte_name;

	arraylist cte_column_names;

	sql_cte_type cte_type;

	union
	{
		sql_dql* dql_query;
		sql_dml* dml_query;
	};
};

sql_cte* new_cte(sql_cte_type cte_type);

void delete_cte(sql_cte* cte);

#endif