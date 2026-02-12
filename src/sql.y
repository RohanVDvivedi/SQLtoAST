%{
#include<stdio.h>
#include<stdlib.h>
#include<string.h>

#include<sqltoast/sqltoast.h>

// destructor functions only needed here and nowhere else
void delete_columns_to_be_set(columns_to_be_set* c);
void delete_projection(projection* p);
void destroy_relation_input(relation_input* ri_p);
void delete_join_with(join_with* j);
void delete_order_by(order_by* o);
%}

%code requires {
	#include<sqltoast/sqltoast.h>
}

/* Enable reentrant parser */
%define api.pure full

/* make all external symbol use sql as prefix */
%define api.prefix {sql}

%lex-param   { void* scanner }

%parse-param { void* scanner }
%parse-param { struct sql** sql_ast }

%start sql_query

%union {
	sql* sql_query;

	sql_dql* dql_query;

	sql_dml* dml_query;

	columns_to_be_set* attribute_assignment;

	sql_tcl* tcl_cmd;

	projection* projection;

	join_with* join_with;

	order_by* order_by;

	sql_expression* expr;

	arraylist ptr_list;

	arraylist ptr_lists[2]; // useful for list of when and then statements

	relation_input rel_input;

	sql_type data_type;

	uint64_t uval;
	int64_t ival;

	uint64_t uvals[5];

	dstring sval; // for any other lexeme
}
/* standard destructors */
%destructor { delete_sql($$); } <sql_query>
%destructor { delete_dql($$); } <dql_query>
%destructor { delete_dml($$); } <dml_query>
%destructor { delete_columns_to_be_set($$); } <attribute_assignment>
%destructor { delete_tcl($$); } <tcl_cmd>
%destructor { delete_projection($$); } <projection>
%destructor { delete_join_with($$); } <join_with>
%destructor { delete_order_by($$); } <order_by>
%destructor { delete_sql_expr($$); } <expr>
%destructor { destroy_relation_input(&($$)); } <rel_input>
%destructor { deinit_dstring(&($$)); } <sval>

/* SQL */
%type <sql_query> sql_query

/* DQL query */
%type <dql_query> dql_query

/* SELECT query */
%type <dql_query> select_query
%type <ptr_list> projection_list
%type <projection> projection
%type <rel_input> rel_input
%type <ptr_list> join_clauses
%type <join_with> join_clause
%type <expr> where_clause
%type <ptr_list> group_by_clause
%type <expr> having_clause
%type <ptr_list> order_by_clause
%type <ptr_list> order_by_expr_and_dir_list
%type <order_by> order_by_expr_and_dir
%type <expr> offset_clause
%type <expr> limit_clause

%type <dql_query> values_query
%type <ptr_list> values_rows_list

%type <uval> set_op_mod

%token INTERSECT
%token UNION
%token EXCEPT
%token DISTINCT

%token VALUES

%token SELECT
%token UNIQUE
%token FROM
%token JOIN
%token ON
%token WHERE
%token GROUP
%token BY
%token HAVING
%token ORDER
%token LIMIT
%token OFFSET

%token USING
%token INNER
%token OUTER
%token LEFT
%token RIGHT
%token FULL
%token CROSS
%token LATERAL
%token NATURAL

%type <uval> projection_all_or_distinct

%type <uval> lateral_opt
%type <uval> join_type
%type <ptr_list> identifier_list

%token ASC
%token DESC

/* DML query */
%type <dml_query> dml_query

/* INSERT query */
%type <dml_query> insert_query

%token INSERT
%token INTO

/* UPDATE query */
%type <dml_query> update_query
%type <ptr_list> set_clause
%type <ptr_list> attribute_assignment_list
%type <attribute_assignment> attribute_assignment

%token UPDATE
%token DEFAULT

%type <ptr_list> defaultable_expr_list
%type <expr> defaultable_expr

/* DELETE query */
%type <dml_query> delete_query

%token DELETE

/* TCL command */
%type <tcl_cmd> tcl_cmd

%token START
%token _BEGIN_
%token TRANSACTION
%token ROLLBACK
%token COMMIT
%token RELEASE
%token TO
%token SAVEPOINT
%token WORK
%token SET
%token CHARACTERISTICS

%token READ
%token WRITE
%token ONLY
%token ISOLATION_LEVEL
%token UNCOMMITTED
%token COMMITTED
%token REPEATABLE
%token SERIALIZABLE

