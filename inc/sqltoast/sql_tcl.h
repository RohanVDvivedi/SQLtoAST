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

typedef enum sql_isolation_level sql_isolation_level;
enum sql_isolation_level
{
	ISO_UNSPECIFIED,
	ISO_READ_UNCOMMITTED,
	ISO_READ_COMMITTED,
	ISO_REPEATABLE_READ,
	ISO_SERIALIZABLE,
};

typedef enum sql_tx_access_mode sql_tx_access_mode;
enum sql_tx_access_mode
{
	TX_ACC_RW_UNSPECIFIED,
	TX_ACC_RW_READ_ONLY,
	TX_ACC_RW_READ_WRITE,
};

typedef struct sql_tcl sql_tcl;
struct sql_tcl
{
	sql_tcl_type type;

	union
	{
		// required only for SAVEPOINT_QUERY, ROLLBACK_TO_SAVEPOINT_QUERY and RELEASE_SAVEPOINT_QUERY
		dstring savepoint_name;

		// required only for BEGIN, SET_TX_QUERY and SET_TX_CHARST_QUERY
		struct
		{
			sql_isolation_level isolation_level;
			sql_tx_access_mode mode;
		};
	};
};

sql_tcl* new_tcl(sql_tcl_type type);

void delete_tcl(sql_tcl* tcl);

#endif