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

static void print_iso_and_mode(const sql_tcl* tcl)
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

void print_tcl(const sql_tcl* tcl)
{
	switch(tcl->type)
	{
		case START_TX_TCL_CMD :
		{
			printf("START TRANSACTION ");
			print_iso_and_mode(tcl);
			break;
		}
		case COMMIT_TCL_CMD :
		{
			printf("COMMIT");
			break;
		}
		case ROLLBACK_TCL_CMD :
		{
			printf("ROLLBACK");
			break;
		}

		case SAVEPOINT_TCL_CMD :
		{
			printf("SAVEPOINT ");
			printf_dstring(&(tcl->savepoint_name));
			break;
		}
		case ROLLBACK_TO_SAVEPOINT_TCL_CMD :
		{
			printf("ROLLBACK TO SAVEPOINT ");
			printf_dstring(&(tcl->savepoint_name));
			break;
		}
		case RELEASE_SAVEPOINT_TCL_CMD :
		{
			printf("RELEASE SAVEPOINT ");
			printf_dstring(&(tcl->savepoint_name));
			break;
		}

		case SET_TX_TCL_CMD :
		{
			printf("SET TRANSACTION ");
			print_iso_and_mode(tcl);
			printf(" ");
			break;
		}
		case SET_TX_CHARACTERISTICS_TCL_CMD :
		{
			printf("SET TRANSACTION CHARACTERISTICS ");
			print_iso_and_mode(tcl);
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