%type <uvals> tx_mods
%type <uval> access_mode
%type <uval> isolation_level

/* SQL EXPRESSION */
%type <expr> expr
%type <ptr_list> expr_list
%type <expr> bool_literal
%type <expr> func_expr
%type <expr> sub_query_expr
%type <ptr_lists> when_then_lists
%type <expr> else_opt

%type <uval> aggregate_distinct_opt

/* SQL DATA TYPE */
%type <data_type> type
%type <data_type> type_name
%type <data_type> type_specs
%type <uval> type_spec
%type <data_type> type_with_or_without_timezone

%token <sval> IDENTIFIER
%token <sval> INTEGER
%token <sval> NUMBER
%token <sval> STRING

%token TRUE
%token FALSE
%token UNKNOWN
%token _NULL_

%token OPEN_BRACKET
%token CLOSE_BRACKET

%token NEG
%token B_NOT
%token L_NOT

%token ADD
%token MUL
%token DIV
%token MOD

%token GT
%token GTE
%token LT
%token LTE
%token EQ
%token NEQ

%type <uval> cmp_rhs_quantifier

%token ANY
%token ALL

%token B_AND
%token B_OR
%token B_XOR

%token L_AND
%token L_OR
%token L_XOR

%token L_SHIFT
%token R_SHIFT

%token CONCAT
%token LIKE

%token IS

%token BETWEEN

%token IN
%token COMMA

%token EXISTS

%token CAST
%token AS

%token CASE
%token WHEN
%token THEN
%token ELSE
%token END

/* sql type names and needed tokens */

%token BOOL

%token BIT

%token SMALLINT
%token INT
%token BIGINT

%token REAL
%token DOUBLE
%token FLOAT

%token DECIMAL
%token NUMERIC

%token TEXT
%token CHAR
%token VARCHAR
%token CLOB
%token BLOB

%token DATE
%token TIME
%token TIMESTAMP

%token WITH_TZ
%token WITHOUT_TZ

/* Precedence + Associativity */

/* Lowest precedence first (so Bison gives them the lowest binding) */

%left UNION EXCEPT
%left INTERSECT

%left L_OR
%left L_XOR
%left L_AND
%right L_NOT

%nonassoc EXISTS
%nonassoc EXISTS_PREC

%nonassoc IS
%nonassoc IS_PREC
%nonassoc BETWEEN
%nonassoc BETWEEN_PREC
%nonassoc IN
%nonassoc IN_PREC
%left EQ NEQ GT GTE LT LTE LIKE LIKE_PREC

%left B_OR
%left B_XOR
%left B_AND
%left L_SHIFT R_SHIFT

%left CONCAT

%left ADD NEG
%left MUL DIV MOD

%right UMINUS UPLUS
%right B_NOT

%%

sql_query :
			dql_query 							{$$ = malloc(sizeof(sql)); $$->type = DQL; $$->dql_query = $1; (*sql_ast) = $$;}
			| dml_query 						{$$ = malloc(sizeof(sql)); $$->type = DML; $$->dml_query = $1; (*sql_ast) = $$;}
			| tcl_cmd 							{$$ = malloc(sizeof(sql)); $$->type = TCL; $$->tcl_cmd = $1; (*sql_ast) = $$;}

tcl_cmd :
			start_tx_keywords tx_mods							{$$ = new_tcl(START_TX_TCL_CMD); $$->isolation_level = $2[0]; $$->mode = $2[1];}
			| COMMIT work_opt									{$$ = new_tcl(COMMIT_TCL_CMD);}
			| ROLLBACK work_opt									{$$ = new_tcl(ROLLBACK_TCL_CMD);}
			| SAVEPOINT IDENTIFIER								{$$ = new_tcl(SAVEPOINT_TCL_CMD); $$->savepoint_name = $2;}
			| RELEASE SAVEPOINT IDENTIFIER						{$$ = new_tcl(RELEASE_SAVEPOINT_TCL_CMD); $$->savepoint_name = $3;}
			| ROLLBACK work_opt TO SAVEPOINT IDENTIFIER			{$$ = new_tcl(ROLLBACK_TO_SAVEPOINT_TCL_CMD); $$->savepoint_name = $5;}
			| SET TRANSACTION tx_mods							{$$ = new_tcl(SET_TX_TCL_CMD); $$->isolation_level = $3[0]; $$->mode = $3[1];}
			| SET TRANSACTION CHARACTERISTICS tx_mods			{$$ = new_tcl(SET_TX_CHARACTERISTICS_TCL_CMD); $$->isolation_level = $4[0]; $$->mode = $4[1];}

