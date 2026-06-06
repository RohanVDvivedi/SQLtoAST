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
		dstring str1;
		init_empty_dstring(&str1, 0);

		/*snprint_sql(&str1, sqlast);
		printf(printf_dstring_format "\n\n", printf_dstring_params(&str1));*/

		flatten_exprs_sql(sqlast);
		
		snprint_sql(&str1, sqlast);
		printf(printf_dstring_format "\n\n", printf_dstring_params(&str1));
		
		printf("error = %d\n", error);
		
		delete_sql(sqlast);
	}
	else
		printf("error = %d\n", error);

	return 0;
}