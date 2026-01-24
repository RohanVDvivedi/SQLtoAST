#include<stdio.h>
#include<stdlib.h>

#include<cutlery/stream.h>
#include<cutlery/stream_for_file_descriptor.h>
#include<sqltoast/sqltoast.h>

int main()
{
	stream rs, ws;
	initialize_stream_for_fd(&rs, 0);
	initialize_stream_for_fd(&ws, 1);

	int error = 0;
	sql* sqlast = parse_sql(&rs, &error);

	if(sqlast)
	{
		print_sql(sqlast);
		// call flatten_similar_associative_operators_in_sql_expression on all sql expressions
		//print_sql(sqlast);
		printf("\n\n");
		printf("error = %d\n", error);
		delete_sql(sqlast);
	}
	else
		printf("error = %d\n", error);

	return 0;
}