start_tx_keywords :
			START TRANSACTION 				{}
			| _BEGIN_						{}
			| _BEGIN_ TRANSACTION 			{}

tx_mods :
		access_mode 								{$$[0] = ISO_UNSPECIFIED; $$[1] = $1;}
		| isolation_level 							{$$[0] = $1; $$[1] = TX_ACC_RW_UNSPECIFIED;}
		| access_mode COMMA isolation_level		 	{$$[0] = $3; $$[1] = $1;}
		| isolation_level COMMA access_mode 		{$$[0] = $1; $$[1] = $3;}

access_mode :
				READ WRITE 				{$$ = TX_ACC_RW_READ_WRITE;}
				| READ ONLY 			{$$ = TX_ACC_RW_READ_ONLY;}

isolation_level :
				ISOLATION_LEVEL READ UNCOMMITTED 		{$$ = ISO_READ_UNCOMMITTED;}
				| ISOLATION_LEVEL READ COMMITTED 		{$$ = ISO_READ_COMMITTED;}
				| ISOLATION_LEVEL REPEATABLE READ  		{$$ = ISO_REPEATABLE_READ;}
				| ISOLATION_LEVEL SERIALIZABLE 			{$$ = ISO_SERIALIZABLE;}

work_opt :
						{}
			| WORK 		{}

dml_query :
			insert_query 			{$$ = $1;}
			| update_query 			{$$ = $1;}
			| delete_query 			{$$ = $1;}

insert_query :
			INSERT INTO IDENTIFIER dql_query														{$$ = new_dml(INSERT_QUERY); $$->insert_query.table_name = $3; $$->insert_query.input_data_query = $4;}
			| INSERT INTO IDENTIFIER OPEN_BRACKET identifier_list CLOSE_BRACKET dql_query			{$$ = new_dml(INSERT_QUERY); $$->insert_query.table_name = $3; $$->insert_query.column_name_list = $5; $$->insert_query.input_data_query = $7;}

update_query :
			UPDATE IDENTIFIER set_clause where_clause 	{$$ = new_dml(UPDATE_QUERY); $$->update_query.table_name = $2; $$->update_query.values_to_be_set = $3; $$->update_query.where_expr = $4;}

set_clause :
			SET attribute_assignment_list 			{$$ = $2;}

attribute_assignment_list :
			attribute_assignment 											{initialize_arraylist(&($$), 8); push_back_to_arraylist(&($$), $1);}
			| attribute_assignment_list COMMA attribute_assignment 			{if(is_full_arraylist(&($1)) && !expand_arraylist(&($1))) exit(-1); push_back_to_arraylist(&($1), $3); $$ = $1;}

attribute_assignment :
			IDENTIFIER EQ defaultable_expr 																		{$$ = new_columns_to_be_set(1); dstring* t = malloc(sizeof(dstring)); (*t) = $1; push_back_to_arraylist(&($$->column_names), t); push_back_to_arraylist(&($$->value_exprs), $3);}
			| OPEN_BRACKET identifier_list CLOSE_BRACKET EQ OPEN_BRACKET defaultable_expr_list CLOSE_BRACKET 	{$$ = new_columns_to_be_set(0); $$->column_names = $2; $$->value_exprs = $6;}

defaultable_expr_list :
			defaultable_expr 											{initialize_expr_list(&($$)); insert_in_expr_list(&($$), $1);}
			| defaultable_expr_list COMMA defaultable_expr 				{insert_in_expr_list(&($1), $3); $$ = $1;}

defaultable_expr :
			expr 			{$$ = $1;}
			| DEFAULT 		{$$ = NULL;}

delete_query :
			DELETE FROM IDENTIFIER where_clause 		{$$ = new_dml(DELETE_QUERY); $$->delete_query.table_name = $3; $$->delete_query.where_expr = $4;}

