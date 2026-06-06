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
			switch(object_type)
			{
				//case SQL_CATALOG :
				case SQL_DATABASE :
				case SQL_SCHEMA :
				{
					init_empty_dstring(&(ddl->alter_rename_only.new_name), 0);
					break;
				}
				case SQL_TABLE :
				{
					init_empty_dstring(&(ddl->alter_table_query.new_name), 0); // initializes both new_name and new_schema
					ddl->alter_table_query.add_table_element = NULL;
					init_empty_dstring(&(ddl->alter_table_query.column_name), 0); // initializes both column_name and constraint_name
					init_empty_dstring(&(ddl->alter_table_query.new_column_name), 0); // initializes both new_column_name and new_constraint_name
					ddl->alter_table_query.new_column_type = NULL;
					ddl->alter_table_query.new_column_default_value = NULL;
					break;
				}
				case SQL_VIEW :
				case SQL_INDEX :
				case SQL_FUNCTION :
				case SQL_PROCEDURE :
				{
					init_empty_dstring(&(ddl->alter_rename_and_set_schema_query.new_name), 0); // initializes new schema also
					break;
				}
				default:
				{
					break;
				}
			}
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

void snprint_table_element(dstring* str_p, const sql_table_element* te_p)
{
	switch(te_p->type)
	{
		case SQL_COLUMN :
		{
			const sql_column_def* c = &(te_p->column_def);

			snprintf_dstring(str_p, " COLUMN ");

			concatenate_dstring(str_p, &(c->column_name));
			snprintf_dstring(str_p, " ");
			snprint_sql_type(str_p, c->type);
			snprintf_dstring(str_p, " ");
			if(c->is_not_null)
			{
				if(!is_empty_dstring(&(c->is_not_null_constraint_name)))
				{
					snprintf_dstring(str_p, "( ");
					concatenate_dstring(str_p, &(c->is_not_null_constraint_name));
					snprintf_dstring(str_p, " )");
				}
				snprintf_dstring(str_p, " NOT NULL ");
			}
			if(c->is_primary_key)
			{
				if(!is_empty_dstring(&(c->is_primary_key_constraint_name)))
				{
					snprintf_dstring(str_p, "( ");
					concatenate_dstring(str_p, &(c->is_primary_key_constraint_name));
					snprintf_dstring(str_p, " )");
				}
				snprintf_dstring(str_p, " PRIMARY KEY ");
			}
			if(c->is_unique)
			{
				if(!is_empty_dstring(&(c->is_unique_constraint_name)))
				{
					snprintf_dstring(str_p, "( ");
					concatenate_dstring(str_p, &(c->is_unique_constraint_name));
					snprintf_dstring(str_p, " )");
				}
				snprintf_dstring(str_p, " UNIQUE ");
			}
			if(c->is_foreign_key)
			{
				if(!is_empty_dstring(&(c->is_foreign_key_constraint_name)))
				{
					snprintf_dstring(str_p, "( ");
					concatenate_dstring(str_p, &(c->is_foreign_key_constraint_name));
					snprintf_dstring(str_p, " )");
				}
				snprintf_dstring(str_p, " FOREIGN_REFERENCE ");
				concatenate_dstring(str_p, &(c->foreign_table));
				if(!is_empty_dstring(&(c->foreign_column)))
				{
					snprintf_dstring(str_p, "(");
					concatenate_dstring(str_p, &(c->foreign_column));
					snprintf_dstring(str_p, ")");
				}
				snprintf_dstring(str_p, " ");
			}
			if(c->default_value)
			{
				snprintf_dstring(str_p, " DEFAULT ");
				snprint_sql_expr(str_p, c->default_value);
				snprintf_dstring(str_p, " ");
			}
			for(cy_uint i = 0; i < get_element_count_arraylist(&(c->constraint_check_exprs)); i++)
			{
				const dstring* name = get_from_front_of_arraylist(&(c->constraint_check_names), i);
				if(!is_empty_dstring(name))
				{
					snprintf_dstring(str_p, "( ");
					printf_dstring(name);
					snprintf_dstring(str_p, " )");
				}
				snprintf_dstring(str_p, " CHECK ( ");
				snprint_sql_expr(str_p, get_from_front_of_arraylist(&(c->constraint_check_exprs), i));
				snprintf_dstring(str_p, " )");
			}
			break;
		}
		case SQL_CONSTRAINT :
		{
			const sql_constraint_def* c = &(te_p->constraint_def);

			snprintf_dstring(str_p, " CONSTRAINT ");

			concatenate_dstring(str_p, &(c->constraint_name));

			switch(c->type)
			{
				case SQL_UNIQUE_KEY :
				{
					snprintf_dstring(str_p, " UNIQUE_KEY_CONSTRAINT ");
					concatenate_dstring(str_p, &(c->constraint_name));
					snprintf_dstring(str_p, " ( ");
					for(cy_uint i = 0; i < get_element_count_arraylist(&(c->column_list)); i++)
					{
						if(i != 0)
							snprintf_dstring(str_p, " , ");
						concatenate_dstring(str_p, (const dstring*)get_from_front_of_arraylist(&(c->column_list), i));
					}
					snprintf_dstring(str_p, " )");
					break;
				}
				case SQL_PRIMARY_KEY :
				{
					snprintf_dstring(str_p, " PRIMARY_KEY_CONSTRAINT ");
					concatenate_dstring(str_p, &(c->constraint_name));
					snprintf_dstring(str_p, " ( ");
					for(cy_uint i = 0; i < get_element_count_arraylist(&(c->column_list)); i++)
					{
						if(i != 0)
							snprintf_dstring(str_p, " , ");
						concatenate_dstring(str_p, (const dstring*)get_from_front_of_arraylist(&(c->column_list), i));
					}
					snprintf_dstring(str_p, " )");
					break;
				}
				case SQL_FOREIGN_KEY :
				{
					snprintf_dstring(str_p, " FOREIGN_KEY_CONSTRAINT ");
					concatenate_dstring(str_p, &(c->constraint_name));
					snprintf_dstring(str_p, " ( ");
					for(cy_uint i = 0; i < get_element_count_arraylist(&(c->column_list)); i++)
					{
						if(i != 0)
							snprintf_dstring(str_p, " , ");
						concatenate_dstring(str_p, (const dstring*)get_from_front_of_arraylist(&(c->column_list), i));
					}
					snprintf_dstring(str_p, " ) REFERENCES ");
					concatenate_dstring(str_p, &(c->foreign_table));
					snprintf_dstring(str_p, " ( ");
					for(cy_uint i = 0; i < get_element_count_arraylist(&(c->foreign_column_list)); i++)
					{
						if(i != 0)
							snprintf_dstring(str_p, " , ");
						concatenate_dstring(str_p, (const dstring*)get_from_front_of_arraylist(&(c->foreign_column_list), i));
					}
					snprintf_dstring(str_p, " )");
					break;
				}
				case SQL_CONSTRAINT_CHECK :
				{
					snprintf_dstring(str_p, " CHECK_CONSTRAINT ");
					concatenate_dstring(str_p, &(c->constraint_name));
					snprintf_dstring(str_p, " ( ");
					snprint_sql_expr(str_p, c->constraint_check_expr);
					snprintf_dstring(str_p, " )");
					break;
				}
			}

			break;
		}
	}
}

