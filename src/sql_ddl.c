#include<sqltoast/sql_ddl.h>

#include<sqltoast/arraylist_deleter.h>

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
				case SQL_TABLE :
				{
					initialize_arraylist(&(ddl->create_table_query.table_elements), 0);
					break;
				}
				case SQL_VIEW :
				{
					initialize_arraylist(&(ddl->create_view_query.column_list), 0);
					ddl->create_view_query.view_query = NULL;
					break;
				}
				case SQL_INDEX :
				{
					init_empty_dstring(&(ddl->create_index_query.table_name), 0);
					initialize_arraylist(&(ddl->create_index_query.key_exprs), 0);
					initialize_arraylist(&(ddl->create_index_query.include_exprs), 0);
					ddl->create_index_query.where_expr = NULL;
					init_empty_dstring(&(ddl->create_index_query.using_index_type), 0);
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
				case SQL_TABLE :
				{
					{
						if(clauses_printed != 0)
							printf(" , ");
						printf("table_elements = ( ");
						for(cy_uint i = 0; i < get_element_count_arraylist(&(ddl->create_table_query.table_elements)); i++)
						{
							if(i != 0)
								printf(" , ");
							const sql_table_element* te_p = get_from_front_of_arraylist(&(ddl->create_table_query.table_elements), i);
							switch(te_p->type)
							{
								case SQL_COLUMN :
								{
									const sql_column_def* c = &(te_p->column_def);

									printf_dstring(&(c->column_name));
									printf(" ");
									print_sql_type(&(c->type));
									if(c->is_not_null)
										printf(" NOT NULL ");
									if(c->is_primary_key)
										printf(" PRIMARY KEY ");
									if(c->is_unique)
										printf(" UNIQUE ");
									if(c->is_foreign_key)
									{
										printf(" FOREIGN_REFERENCE ");
										printf_dstring(&(c->foreign_table));
										if(!is_empty_dstring(&(c->foreign_column)))
										{
											printf("(");
											printf_dstring(&(c->foreign_column));
											printf(")");
										}
										printf(" ");
									}
									if(c->default_value)
									{
										printf(" DEFAULT ");
										print_sql_expr(c->default_value);
									}
									for(cy_uint i = 0; i < get_element_count_arraylist(&(c->constraint_check_exprs)); i++)
									{
										printf(" CHECK ( ");
										print_sql_expr((sql_expression*) get_from_front_of_arraylist(&(c->constraint_check_exprs), i));
										printf(" )");
									}
									break;
								}
								case SQL_CONSTRAINT :
								{
									const sql_constraint_def* c = &(te_p->constraint_def);

									printf(" CONSTRAINT ");

									printf_dstring(&(c->constraint_name));

									switch(c->type)
									{
										case SQL_UNIQUE_KEY :
										{
											printf(" UNIQUE_KEY_CONSTRAINT ");
											printf_dstring(&(c->constraint_name));
											printf(" ( ");
											for(cy_uint i = 0; i < get_element_count_arraylist(&(c->column_list)); i++)
											{
												if(i != 0)
													printf(" , ");
												printf_dstring((const dstring*)get_from_front_of_arraylist(&(c->column_list), i));
											}
											printf(" )");
											break;
										}
										case SQL_PRIMARY_KEY :
										{
											printf(" PRIMARY_KEY_CONSTRAINT ");
											printf_dstring(&(c->constraint_name));
											printf(" ( ");
											for(cy_uint i = 0; i < get_element_count_arraylist(&(c->column_list)); i++)
											{
												if(i != 0)
													printf(" , ");
												printf_dstring((const dstring*)get_from_front_of_arraylist(&(c->column_list), i));
											}
											printf(" )");
											break;
										}
										case SQL_FOREIGN_KEY :
										{
											printf(" FOREIGN_KEY_CONSTRAINT ");
											printf_dstring(&(c->constraint_name));
											printf(" ( ");
											for(cy_uint i = 0; i < get_element_count_arraylist(&(c->column_list)); i++)
											{
												if(i != 0)
													printf(" , ");
												printf_dstring((const dstring*)get_from_front_of_arraylist(&(c->column_list), i));
											}
											printf(" ) REFERENCES ");
											printf_dstring(&(c->foreign_table));
											printf(" ( ");
											for(cy_uint i = 0; i < get_element_count_arraylist(&(c->foreign_column_list)); i++)
											{
												if(i != 0)
													printf(" , ");
												printf_dstring((const dstring*)get_from_front_of_arraylist(&(c->foreign_column_list), i));
											}
											printf(" )");
											break;
										}
										case SQL_CONSTRAINT_CHECK :
										{
											printf(" CHECK_CONSTRAINT ");
											printf_dstring(&(c->constraint_name));
											printf(" ( ");
											print_sql_expr(c->constraint_check_expr);
											printf(" )");
											break;
										}
									}

									break;
								}
							}
						}
						printf(" )");
						clauses_printed++;
					}
					break;
				}
				case SQL_VIEW :
				{
					if(get_element_count_arraylist(&(ddl->create_view_query.column_list)) > 0)
					{
						if(clauses_printed != 0)
							printf(" , ");
						printf("column_list = ( ");
						for(cy_uint i = 0; i < get_element_count_arraylist(&(ddl->create_view_query.column_list)); i++)
						{
							if(i != 0)
								printf(" , ");
							printf_dstring((const dstring*)get_from_front_of_arraylist(&(ddl->create_view_query.column_list), i));
						}
						printf(" )");
						clauses_printed++;
					}
					{
						if(clauses_printed != 0)
							printf(" , ");
						printf("view_query = ( ");
						print_dql(ddl->create_view_query.view_query);
						printf(" )");
						clauses_printed++;
					}
					if(ddl->create_view_query.check_option != CHECK_OPTION_NONE)
					{
						if(clauses_printed != 0)
							printf(" , ");
						printf("check_option = ");
						switch(ddl->create_view_query.check_option)
						{
							case CHECK_OPTION_NONE :
							{
								printf("NONE");
								break;
							}
							case CHECK_OPTION_LOCAL :
							{
								printf("LOCAL");
								break;
							}
							case CHECK_OPTION_CASCADED :
							{
								printf("CASCADED");
								break;
							}
						}
						clauses_printed++;
					}
					break;
				}
				case SQL_INDEX :
				{
					{
						if(clauses_printed != 0)
							printf(" , ");
						printf("on_table = ");
						printf_dstring(&(ddl->create_index_query.table_name));
						clauses_printed++;
					}
					{
						if(clauses_printed != 0)
							printf(" , ");
						printf("keys = ( ");
						for(cy_uint i = 0; i < get_element_count_arraylist(&(ddl->create_index_query.key_exprs)); i++)
						{
							if(i != 0)
								printf(" , ");
							index_key_expr* k = (index_key_expr*) get_from_front_of_arraylist(&(ddl->create_index_query.key_exprs), i);
							printf("( ( ");
							print_sql_expr(k->ordering_expr);
							printf(" ) in %s order )", ((k->dir == ORDER_BY_ASC) ? "ascending" : "descending"));
						}
						clauses_printed++;
					}
					if(get_element_count_arraylist(&(ddl->create_index_query.include_exprs)) > 0)
					{
						if(clauses_printed != 0)
							printf(" , ");
						printf("include = ( ");
						for(cy_uint i = 0; i < get_element_count_arraylist(&(ddl->create_index_query.include_exprs)); i++)
						{
							if(i != 0)
								printf(" , ");
							sql_expression* x = (sql_expression*) get_from_front_of_arraylist(&(ddl->create_index_query.include_exprs), i);
							printf("( ");
							print_sql_expr(x);
							printf(" )");
						}
						printf(" )");
						clauses_printed++;
					}
					if(ddl->create_index_query.where_expr != NULL)
					{
						if(clauses_printed != 0)
							printf(" , ");
						printf("where = ( ");
						printf_sql_expr(ddl->create_index_query.where_expr);
						printf(" )");
						clauses_printed++;
					}
					if(!is_empty_dstring(&(ddl->create_index_query.using_index_type)))
					{
						if(clauses_printed != 0)
							printf(" , ");
						printf("using_index_type = ");
						printf_dstring(&(ddl->create_index_query.using_index_type));
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

void delete_table_element(sql_table_element* table_element)
{
	switch(table_element->type)
	{
		case SQL_COLUMN :
		{
			sql_column_def* c = &(table_element->column_def);

			deinit_dstring(&(c->column_name));
			deinit_dstring(&(c->foreign_table));
			deinit_dstring(&(c->foreign_column));
			if(c->default_value)
				delete_sql_expr(c->default_value);
			for(cy_uint i = 0; i < get_element_count_arraylist(&(c->constraint_check_exprs)); i++)
				delete_sql_expr((sql_expression*) get_from_front_of_arraylist(&(c->constraint_check_exprs), i));
			deinitialize_arraylist(&(c->constraint_check_exprs));

			break;
		}
		case SQL_CONSTRAINT :
		{
			sql_constraint_def* c = &(table_element->constraint_def);

			deinit_dstring(&(c->constraint_name));

			for(cy_uint i = 0; i < get_element_count_arraylist(&(c->column_list)); i++)
			{
				dstring* col_name = (dstring*) get_from_front_of_arraylist(&(c->column_list), i);
				deinit_dstring(col_name);
				free(col_name);
			}
			deinitialize_arraylist(&(c->column_list));

			deinit_dstring(&(c->foreign_table));

			for(cy_uint i = 0; i < get_element_count_arraylist(&(c->foreign_column_list)); i++)
			{
				dstring* col_name = (dstring*) get_from_front_of_arraylist(&(c->foreign_column_list), i);
				deinit_dstring(col_name);
				free(col_name);
			}
			deinitialize_arraylist(&(c->foreign_column_list));

			if(c->constraint_check_expr)
				delete_sql_expr(c->constraint_check_expr);

			break;
		}
	}
	free(table_element);
}

void delete_order_by(order_by* o);

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
				case SQL_TABLE :
				{
					for(cy_uint i = 0; i < get_element_count_arraylist(&(ddl->create_table_query.table_elements)); i++)
					{
						sql_table_element* table_element = (sql_table_element*) get_from_front_of_arraylist(&(ddl->create_table_query.table_elements), i);
						delete_table_element(table_element);
					}
					deinitialize_arraylist(&(ddl->create_table_query.table_elements));
					break;
				}
				case SQL_VIEW :
				{
					for(cy_uint i = 0; i < get_element_count_arraylist(&(ddl->create_view_query.column_list)); i++)
					{
						dstring* c = (dstring*) get_from_front_of_arraylist(&(ddl->create_view_query.column_list), i);
						deinit_dstring(c);
						free(c);
					}
					deinitialize_arraylist(&(ddl->create_view_query.column_list));
					delete_dql(ddl->create_view_query.view_query);
					break;
				}
				case SQL_INDEX :
				{
					deinit_dstring(&(ddl->create_index_query.table_name));
					delete_all_and_deinitialize_arraylist_1d(&(ddl->create_index_query.key_exprs), (void(*)(void*))delete_order_by);
					delete_all_and_deinitialize_arraylist_1d(&(ddl->create_index_query.include_exprs), (void(*)(void*))delete_sql_expr);
					if(ddl->create_index_query.where_expr != NULL)
						delete_sql_expr(ddl->create_index_query.where_expr);
					deinit_dstring(&(ddl->create_index_query.using_index_type));
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