#ifndef SQL_EXPRESSION_ITER_H
#define SQL_EXPRESSION_ITER_H

#include<sqltoast/sql_expression.h>

typedef enum traversal_status traversal_status;
enum traversal_status
{
	CONTINUE_ITERATION,   // normal return
	SKIP_CHILD_ITERATION, // do not iterate over children of this expresison, do post order and exit
	EXIT_EXPRESSION,      // skip children and even post_order
	EXIT_ITERATION,       // exit out of the iteration, the result has been produced
};

// this is a recursive function that can behave as pre-order/post-order traversal over the sql_expression tree
void iterate_over_sql_expr(const sql_expression* expr,
						traversal_status (*pre_order_callback)(const sql_expression* expr, void* context), // tells us what to do next
						void (*post_order_callback)(const sql_expression* expr, void* context), // perform any cleanup process here
						void* context);

#endif