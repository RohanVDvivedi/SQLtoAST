#include<sqltoast/sql_tcl.h>

#include<stdlib.h>

void destroytcl(sql_tcl* tcl)
{
	if(tcl->type == SAVEPOINT_QUERY || tcl->type == ROLLBACK_TO_SAVEPOINT_QUERY || tcl->type == RELEASE_SAVEPOINT_QUERY)
		deinit_dstring(&(tcl->savepoint_name));

	free(tcl);
}