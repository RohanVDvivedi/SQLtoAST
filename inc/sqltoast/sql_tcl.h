#ifndef SQL_TCL_H
#define SQL_TCL_H

typedef enum sql_tcl_type sql_tcl_type;
enum sql_tcl_type
{
	BEGIN_QUERY,
	COMMIT_QUERY,
	ROLLBACK_QUERY,
};

typedef struct sql_tcl sql_tcl;
struct sql_tcl
{
	sql_tcl_type type;
};

void destroytcl(sql_tcl* tcl);

#endif