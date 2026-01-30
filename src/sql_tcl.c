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
		printf("isolation_level = ");
		switch(tcl->isolation_level)
		{
			case ISO_UNSPECIFIED : printf("ISO_UNSPECIFIED"); break;
			case ISO_READ_UNCOMMITTED : printf("ISO_READ_UNCOMMITTED"); break;
			case ISO_READ_COMMITTED : printf("ISO_READ_COMMITTED"); break;
			case ISO_REPEATABLE_READ : printf("ISO_REPEATABLE_READ"); break;
			case ISO_SERIALIZABLE : printf("ISO_SERIALIZABLE"); break;
		}
		clauses_printed++;
	}

	if(tcl->mode != TX_ACC_RW_UNSPECIFIED)
	{
		if(clauses_printed != 0)
			printf(" , ");
		printf("mode = ");
		switch(tcl->mode)
		{
			case TX_ACC_RW_UNSPECIFIED : printf("TX_ACC_RW_UNSPECIFIED"); break;
			case TX_ACC_RW_READ_ONLY : printf("TX_ACC_RW_READ_ONLY"); break;
			case TX_ACC_RW_READ_WRITE : printf("TX_ACC_RW_READ_WRITE"); break;
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
			printf("start_tx (");

			print_iso_and_mode(tcl);

			printf(" )");
			break;
		}
		case COMMIT_TCL_CMD :
		{
			printf("commit");
			break;
		}
		case ROLLBACK_TCL_CMD :
		{
			printf("rollback");
			break;
		}

		case SAVEPOINT_TCL_CMD :
		{
			printf("savepoint( \"");
			printf_dstring(&(tcl->savepoint_name));
			printf("\" )");
			break;
		}
		case ROLLBACK_TO_SAVEPOINT_TCL_CMD :
		{
			printf("rollback_to_savepoint( \"");
			printf_dstring(&(tcl->savepoint_name));
			printf("\" )");
			break;
		}
		case RELEASE_SAVEPOINT_TCL_CMD :
		{
			printf("release_savepoint( \"");
			printf_dstring(&(tcl->savepoint_name));
			printf("\" )");
			break;
		}

		case SET_TX_TCL_CMD :
		{
			printf("set_tx( ");

			print_iso_and_mode(tcl);

			printf(" )");
			break;
		}
		case SET_TX_CHARACTERISTICS_TCL_CMD :
		{
			printf("set_tx_characteristics( ");

			print_iso_and_mode(tcl);

			printf(")");
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