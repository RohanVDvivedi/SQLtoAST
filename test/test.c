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
	sql* sqlast = parsesql(&rs, &error);

	if(sqlast)
	{
		print_sql_expr(sqlast->expr);
		printf("\n");
		printf("error = %d\n", error);
		destroysql(sqlast);
	}
	else
		printf("error = %d\n", error);

	return 0;
}