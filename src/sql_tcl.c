#include<sqltoast/sql_tcl.h>

#include<stdlib.h>

sql_tcl* new_tcl(sql_tcl_type type)
{
	sql_tcl* tcl = malloc(sizeof(sql_tcl));

	tcl->type = type;

	if(type == SAVEPOINT_QUERY || type == ROLLBACK_TO_SAVEPOINT_QUERY || type == RELEASE_SAVEPOINT_QUERY)
		init_empty_dstring(&(tcl->savepoint_name), 0);

	return tcl;
}

void delete_tcl(sql_tcl* tcl)
{
	if(tcl->type == SAVEPOINT_QUERY || tcl->type == ROLLBACK_TO_SAVEPOINT_QUERY || tcl->type == RELEASE_SAVEPOINT_QUERY)
		deinit_dstring(&(tcl->savepoint_name));

	free(tcl);
}