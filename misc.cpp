#include <stdlib.h>
#include "misc.hpp"


//Adds ten elements to the list if this is necessary.
void add_element(void** list, int *required, int *available, size_t element_size){
	(*required)++;
	if(*required>*available){
		(*available)+=10;
		*list = realloc(*list,*available*element_size);
	}
	return;
}


//Deletes the node that comes after the specified node from the linked list
void del_node(node_t* node){
	node_t* next=node->next->next;
	free(node->next);
	node->next = next;
}


//Adds a node after the specified node in the linked list and returns a pointer to that new node
node_t* add_node(node_t* node){
	node_t* next = node->next;
	node->next = (node_t*)malloc(sizeof(node_t));
	node->next->next = next;
	return (node->next);
}

// vim: cc=100