dql_query :
			select_query 										{$$ = $1;}
			| values_query 										{$$ = $1;}
			| dql_query INTERSECT set_op_mod dql_query 			{$$ = new_dql(SET_OPERATION); $$->set_operation.op_type = SQL_SET_INTERSECT; $$->set_operation.op_mod = $3; $$->set_operation.left = $1; $$->set_operation.right = $4;}
			| dql_query UNION set_op_mod dql_query				{$$ = new_dql(SET_OPERATION); $$->set_operation.op_type = SQL_SET_UNION; $$->set_operation.op_mod = $3; $$->set_operation.left = $1; $$->set_operation.right = $4;}
			| dql_query EXCEPT set_op_mod dql_query				{$$ = new_dql(SET_OPERATION); $$->set_operation.op_type = SQL_SET_EXCEPT; $$->set_operation.op_mod = $3; $$->set_operation.left = $1; $$->set_operation.right = $4;}
			| OPEN_BRACKET dql_query CLOSE_BRACKET				{$$ = $2;}

values_query :
			VALUES values_rows_list 					{$$ = new_dql(VALUES_QUERY); $$->values_query.values = $2;}

values_rows_list :
			OPEN_BRACKET defaultable_expr_list CLOSE_BRACKET 								{initialize_arraylist(&$$, 1); arraylist* t = malloc(sizeof(arraylist)); (*t) = $2; push_back_to_arraylist(&$$, t);}
			| values_rows_list COMMA OPEN_BRACKET defaultable_expr_list CLOSE_BRACKET 		{if(is_full_arraylist(&($1)) && !expand_arraylist(&($1))) exit(-1); arraylist* t = malloc(sizeof(arraylist)); (*t) = $4; push_back_to_arraylist(&($1), t); $$ = $1;}

set_op_mod :
							{$$ = SQL_RESULT_SET_DISTINCT;}
			| ALL 			{$$ = SQL_RESULT_SET_ALL;}
			| DISTINCT 		{$$ = SQL_RESULT_SET_DISTINCT;}

select_query :
			SELECT projection_all_or_distinct projection_list FROM rel_input join_clauses where_clause group_by_clause having_clause order_by_clause offset_clause limit_clause {
				$$ = new_dql(SELECT_QUERY);
				$$->select_query.projection_mode = $2;
				$$->select_query.projections = $3;
				$$->select_query.base_input = $5;
				$$->select_query.joins_with = $6;
				$$->select_query.where_expr = $7;
				$$->select_query.group_by = $8;
				$$->select_query.having_expr = $9;
				$$->select_query.ordered_by = $10;
				$$->select_query.offset_expr = $11;
				$$->select_query.limit_expr = $12;
			}

projection_all_or_distinct :
														{$$ = SQL_RESULT_SET_ALL;}
								| ALL 					{$$ = SQL_RESULT_SET_ALL;}
								| DISTINCT 				{$$ = SQL_RESULT_SET_DISTINCT;}
								| UNIQUE 				{$$ = SQL_RESULT_SET_DISTINCT;}

projection_list :
					projection 											{initialize_arraylist(&($$), 8); push_back_to_arraylist(&($$), $1);}
					| projection_list COMMA projection  				{if(is_full_arraylist(&($1)) && !expand_arraylist(&($1))) exit(-1); push_back_to_arraylist(&($1), $3); $$ = $1;}

projection :
					expr 													{$$ = malloc(sizeof(projection)); (*$$) = (projection){$1, new_copy_dstring(&get_dstring_pointing_to_cstring(""))};}
					| expr AS IDENTIFIER 									{$$ = malloc(sizeof(projection)); (*$$) = (projection){$1, $3};}
					| expr IDENTIFIER 										{$$ = malloc(sizeof(projection)); (*$$) = (projection){$1, $2};}

rel_input :
			IDENTIFIER 																{$$ = new_relation_input($1, new_copy_dstring(&get_dstring_pointing_to_cstring("")));}
			| IDENTIFIER AS IDENTIFIER												{$$ = new_relation_input($1, $3);}
			| IDENTIFIER IDENTIFIER													{$$ = new_relation_input($1, $2);}
			| OPEN_BRACKET dql_query CLOSE_BRACKET 									{$$ = new_sub_query_relation_input($2, new_copy_dstring(&get_dstring_pointing_to_cstring("")));}
			| OPEN_BRACKET dql_query CLOSE_BRACKET AS IDENTIFIER					{$$ = new_sub_query_relation_input($2, $5);}
			| OPEN_BRACKET dql_query CLOSE_BRACKET IDENTIFIER						{$$ = new_sub_query_relation_input($2, $4);}
			| func_expr 															{$$ = new_function_call_relation_input($1, new_copy_dstring(&get_dstring_pointing_to_cstring("")));}
			| func_expr AS IDENTIFIER												{$$ = new_function_call_relation_input($1, $3);}
			| func_expr IDENTIFIER													{$$ = new_function_call_relation_input($1, $2);}

