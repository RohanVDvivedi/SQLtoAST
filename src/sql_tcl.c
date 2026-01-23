#include<sqltoast/sql_tcl.h>

void destroytcl(sql_tcl* tcl)
{
	if(tcp->type == SAVEPOINT_QUERY || tcl->type == ROLLBACK_TO_SAVEPOINT_QUERY || tcl->type == RELEASE_SAVEPOINT_QUERY)
		deinit_dstring(&(tcl->savepoint_name));

	free(tcl);
}