void snprint_ddl(dstring* str_p, const sql_ddl* ddl)
{
	switch(ddl->type)
	{
		case CREATE_QUERY :
		{
			snprintf_dstring(str_p, "CREATE");
			break;
		}
		case ALTER_QUERY :
		{
			snprintf_dstring(str_p, "ALTER");
			break;
		}
		case DROP_QUERY :
		{
			snprintf_dstring(str_p, "DROP");
			break;
		}
		case TRUNCATE_QUERY :
		{
			snprintf_dstring(str_p, "TRUNCATE");
			break;
		}
	}

	snprintf_dstring(str_p, " ");

	switch(ddl->object_type)
	{
		//case SQL_CATALOG :
		case SQL_DATABASE :
		{
			snprintf_dstring(str_p, "DATABASE/CATALOG");
			break;
		}
		case SQL_SCHEMA :
		{
			snprintf_dstring(str_p, "SCHEMA");
			break;
		}
		case SQL_TABLE :
		{
			snprintf_dstring(str_p, "TABLE");
			break;
		}
		case SQL_VIEW :
		{
			snprintf_dstring(str_p, "VIEW");
			break;
		}
		case SQL_INDEX :
		{
			snprintf_dstring(str_p, "INDEX");
			break;
		}
		case SQL_FUNCTION :
		{
			snprintf_dstring(str_p, "FUNCTION");
			break;
		}
		case SQL_PROCEDURE :
		{
			snprintf_dstring(str_p, "PROCEDURE");
			break;
		}
		case SQL_TYPE :
		{
			snprintf_dstring(str_p, "TYPE");
			break;
		}
		case SQL_DOMAIN :
		{
			snprintf_dstring(str_p, "DOMAIN");
			break;
		}
		case SQL_SEQUENCE :
		{
			snprintf_dstring(str_p, "SEQUENCE");
			break;
		}
		case SQL_TRIGGER :
		{
			snprintf_dstring(str_p, "TRIGGER");
			break;
		}
	}

	snprintf_dstring(str_p, "( ");

	int clauses_printed = 0;

	{
		if(clauses_printed != 0)
			snprintf_dstring(str_p, " , ");
		snprintf_dstring(str_p, "name = ");
		concatenate_dstring(str_p, &(ddl->object_name));
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
							snprintf_dstring(str_p, " , ");
						snprintf_dstring(str_p, "authorization = ");
						concatenate_dstring(str_p, &(ddl->create_schema_query.authorization));
						clauses_printed++;
					}
					break;
				}
				case SQL_TABLE :
				{
					{
						if(clauses_printed != 0)
							snprintf_dstring(str_p, " , ");
						snprintf_dstring(str_p, "table_elements = ( ");
						for(cy_uint i = 0; i < get_element_count_arraylist(&(ddl->create_table_query.table_elements)); i++)
						{
							if(i != 0)
								snprintf_dstring(str_p, " , ");
							const sql_table_element* te_p = get_from_front_of_arraylist(&(ddl->create_table_query.table_elements), i);
							snprint_table_element(str_p, te_p);
						}
						snprintf_dstring(str_p, " )");
						clauses_printed++;
					}
					break;
				}
				case SQL_VIEW :
				{
					if(get_element_count_arraylist(&(ddl->create_view_query.column_list)) > 0)
					{
						if(clauses_printed != 0)
							snprintf_dstring(str_p, " , ");
						snprintf_dstring(str_p, "column_list = ( ");
						for(cy_uint i = 0; i < get_element_count_arraylist(&(ddl->create_view_query.column_list)); i++)
						{
							if(i != 0)
								snprintf_dstring(str_p, " , ");
							concatenate_dstring(str_p, (const dstring*)get_from_front_of_arraylist(&(ddl->create_view_query.column_list), i));
						}
						snprintf_dstring(str_p, " )");
						clauses_printed++;
					}
					{
						if(clauses_printed != 0)
							snprintf_dstring(str_p, " , ");
						snprintf_dstring(str_p, "view_query = ( ");
						snprint_dql(str_p, ddl->create_view_query.view_query);
						snprintf_dstring(str_p, " )");
						clauses_printed++;
					}
					if(ddl->create_view_query.check_option != CHECK_OPTION_NONE)
					{
						if(clauses_printed != 0)
							snprintf_dstring(str_p, " , ");
						snprintf_dstring(str_p, "check_option = ");
						switch(ddl->create_view_query.check_option)
						{
							case CHECK_OPTION_NONE :
							{
								snprintf_dstring(str_p, "NONE");
								break;
							}
							case CHECK_OPTION_LOCAL :
							{
								snprintf_dstring(str_p, "LOCAL");
								break;
							}
							case CHECK_OPTION_CASCADED :
							{
								snprintf_dstring(str_p, "CASCADED");
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
							snprintf_dstring(str_p, " , ");
						snprintf_dstring(str_p, "is_unique = %d", !!(ddl->create_index_query.is_unique));
						clauses_printed++;
					}
					{
						if(clauses_printed != 0)
							snprintf_dstring(str_p, " , ");
						snprintf_dstring(str_p, "on_table = ");
						concatenate_dstring(str_p, &(ddl->create_index_query.table_name));
						clauses_printed++;
					}
					{
						if(clauses_printed != 0)
							snprintf_dstring(str_p, " , ");
						snprintf_dstring(str_p, "keys = ( ");
						for(cy_uint i = 0; i < get_element_count_arraylist(&(ddl->create_index_query.key_exprs)); i++)
						{
							if(i != 0)
								snprintf_dstring(str_p, " , ");
							index_key_expr* k = (index_key_expr*) get_from_front_of_arraylist(&(ddl->create_index_query.key_exprs), i);
							snprintf_dstring(str_p, "( ( ");
							snprint_sql_expr(str_p, k->ordering_expr);
							snprintf_dstring(str_p, " ) in %s order )", ((k->dir == ORDER_BY_ASC) ? "ascending" : "descending"));
						}
						snprintf_dstring(str_p, " )");
						clauses_printed++;
					}
					if(get_element_count_arraylist(&(ddl->create_index_query.include_exprs)) > 0)
					{
						if(clauses_printed != 0)
							snprintf_dstring(str_p, " , ");
						snprintf_dstring(str_p, "include = ( ");
						for(cy_uint i = 0; i < get_element_count_arraylist(&(ddl->create_index_query.include_exprs)); i++)
						{
							if(i != 0)
								snprintf_dstring(str_p, " , ");
							sql_expression* x = (sql_expression*) get_from_front_of_arraylist(&(ddl->create_index_query.include_exprs), i);
							snprintf_dstring(str_p, "( ");
							snprint_sql_expr(str_p, x);
							snprintf_dstring(str_p, " )");
						}
						snprintf_dstring(str_p, " )");
						clauses_printed++;
					}
					if(ddl->create_index_query.where_expr != NULL)
					{
						if(clauses_printed != 0)
							snprintf_dstring(str_p, " , ");
						snprintf_dstring(str_p, "where = ( ");
						snprint_sql_expr(str_p, ddl->create_index_query.where_expr);
						snprintf_dstring(str_p, " )");
						clauses_printed++;
					}
					if(!is_empty_dstring(&(ddl->create_index_query.using_index_type)))
					{
						if(clauses_printed != 0)
							snprintf_dstring(str_p, " , ");
						snprintf_dstring(str_p, "using_index_type = ");
						concatenate_dstring(str_p, &(ddl->create_index_query.using_index_type));
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
			switch(ddl->object_type)
			{
				//case SQL_CATALOG :
				case SQL_DATABASE :
				case SQL_SCHEMA :
				{
					{
						if(clauses_printed != 0)
							snprintf_dstring(str_p, " , ");
						snprintf_dstring(str_p, "rename = ");
						concatenate_dstring(str_p, &(ddl->alter_rename_only.new_name));
						clauses_printed++;
					}
					break;
				}
				case SQL_TABLE :
				{
					switch(ddl->alter_table_query.type)
					{
						case SQL_ALTER_RENAME :
						{
							{
								if(clauses_printed != 0)
									snprintf_dstring(str_p, " , ");
								snprintf_dstring(str_p, "rename = ");
								concatenate_dstring(str_p, &(ddl->alter_table_query.new_name));
								clauses_printed++;
							}
							break;
						}
						case SQL_ALTER_SET_SCHEMA :
						{
							{
								if(clauses_printed != 0)
									snprintf_dstring(str_p, " , ");
								snprintf_dstring(str_p, "set_schema = ");
								concatenate_dstring(str_p, &(ddl->alter_table_query.new_schema));
								clauses_printed++;
							}
							break;
						}
						case SQL_ALTER_TABLE_ADD_COLUMN :
						{
							{
								if(clauses_printed != 0)
									snprintf_dstring(str_p, " , ");
								snprintf_dstring(str_p, "add_column = ");
								snprint_table_element(str_p, ddl->alter_table_query.add_table_element);
								clauses_printed++;
							}
							break;
						}
						case SQL_ALTER_TABLE_ADD_CONSTRAINT :
						{
							{
								if(clauses_printed != 0)
									snprintf_dstring(str_p, " , ");
								snprintf_dstring(str_p, "add_constraint = ");
								snprint_table_element(str_p, ddl->alter_table_query.add_table_element);
								clauses_printed++;
							}
							break;
						}
						case SQL_ALTER_TABLE_SET_COLUMN_DEFAULT :
						{
							{
								if(clauses_printed != 0)
									snprintf_dstring(str_p, " , ");
								snprintf_dstring(str_p, "for_column = ");
								concatenate_dstring(str_p, &(ddl->alter_table_query.column_name));
								clauses_printed++;
							}
							{
								if(clauses_printed != 0)
									snprintf_dstring(str_p, " , ");
								snprintf_dstring(str_p, "new_default_value = ( ");
								snprint_sql_expr(str_p, ddl->alter_table_query.new_column_default_value);
								snprintf_dstring(str_p, " )");
								clauses_printed++;
							}
							break;
						}
						case SQL_ALTER_TABLE_DROP_COLUMN_DEFAULT :
						{
							{
								if(clauses_printed != 0)
									snprintf_dstring(str_p, " , ");
								snprintf_dstring(str_p, "drop_default for_column = ");
								concatenate_dstring(str_p, &(ddl->alter_table_query.column_name));
								clauses_printed++;
							}
							break;
						}
						case SQL_ALTER_TABLE_SET_COLUMN_NOT_NULL :
						{
							{
								if(clauses_printed != 0)
									snprintf_dstring(str_p, " , ");
								snprintf_dstring(str_p, "set_not_null for_column = ");
								concatenate_dstring(str_p, &(ddl->alter_table_query.column_name));
								clauses_printed++;
							}
							break;
						}
						case SQL_ALTER_TABLE_DROP_COLUMN_NOT_NULL :
						{
							{
								if(clauses_printed != 0)
									snprintf_dstring(str_p, " , ");
								snprintf_dstring(str_p, "drop_not_null for_column = ");
								concatenate_dstring(str_p, &(ddl->alter_table_query.column_name));
								clauses_printed++;
							}
							break;
						}
						case SQL_ALTER_TABLE_SET_COLUMN_TYPE :
						{
							{
								if(clauses_printed != 0)
									snprintf_dstring(str_p, " , ");
								snprintf_dstring(str_p, "for_column = ");
								concatenate_dstring(str_p, &(ddl->alter_table_query.column_name));
								clauses_printed++;
							}
							{
								if(clauses_printed != 0)
									snprintf_dstring(str_p, " , ");
								snprintf_dstring(str_p, "new_type = ");
								snprint_sql_type(str_p, ddl->alter_table_query.new_column_type);
								clauses_printed++;
							}
							break;
						}
						case SQL_ALTER_TABLE_RENAME_COLUMN :
						{
							{
								if(clauses_printed != 0)
									snprintf_dstring(str_p, " , ");
								snprintf_dstring(str_p, "rename_column = ");
								concatenate_dstring(str_p, &(ddl->alter_table_query.column_name));
								clauses_printed++;
							}
							{
								if(clauses_printed != 0)
									snprintf_dstring(str_p, " , ");
								snprintf_dstring(str_p, "new_name = ");
								concatenate_dstring(str_p, &(ddl->alter_table_query.new_column_name));
								clauses_printed++;
							}
							break;
						}
						case SQL_ALTER_TABLE_RENAME_CONSTRAINT :
						{
							{
								if(clauses_printed != 0)
									snprintf_dstring(str_p, " , ");
								snprintf_dstring(str_p, "rename_constraint = ");
								concatenate_dstring(str_p, &(ddl->alter_table_query.constraint_name));
								clauses_printed++;
							}
							{
								if(clauses_printed != 0)
									snprintf_dstring(str_p, " , ");
								snprintf_dstring(str_p, "new_name = ");
								concatenate_dstring(str_p, &(ddl->alter_table_query.new_constraint_name));
								clauses_printed++;
							}
							break;
						}
						case SQL_ALTER_TABLE_DROP_COLUMN :
						{
							{
								if(clauses_printed != 0)
									snprintf_dstring(str_p, " , ");
								snprintf_dstring(str_p, "drop_column = ");
								concatenate_dstring(str_p, &(ddl->alter_table_query.column_name));
								clauses_printed++;
							}
							{
								if(clauses_printed != 0)
									snprintf_dstring(str_p, " , ");
								switch(ddl->drop_behavior)
								{
									case DROP_RESTRICT :
									{
										snprintf_dstring(str_p, "RESTRICT");
										break;
									}
									case DROP_CASCADE :
									{
										snprintf_dstring(str_p, "CASCADE");
										break;
									}
								}
								clauses_printed++;
							}
							break;
						}
						case SQL_ALTER_TABLE_DROP_CONSTRAINT :
						{
							{
								if(clauses_printed != 0)
									snprintf_dstring(str_p, " , ");
								snprintf_dstring(str_p, "drop_constraint = ");
								concatenate_dstring(str_p, &(ddl->alter_table_query.constraint_name));
								clauses_printed++;
							}
							{
								if(clauses_printed != 0)
									snprintf_dstring(str_p, " , ");
								switch(ddl->drop_behavior)
								{
									case DROP_RESTRICT :
									{
										snprintf_dstring(str_p, "RESTRICT");
										break;
									}
									case DROP_CASCADE :
									{
										snprintf_dstring(str_p, "CASCADE");
										break;
									}
								}
								clauses_printed++;
							}
							break;
						}
					}
					break;
				}
				case SQL_VIEW :
				case SQL_INDEX :
				case SQL_FUNCTION :
				case SQL_PROCEDURE :
				{
					switch(ddl->alter_rename_and_set_schema_query.type)
					{
						case SQL_ALTER_RENAME :
						{
							{
								if(clauses_printed != 0)
									snprintf_dstring(str_p, " , ");
								snprintf_dstring(str_p, "rename = ");
								concatenate_dstring(str_p, &(ddl->alter_rename_and_set_schema_query.new_name));
								clauses_printed++;
							}
							break;
						}
						case SQL_ALTER_SET_SCHEMA :
						{
							{
								if(clauses_printed != 0)
									snprintf_dstring(str_p, " , ");
								snprintf_dstring(str_p, "set_schema = ");
								concatenate_dstring(str_p, &(ddl->alter_rename_and_set_schema_query.new_schema));
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
				default :
				{
					break;
				}
			}
			break;
		}
		case DROP_QUERY :
		{
			{
				if(clauses_printed != 0)
					snprintf_dstring(str_p, " , ");
				switch(ddl->drop_behavior)
				{
					case DROP_RESTRICT :
					{
						snprintf_dstring(str_p, "RESTRICT");
						break;
					}
					case DROP_CASCADE :
					{
						snprintf_dstring(str_p, "CASCADE");
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

	snprintf_dstring(str_p, " )");
}

void delete_dstring(dstring* d);

void delete_table_element(sql_table_element* table_element)
{
	switch(table_element->type)
	{
		case SQL_COLUMN :
		{
			sql_column_def* c = &(table_element->column_def);

			deinit_dstring(&(c->column_name));
			delete_sql_type(c->type);
			deinit_dstring(&(c->foreign_table));
			deinit_dstring(&(c->foreign_column));
			if(c->default_value)
				delete_sql_expr(c->default_value);
			for(cy_uint i = 0; i < get_element_count_arraylist(&(c->constraint_check_exprs)); i++)
				delete_sql_expr((sql_expression*) get_from_front_of_arraylist(&(c->constraint_check_exprs), i));
			deinitialize_arraylist(&(c->constraint_check_exprs));

			deinit_dstring(&(c->is_not_null_constraint_name));
			deinit_dstring(&(c->is_primary_key_constraint_name));
			deinit_dstring(&(c->is_unique_constraint_name));
			deinit_dstring(&(c->is_foreign_key_constraint_name));
			delete_all_and_deinitialize_arraylist_1d(&(c->constraint_check_names), (void(*)(void*))delete_dstring);

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
			switch(ddl->object_type)
			{
				//case SQL_CATALOG :
				case SQL_DATABASE :
				case SQL_SCHEMA :
				{
					deinit_dstring(&(ddl->alter_rename_only.new_name));
					break;
				}
				case SQL_TABLE :
				{
					deinit_dstring(&(ddl->alter_table_query.new_name));
					if(ddl->alter_table_query.add_table_element)
						delete_table_element(ddl->alter_table_query.add_table_element);
					deinit_dstring(&(ddl->alter_table_query.column_name));
					deinit_dstring(&(ddl->alter_table_query.new_column_name));
					if(ddl->alter_table_query.new_column_type != NULL)
						delete_sql_type(ddl->alter_table_query.new_column_type);
					if(ddl->alter_table_query.new_column_default_value != NULL)
						delete_sql_expr(ddl->alter_table_query.new_column_default_value);
					break;
				}
				case SQL_VIEW :
				case SQL_INDEX :
				case SQL_FUNCTION :
				case SQL_PROCEDURE :
				{
					switch(ddl->alter_rename_and_set_schema_query.type)
					{
						case SQL_ALTER_RENAME :
						{
							deinit_dstring(&(ddl->alter_rename_and_set_schema_query.new_name));
							break;
						}
						case SQL_ALTER_SET_SCHEMA :
						{
							deinit_dstring(&(ddl->alter_rename_and_set_schema_query.new_schema));
							break;
						}
						default :
						{
							break;
						}
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