join_clauses :
												{initialize_arraylist(&($$), 0);}
				| join_clause 					{initialize_arraylist(&($$), 8); push_back_to_arraylist(&($$), $1);}
				| join_clauses join_clause 		{if(is_full_arraylist(&($1)) && !expand_arraylist(&($1))) exit(-1); push_back_to_arraylist(&($1), $2); $$ = $1;}

join_clause : 
				join_type JOIN lateral_opt rel_input ON expr													{$$ = malloc(sizeof(join_with)); $$->type = $1; $$->is_lateral = $3; $$->input = $4; $$->condition_type = ON_EXPR_JOIN_CONDITION; $$->on_expr = $6;}
				| join_type JOIN lateral_opt rel_input USING OPEN_BRACKET identifier_list CLOSE_BRACKET			{$$ = malloc(sizeof(join_with)); $$->type = $1; $$->is_lateral = $3; $$->input = $4; $$->condition_type = USING_JOIN_CONDITION; $$->using_cols = $7;}
				| NATURAL join_type JOIN lateral_opt rel_input													{$$ = malloc(sizeof(join_with)); $$->type = $2; $$->is_lateral = $4; $$->input = $5; $$->condition_type = NATURAL_JOIN_CONDITION;}
				| CROSS JOIN lateral_opt rel_input																{$$ = malloc(sizeof(join_with)); $$->type = CROSS_JOIN; $$->is_lateral = $3; $$->input = $4; $$->condition_type = NO_JOIN_CONDITION;}


join_type :
			 							{$$ = INNER_JOIN;}
			| INNER 					{$$ = INNER_JOIN;}
			| LEFT outer_opt 			{$$ = LEFT_JOIN;}
			| RIGHT outer_opt 			{$$ = RIGHT_JOIN;}
			| FULL outer_opt 			{$$ = FULL_JOIN;}

outer_opt :
							{}
				| OUTER 	{}

lateral_opt :
							{$$ = 0;}
				| LATERAL 	{$$ = 1;}

identifier_list :
				IDENTIFIER 								{dstring* t = malloc(sizeof(dstring)); (*t) = $1; initialize_arraylist(&($$), 8); push_back_to_arraylist(&($$), t);}
				| identifier_list COMMA IDENTIFIER 		{dstring* t = malloc(sizeof(dstring)); (*t) = $3; if(is_full_arraylist(&($1)) && !expand_arraylist(&($1))) exit(-1); push_back_to_arraylist(&($1), t); $$ = $1;}

where_clause :
										{$$ = NULL;}
				| WHERE expr 			{$$ = $2;}

group_by_clause :
										{initialize_arraylist(&($$), 0);}
				| GROUP BY expr_list 	{$$ = $3;}

having_clause :
										{$$ = NULL;}
				| HAVING expr 			{$$ = $2;}

order_by_clause :
															{initialize_arraylist(&($$), 0);}
				| ORDER BY order_by_expr_and_dir_list 		{$$ = $3;}

order_by_expr_and_dir_list :
				order_by_expr_and_dir 											{initialize_arraylist(&($$), 8); push_back_to_arraylist(&($$), $1);}
				| order_by_expr_and_dir_list COMMA order_by_expr_and_dir 		{if(is_full_arraylist(&($1)) && !expand_arraylist(&($1))) exit(-1); push_back_to_arraylist(&($1), $3); $$ = $1;}

order_by_expr_and_dir :
				expr 					{$$ = malloc(sizeof(order_by)); $$->ordering_expr = $1; $$->dir = ORDER_BY_ASC;}
				| expr ASC 				{$$ = malloc(sizeof(order_by)); $$->ordering_expr = $1; $$->dir = ORDER_BY_ASC;}
				| expr DESC 			{$$ = malloc(sizeof(order_by)); $$->ordering_expr = $1; $$->dir = ORDER_BY_DESC;}

offset_clause :
										{$$ = NULL;}
				| OFFSET expr 			{$$ = $2;}

