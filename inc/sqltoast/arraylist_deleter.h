#ifndef ARRAYLIST_DELETER_H
#define ARRAYLIST_DELETER_H

void delete_all_and_deinitialize_arraylist_1d(arraylist* a, void (*delete_object)(void* o));

void delete_all_and_deinitialize_arraylist_2d(arraylist* a, void (*delete_object)(void* o));

#endif