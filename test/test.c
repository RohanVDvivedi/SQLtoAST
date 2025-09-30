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
	sql* tree = parsesql(&rs, &error);

	if(tree)
	{
		printf("%d with %d errr\n", tree->ival, error);
		free(tree);
	}
	else
		printf("error = %d\n", error);

	return 0;
}