limit_clause :
										{$$ = NULL;}
				| LIMIT expr 			{$$ = $2;}

expr :
			OPEN_BRACKET expr CLOSE_BRACKET 														{$$ = $2;}

			| L_NOT expr 																			{$$ = new_unary_sql_expr(SQL_LOGNOT, $2);}

			| expr EQ expr																			{$$ = new_compare_sql_expr(SQL_EQ, SQL_CMP_NONE, $1, $3);}
			| expr NEQ expr 																		{$$ = new_compare_sql_expr(SQL_NEQ, SQL_CMP_NONE, $1, $3);}
			| expr GT expr 																			{$$ = new_compare_sql_expr(SQL_GT, SQL_CMP_NONE, $1, $3);}
			| expr GTE expr 																		{$$ = new_compare_sql_expr(SQL_GTE, SQL_CMP_NONE, $1, $3);}
			| expr LT expr 																			{$$ = new_compare_sql_expr(SQL_LT, SQL_CMP_NONE, $1, $3);}
			| expr LTE expr 																		{$$ = new_compare_sql_expr(SQL_LTE, SQL_CMP_NONE, $1, $3);}

			| expr EQ cmp_rhs_quantifier OPEN_BRACKET dql_query	CLOSE_BRACKET						{$$ = new_compare_sql_expr(SQL_EQ, $3, $1, $5);}
			| expr NEQ cmp_rhs_quantifier OPEN_BRACKET dql_query CLOSE_BRACKET						{$$ = new_compare_sql_expr(SQL_NEQ, $3, $1, $5);}
			| expr GT cmp_rhs_quantifier OPEN_BRACKET dql_query CLOSE_BRACKET						{$$ = new_compare_sql_expr(SQL_GT, $3, $1, $5);}
			| expr GTE cmp_rhs_quantifier OPEN_BRACKET dql_query CLOSE_BRACKET						{$$ = new_compare_sql_expr(SQL_GTE, $3, $1, $5);}
			| expr LT cmp_rhs_quantifier OPEN_BRACKET dql_query CLOSE_BRACKET						{$$ = new_compare_sql_expr(SQL_LT, $3, $1, $5);}
			| expr LTE cmp_rhs_quantifier OPEN_BRACKET dql_query CLOSE_BRACKET						{$$ = new_compare_sql_expr(SQL_LTE, $3, $1, $5);}

			| expr LIKE expr %prec LIKE_PREC														{$$ = new_binary_sql_expr(SQL_LIKE, $1, $3);}
			| expr L_NOT LIKE expr %prec LIKE_PREC													{$$ = new_unary_sql_expr(SQL_LOGNOT, new_binary_sql_expr(SQL_LIKE, $1, $4));}

			| expr IN OPEN_BRACKET expr_list CLOSE_BRACKET %prec IN_PREC							{$$ = new_in_sql_expr($1, NULL, $4);}
			| expr L_NOT IN OPEN_BRACKET expr_list CLOSE_BRACKET %prec IN_PREC						{$$ = new_unary_sql_expr(SQL_LOGNOT, new_in_sql_expr($1, NULL, $5));}

			| expr IN OPEN_BRACKET dql_query CLOSE_BRACKET %prec IN_PREC							{arraylist t; initialize_arraylist(&t, 0); $$ = new_in_sql_expr($1, $4, t);}
			| expr L_NOT IN OPEN_BRACKET dql_query CLOSE_BRACKET %prec IN_PREC						{arraylist t; initialize_arraylist(&t, 0); $$ = new_unary_sql_expr(SQL_LOGNOT, new_in_sql_expr($1, $5, t));}

			| expr BETWEEN expr L_AND expr %prec BETWEEN_PREC										{$$ = new_between_sql_expr($1, $3, $5);}
			| expr L_NOT BETWEEN expr L_AND expr %prec BETWEEN_PREC									{$$ = new_unary_sql_expr(SQL_LOGNOT, new_between_sql_expr($1, $4, $6));}

			| expr IS bool_literal %prec IS_PREC													{$$ = new_binary_sql_expr(SQL_IS, $1, $3);}
			| expr IS L_NOT bool_literal %prec IS_PREC 												{$$ = new_unary_sql_expr(SQL_LOGNOT, new_binary_sql_expr(SQL_IS, $1, $4));}

			| expr L_AND expr 																		{$$ = new_binary_sql_expr(SQL_LOGAND, $1, $3);}
			| expr L_OR expr 																		{$$ = new_binary_sql_expr(SQL_LOGOR, $1, $3);}
			| expr L_XOR expr 																		{$$ = new_binary_sql_expr(SQL_LOGXOR, $1, $3);}

			| INTEGER 																				{$$ = new_valued_sql_expr(SQL_NUM, $1);}
			| NUMBER																				{$$ = new_valued_sql_expr(SQL_NUM, $1);}
			| STRING																				{$$ = new_valued_sql_expr(SQL_STR, $1);}
			| IDENTIFIER																			{$$ = new_valued_sql_expr(SQL_VAR, $1);}
			| MUL																					{$$ = new_valued_sql_expr(SQL_VAR, new_dstring("*", 1));}
			| TRUE																					{$$ = new_const_non_valued_sql_expr(SQL_TRUE);}
			| FALSE																					{$$ = new_const_non_valued_sql_expr(SQL_FALSE);}
			| _NULL_																				{$$ = new_const_non_valued_sql_expr(SQL_NULL);}
			| UNKNOWN																				{$$ = new_const_non_valued_sql_expr(SQL_UNKNOWN);}

			| B_NOT expr 																			{$$ = new_unary_sql_expr(SQL_BITNOT, $2);}
			| NEG expr %prec UMINUS																	{$$ = new_unary_sql_expr(SQL_NEG, $2);}
			| ADD expr %prec UPLUS																	{$$ = $2;}

			| expr MUL expr 																		{$$ = new_binary_sql_expr(SQL_MUL, $1, $3);}
			| expr DIV expr 																		{$$ = new_binary_sql_expr(SQL_DIV, $1, $3);}
			| expr MOD expr 																		{$$ = new_binary_sql_expr(SQL_MOD, $1, $3);}

			| expr ADD expr 																		{$$ = new_binary_sql_expr(SQL_ADD, $1, $3);}
			| expr NEG expr 																		{$$ = new_binary_sql_expr(SQL_SUB, $1, $3);}

			| expr B_AND expr 																		{$$ = new_binary_sql_expr(SQL_BITAND, $1, $3);}
			| expr B_OR expr 																		{$$ = new_binary_sql_expr(SQL_BITOR, $1, $3);}
			| expr B_XOR expr 																		{$$ = new_binary_sql_expr(SQL_BITXOR, $1, $3);}

			| expr L_SHIFT expr 																	{$$ = new_binary_sql_expr(SQL_LSHIFT, $1, $3);}
			| expr R_SHIFT expr 																	{$$ = new_binary_sql_expr(SQL_RSHIFT, $1, $3);}

			| expr CONCAT expr 																		{$$ = new_binary_sql_expr(SQL_CONCAT, $1, $3);}

			| func_expr																				{$$ = $1;}

			| CAST OPEN_BRACKET expr AS type CLOSE_BRACKET 											{$$ = new_cast_sql_expr($3, $5);}

			| sub_query_expr 																		{$$ = $1;}

			| EXISTS OPEN_BRACKET dql_query CLOSE_BRACKET 											{$$ = new_sub_query_sql_expr(SQL_EXISTS, $3);}

			| CASE      when_then_lists else_opt END 												{$$ = new_case_sql_expr(NULL, $2[0], $2[1], $3);}
			| CASE expr when_then_lists else_opt END 												{$$ = new_case_sql_expr($2, $3[0], $3[1], $4);}

