#include <stddef.h>
#include "assert.h"

#include "List.h"

void List_init(list_t* list){
	assert(list != NULL);

	list->head = (lnode_t*) list;
	list->tail = (lnode_t*) list;
}

void List_pushFront(list_t* list, lnode_t* node){
	assert(list != NULL);
	assert(node != NULL);

	if (List_isEmpty(list)){
		list->head = node;
		list->tail = node;
		node->next = (lnode_t*) list;
		node->prev = (lnode_t*) list;
		return;
	}

	list->head->prev = node;
	node->next = list->head;
	node->prev = (lnode_t*) list;
	list->head = node;
}

void List_pushBack(list_t* list, lnode_t* node){
	assert(list != NULL);
	assert(node != NULL);

	if (List_isEmpty(list)){
		list->head = node;
		list->tail = node;
		node->next = (lnode_t*) list;
		node->prev = (lnode_t*) list;
		return;
	}

	list->tail->next = node;
	node->prev = list->tail;
	node->next = (lnode_t*) list;
	list->tail = node;
}

void List_empty(list_t* list){
	assert(list != NULL);

	list->head = (lnode_t*) list;
	list->tail = (lnode_t*) list;
}

// List only contain one element, pop it
static inline void popUniqueElement(list_t* list){
	list->head->next = NULL;
	list->head->prev = NULL;

	list->head = (lnode_t*) list;
	list->tail = (lnode_t*) list;
}

static inline void popFrontInternal(list_t* list){
	lnode_t* front_node = list->head;

	list->head = list->head->next;
	list->head->prev = (lnode_t*)list;

	front_node->next = NULL;
	front_node->prev = NULL;
}

static inline void popBackInternal(list_t* list){
	lnode_t* back_node = list->tail;

	list->tail = back_node->prev;
	back_node->prev->next = (lnode_t*)list;

	back_node->next = NULL;
	back_node->prev = NULL;
}

void List_popFront(list_t* list){
	assert(list != NULL);
	if (List_isEmpty(list)) return;

	if (list->head == list->tail)
		return popUniqueElement(list);

	popFrontInternal(list);
}

void List_popBack(list_t* list){
	assert(list != NULL);
	if (List_isEmpty(list)) return;

	if (list->head == list->tail)
		return popUniqueElement(list);

	popBackInternal(list);
}

void List_pop(list_t* list, lnode_t* node){
	assert(list != NULL);
	assert(node != NULL);
	if (List_isEmpty(list)) return;

	if (node == list->head && node == list->tail)
		return popUniqueElement(list);
	if (node == list->head)
		return popFrontInternal(list);
	if (node == list->tail)
		return popBackInternal(list);

	node->next->prev = node->prev;
	node->prev->next = node->next;

	node->next = NULL;
	node->prev = NULL;
}
