#include<sqltoast/sql_tcl.h>

#include<stdlib.h>
#include<stdio.h>

sql_tcl* new_tcl(sql_tcl_type type)
{
	sql_tcl* tcl = malloc(sizeof(sql_tcl));

	tcl->type = type;

	if(type == SAVEPOINT_TCL_CMD || type == ROLLBACK_TO_SAVEPOINT_TCL_CMD || type == RELEASE_SAVEPOINT_TCL_CMD)
		init_empty_dstring(&(tcl->savepoint_name), 0);
	else if(type == START_TX_TCL_CMD || type == SET_TX_TCL_CMD || type == SET_TX_CHARACTERISTICS_TCL_CMD)
	{
		tcl->isolation_level = ISO_UNSPECIFIED;
		tcl->mode = TX_ACC_RW_UNSPECIFIED;
	}

	return tcl;
}

static void snprint_iso_and_mode(dstring* str_p, const sql_tcl* tcl)
{
	int clauses_printed = 0;

	if(tcl->isolation_level != ISO_UNSPECIFIED)
	{
		if(clauses_printed != 0)
			printf(" , ");

		printf("ISOLATION LEVEL ");

		switch(tcl->isolation_level)
		{
			case ISO_READ_UNCOMMITTED:
				printf("READ UNCOMMITTED");
				break;

			case ISO_READ_COMMITTED:
				printf("READ COMMITTED");
				break;

			case ISO_REPEATABLE_READ:
				printf("REPEATABLE READ");
				break;

			case ISO_SERIALIZABLE:
				printf("SERIALIZABLE");
				break;

			default:
				break;
		}

		clauses_printed++;
	}

	if(tcl->mode != TX_ACC_RW_UNSPECIFIED)
	{
		if(clauses_printed != 0)
			printf(" , ");

		switch(tcl->mode)
		{
			case TX_ACC_RW_READ_ONLY:
				printf("READ ONLY");
				break;

			case TX_ACC_RW_READ_WRITE:
				printf("READ WRITE");
				break;

			default:
				break;
		}

		clauses_printed++;
	}
}

void snprint_tcl(dstring* str_p, const sql_tcl* tcl)
{
	switch(tcl->type)
	{
		case START_TX_TCL_CMD :
		{
			snprintf_dstring(str_p, "START TRANSACTION ");
			snprint_iso_and_mode(str_p, tcl);
			break;
		}
		case COMMIT_TCL_CMD :
		{
			snprintf_dstring(str_p, "COMMIT");
			break;
		}
		case ROLLBACK_TCL_CMD :
		{
			snprintf_dstring(str_p, "ROLLBACK");
			break;
		}

		case SAVEPOINT_TCL_CMD :
		{
			snprintf_dstring(str_p, "SAVEPOINT ");
			concatenate_dstring(str_p, &(tcl->savepoint_name));
			break;
		}
		case ROLLBACK_TO_SAVEPOINT_TCL_CMD :
		{
			snprintf_dstring(str_p, "ROLLBACK TO SAVEPOINT ");
			concatenate_dstring(str_p, &(tcl->savepoint_name));
			break;
		}
		case RELEASE_SAVEPOINT_TCL_CMD :
		{
			snprintf_dstring(str_p, "RELEASE SAVEPOINT ");
			concatenate_dstring(str_p, &(tcl->savepoint_name));
			break;
		}

		case SET_TX_TCL_CMD :
		{
			snprintf_dstring(str_p, "SET TRANSACTION ");
			snprint_iso_and_mode(str_p, tcl);
			break;
		}
		case SET_TX_CHARACTERISTICS_TCL_CMD :
		{
			snprintf_dstring(str_p, "SET TRANSACTION CHARACTERISTICS ");
			snprint_iso_and_mode(str_p, tcl);
			break;
		}
	}
}

void delete_tcl(sql_tcl* tcl)
{
	if(tcl->type == SAVEPOINT_TCL_CMD || tcl->type == ROLLBACK_TO_SAVEPOINT_TCL_CMD || tcl->type == RELEASE_SAVEPOINT_TCL_CMD)
		deinit_dstring(&(tcl->savepoint_name));

	free(tcl);
}