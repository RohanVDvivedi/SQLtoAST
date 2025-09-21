#ifndef SQLTOAST_H
#define SQLTOAST_H

#include<cutlery/stream.h>

#include<sqltoast/sqltree.h>

// main function that builds the AST form the SQL you get from cutlery like stream
SQLtree* SQL_to_AST(stream* input, int* error);

#endif