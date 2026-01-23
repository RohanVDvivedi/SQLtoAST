#include<sqltoast/sql_dql.h>

#include<stdio.h>
#include<stdlib.h>

sql_dql* new_dql()
{
	sql_dql* dql = malloc(sizeof(sql_dql));

	dql->type = SELECT_QUERY;

	dql->where_expr = NULL;
	dql->having_expr = NULL;
	dql->offset_expr = NULL;
	dql->limit_expr = NULL;

	return dql;
}

void printdql(const sql_dql* dql)
{
	printf("SELECT : \n");

	printf("\nWHERE : \n\t");
	if(dql->where_expr)
		print_sql_expr(dql->where_expr);
	else
		printf("NULL\n");

	printf("\nHAVING : \n\t");
	if(dql->having_expr)
		print_sql_expr(dql->having_expr);
	else
		printf("NULL\n");

	printf("\nOFFSET : \n\t");
	if(dql->offset_expr)
		print_sql_expr(dql->offset_expr);
	else
		printf("NULL\n");

	printf("\nLIMIT : \n\t");
	if(dql->limit_expr)
		print_sql_expr(dql->limit_expr);
	else
		printf("NULL\n");
}

void destroydql(sql_dql* dql)
{
	if(dql->where_expr)
		delete_sql_expr(dql->where_expr);

	if(dql->having_expr)
		delete_sql_expr(dql->having_expr);

	if(dql->offset_expr)
		delete_sql_expr(dql->offset_expr);

	if(dql->limit_expr)
		delete_sql_expr(dql->limit_expr);

	free(dql);
}