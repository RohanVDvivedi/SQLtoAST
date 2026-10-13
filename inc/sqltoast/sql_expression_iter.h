#ifndef SQL_EXPRESSION_ITER_H
#define SQL_EXPRESSION_ITER_H

#include<sqltoast/sql_expression.h>

typedef enum traversal_status traversal_status;
enum traversal_status
{
	CONTINUE_ITERATION,   // normal success return, proceed further
	SKIP_CHILD_ITERATION, // do not iterate over children of this expresison, do post order and exit, and continue with the rest of the expressions as usual
	EXIT_ITERATION,       // exit out of the iteration, calling post-order callback on the rest of the expressions on the stack (including this one) and exit
};

// this is a recursive function that can behave as pre-order/post-order traversal over the sql_expression tree
// sub-queries are not traversed
// return value of 0, implies EXIT_ITERATION was encountered
int iterate_over_sql_expr(const sql_expression* expr,
						traversal_status (*pre_order_callback)(const sql_expression* expr, void* context), // tells us what to do next
						void (*post_order_callback)(const sql_expression* expr, void* context), // perform any cleanup process here
						void* context);

#endif