when_then_lists :
			WHEN expr THEN expr 						{initialize_expr_list(&($$[0])); initialize_expr_list(&($$[1])); insert_in_expr_list(&($$[0]), $2); insert_in_expr_list(&($$[1]), $4);}
			| when_then_lists WHEN expr THEN expr 		{insert_in_expr_list(&($$[0]), $3); insert_in_expr_list(&($$[1]), $5);}

else_opt :
								{$$ = NULL;}
			| ELSE expr 		{$$ = $2;}

cmp_rhs_quantifier :
			ANY 					{$$ = SQL_CMP_ANY;}
			| ALL 					{$$ = SQL_CMP_ALL;}

bool_literal:
			TRUE				{$$ = new_const_non_valued_sql_expr(SQL_TRUE);}
			| FALSE				{$$ = new_const_non_valued_sql_expr(SQL_FALSE);}
			| _NULL_			{$$ = new_const_non_valued_sql_expr(SQL_NULL);}
			| UNKNOWN			{$$ = new_const_non_valued_sql_expr(SQL_UNKNOWN);}

aggregate_distinct_opt :
												{$$ = SQL_RESULT_SET_ALL;}
						| ALL 					{$$ = SQL_RESULT_SET_ALL;}
						| DISTINCT 				{$$ = SQL_RESULT_SET_DISTINCT;}

