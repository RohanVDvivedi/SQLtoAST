#include<stdio.h>
#include<stdlib.h>

#include<cutlery/stream.h>
#include<cutlery/stream_for_file_descriptor.h>
#include<cutlery/stream_for_dstring.h>

#include<sqltoast/sqltoast.h>

int main()
{
	stream rs, ws;
	initialize_stream_for_fd(&rs, 0);
	initialize_stream_for_fd(&ws, 1);

	int error1 = 0;
	sql* sqlast1 = parse_sql(&rs, &error1);

	if(sqlast1)
	{
		dstring str1;
		init_empty_dstring(&str1, 0);

		make_dstring_empty(&str1);
		snprint_sql(&str1, sqlast1);
		printf("unflattened " printf_dstring_format "\n\n", printf_dstring_params(&str1));

		flatten_exprs_sql(sqlast1);
		
		make_dstring_empty(&str1);
		snprint_sql(&str1, sqlast1);
		printf("flattened " printf_dstring_format "\n\n", printf_dstring_params(&str1));

		if(sqlast1->type != DDL)
		{
			dstring stream_reader_str = get_dstring_pointing_to_dstring(&str1);

			stream re_strm;
			initialize_dstring_stream(&re_strm, &stream_reader_str);

			int error2 = 0;
			sql* sqlast2 = parse_sql(&re_strm, &error2);

			int temp_re_strm_error = 0;
			close_stream(&re_strm, &temp_re_strm_error);
			deinitialize_stream(&re_strm);

			if(sqlast2 != NULL)
			{
				dstring str2;
				init_empty_dstring(&str2, 0);

				snprint_sql(&str2, sqlast2);
				printf("reparsed of serialized copy " printf_dstring_format "\n\n", printf_dstring_params(&str2));

				if(compare_dstring(&str1, &str2) != 0)
				{
					printf("REPARSING SERIALIZED QUERY STILL NOT SAME\n");
				}
				else
				{
					printf("REPARSING SERIALIZED QUERY PRODUCES SAME SERIALIZED STRING\n");
				}

				deinit_dstring(&str2);

				delete_sql(sqlast2);
			}
			else
				printf("error2 = %d\n", error2);
		}

		deinit_dstring(&str1);

		delete_sql(sqlast1);
	}
	else
		printf("error1 = %d\n", error1);

	deinitialize_stream(&rs);
	deinitialize_stream(&ws);

	return 0;
}