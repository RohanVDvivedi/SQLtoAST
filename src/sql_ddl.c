#include<sqltoast/sql_ddl.h>

#include<stdio.h>
#include<stdlib.h>

sql_ddl* new_ddl(sql_ddl_type type, sql_object_type object_type)
{
	sql_ddl* ddl = malloc(sizeof(sql_ddl));

	ddl->type = type;
	ddl->object_type = object_type;

	// every ddl query has an object name
	init_empty_dstring(&(ddl->object_name), 0);

	switch(type)
	{
		case CREATE_QUERY :
		{
			switch(object_type)
			{
				case SQL_SCHEMA :
				{
					init_empty_dstring(&(ddl->create_schema_query.authorization), 0);
					break;
				}
				default :
				{
					break;
				}
			}
			break;
		}
		case ALTER_QUERY :
		{
			break;
		}
		case DROP_QUERY :
		{
			break;
		}
		case TRUNCATE_QUERY :
		{
			break;
		}
	}

	return ddl;
}

void print_ddl(const sql_ddl* ddl)
{
	switch(ddl->type)
	{
		case CREATE_QUERY :
		{
			printf("CREATE");
			break;
		}
		case ALTER_QUERY :
		{
			printf("ALTER");
			break;
		}
		case DROP_QUERY :
		{
			printf("DROP");
			break;
		}
		case TRUNCATE_QUERY :
		{
			printf("TRUNCATE");
			break;
		}
	}

	printf(" ");

	switch(ddl->object_type)
	{
		//case SQL_CATALOG :
		case SQL_DATABASE :
		{
			printf("DATABASE/CATALOG");
			break;
		}
		case SQL_SCHEMA :
		{
			printf("SCHEMA");
			break;
		}
		case SQL_TABLE :
		{
			printf("TABLE");
			break;
		}
		case SQL_VIEW :
		{
			printf("VIEW");
			break;
		}
		case SQL_INDEX :
		{
			printf("INDEX");
			break;
		}
		case SQL_FUNCTION :
		{
			printf("FUNCTION");
			break;
		}
		case SQL_PROCEDURE :
		{
			printf("PROCEDURE");
			break;
		}
		case SQL_TYPE :
		{
			printf("TYPE");
			break;
		}
		case SQL_DOMAIN :
		{
			printf("DOMAIN");
			break;
		}
		case SQL_SEQUENCE :
		{
			printf("SEQUENCE");
			break;
		}
		case SQL_TRIGGER :
		{
			printf("TRIGGER");
			break;
		}
	}

	printf("( ");

	int clauses_printed = 0;

	{
		if(clauses_printed != 0)
			printf(" , ");
		printf("name = ");
		printf_dstring(&(ddl->object_name));
		clauses_printed++;
	}

	switch(ddl->type)
	{
		case CREATE_QUERY :
		{
			switch(ddl->object_type)
			{
				case SQL_SCHEMA :
				{
					if(!is_empty_dstring(&(ddl->create_schema_query.authorization)))
					{
						if(clauses_printed != 0)
							printf(" , ");
						printf("authorization = ");
						printf_dstring(&(ddl->create_schema_query.authorization));
						clauses_printed++;
					}
					break;
				}
				default :
				{
					break;
				}
			}
			break;
		}
		case ALTER_QUERY :
		{
			break;
		}
		case DROP_QUERY :
		{
			{
				if(clauses_printed != 0)
					printf(" , ");
				switch(ddl->drop_behavior)
				{
					case DROP_RESTRICT :
					{
						printf("RESTRICT");
						break;
					}
					case DROP_CASCADE :
					{
						printf("CASCADE");
						break;
					}
				}
				clauses_printed++;
			}
			break;
		}
		case TRUNCATE_QUERY :
		{
			break;
		}
	}

	printf(" )");
}

void delete_ddl(sql_ddl* ddl)
{
	switch(ddl->type)
	{
		case CREATE_QUERY :
		{
			switch(ddl->object_type)
			{
				case SQL_SCHEMA :
				{
					deinit_dstring(&(ddl->create_schema_query.authorization));
					break;
				}
				default :
				{
					break;
				}
			}
			break;
		}
		case ALTER_QUERY :
		{
			break;
		}
		case DROP_QUERY :
		{
			break;
		}
		case TRUNCATE_QUERY :
		{
			break;
		}
	}

	deinit_dstring(&(ddl->object_name));
	free(ddl);
}