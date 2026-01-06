#ifndef SQL_TCL_H
#define SQL_TCL_H

#include<cutlery/dstring.h>

typedef enum sql_tcl_type sql_tcl_type;
enum sql_tcl_type
{
	BEGIN_QUERY,
	COMMIT_QUERY,
	ROLLBACK_QUERY,

	SAVEPOINT_QUERY,
	ROLLBACK_TO_SAVEPOINT_QUERY,
	RELEASE_SAVEPOINT_QUERY,

	SET_TX_QUERY,	// set transaction
	SET_TX_CHARST_QUERY, // set transaction characteristics
};

typedef struct sql_tcl sql_tcl;
struct sql_tcl
{
	sql_tcl_type type;

	union
	{
		// required only for SAVEPOINT_QUERY, ROLLBACK_TO_SAVEPOINT_QUERY and RELEASE_SAVEPOINT_QUERY
		dstring savepoint_name;

		// required only for SET_TX_QUERY and SET_TX_CHARST_QUERY
		struct
		{
			dstring isolation_level;
			int read_only;
		};
	};
};

void destroytcl(sql_tcl* tcl);

#endif