func_expr : IDENTIFIER OPEN_BRACKET aggregate_distinct_opt expr_list CLOSE_BRACKET					{$$ = new_func_sql_expr($1, $3, $4);}

sub_query_expr :
			dql_query 															{$$ = new_sub_query_sql_expr(SQL_SUB_QUERY, $1);}

expr_list :
			expr 								{initialize_expr_list(&($$)); insert_in_expr_list(&($$), $1);}
			| expr_list COMMA expr 				{insert_in_expr_list(&($1), $3); $$ = $1;}

type : type_name type_specs type_with_or_without_timezone							{$$ = $1; $$.spec_size = $2.spec_size; memcpy($$.spec, $2.spec, sizeof($$.spec)); $$.with_time_zone = $3.with_time_zone;}

type_name : 	BOOL 					{$$.type_name = SQL_BOOL;}
				| BIT 					{$$.type_name = SQL_BIT;}
				| SMALLINT 				{$$.type_name = SQL_SMALLINT;}
				| INT 					{$$.type_name = SQL_INT;}
				| BIGINT 				{$$.type_name = SQL_BIGINT;}
				| REAL 					{$$.type_name = SQL_REAL;}
				| DOUBLE 				{$$.type_name = SQL_DOUBLE;}
				| FLOAT 				{$$.type_name = SQL_FLOAT;}
				| DECIMAL 				{$$.type_name = SQL_DECIMAL;}
				| NUMERIC 				{$$.type_name = SQL_NUMERIC;}
				| TEXT 					{$$.type_name = SQL_TEXT;}
				| CHAR 					{$$.type_name = SQL_CHAR;}
				| VARCHAR 				{$$.type_name = SQL_VARCHAR;}
				| CLOB 					{$$.type_name = SQL_CLOB;}
				| BLOB 					{$$.type_name = SQL_BLOB;}
				| DATE 					{$$.type_name = SQL_DATE;}
				| TIME 					{$$.type_name = SQL_TIME;}
				| TIMESTAMP 			{$$.type_name = SQL_TIMESTAMP;}

type_specs : 																												{$$.spec_size = 0;}
				| OPEN_BRACKET type_spec CLOSE_BRACKET 																		{$$.spec[0] = $2; $$.spec_size = 1;}
				| OPEN_BRACKET type_spec COMMA type_spec CLOSE_BRACKET 														{$$.spec[0] = $2; $$.spec[1] = $4; $$.spec_size = 2;}
				| OPEN_BRACKET type_spec COMMA type_spec COMMA type_spec CLOSE_BRACKET 										{$$.spec[0] = $2; $$.spec[1] = $4; $$.spec[2] = $6; $$.spec_size = 3;}
				| OPEN_BRACKET type_spec COMMA type_spec COMMA type_spec COMMA type_spec CLOSE_BRACKET 						{$$.spec[0] = $2; $$.spec[1] = $4; $$.spec[2] = $6; $$.spec[3] = $8; $$.spec_size = 4;}
				| OPEN_BRACKET type_spec COMMA type_spec COMMA type_spec COMMA type_spec COMMA type_spec CLOSE_BRACKET 		{$$.spec[0] = $2; $$.spec[1] = $4; $$.spec[2] = $6; $$.spec[3] = $8; $$.spec[4] = $10; $$.spec_size = 5;}

type_spec : INTEGER  			{unsigned long long int res = 0; get_unsigned_long_long_int_from_dstring(&($1), 10, &res); $$ = res; deinit_dstring(&($1));}

type_with_or_without_timezone : 						{$$.with_time_zone = 0;}
									| WITH_TZ 			{$$.with_time_zone = 1;}
									| WITHOUT_TZ 		{$$.with_time_zone = 0;}

%%

/* Error handling */
int sqlerror(void *scanner, struct sql** sql_ast, const char *msg) {
	fprintf(stderr, "Error: %s\n", msg);
	return 0;
}