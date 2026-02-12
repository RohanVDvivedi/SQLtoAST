#include<sqltoast/sqltoast.h>

#include<stdlib.h>

void delete_all_and_deinitialize_arraylist_1d(arraylist* a, void (*delete_object)(void* o))
{
	for(cy_uint i = 0; i < get_element_count_arraylist(a); i++)
	{
		void* o = (void*) get_from_front_of_arraylist(a, i);
		if(o)
			delete_object(o);
	}
	deinitialize_arraylist(a);
}

void delete_all_and_deinitialize_arraylist_2d(arraylist* a, void (*delete_object)(void* o))
{
	for(cy_uint i = 0; i < get_element_count_arraylist(a); i++)
	{
		arraylist* row = (arraylist*) get_from_front_of_arraylist(a, i);
		if(row)
		{
			for(cy_uint j = 0; j < get_element_count_arraylist(row); j++)
			{
				void* cell = (void*) get_from_front_of_arraylist(row, j);
				if(cell != NULL)
					delete_object(cell);
			}
			deinitialize_arraylist(row);
			free(row);
		}
	}
	deinitialize_arraylist(a);
}
