#ifndef __LIST_H__
#define __LIST_H__

// List.h: Circular doubly-linked list implementation
// Note: an empty list head (list_t) refers itself. In the same way, first->prev & last->next
// nodes refers to the the head too. This avoids checking for NULL all the time.

// List node. Embed it in the structure you're making a list of
typedef struct ListNode {
	struct ListNode* next;
	struct ListNode* prev;
} lnode_t;

// List head. Declare once to keep track of your list
typedef struct List {
	struct ListNode* head;
	struct ListNode* tail;
} list_t;

/// @brief Statically declare a list variable, with name `list_name`.
/// Use this if you need the list initialized at compile time. Or use List_init at runtime.
/// Example: `DECLARE_LIST(my_list);` declares and initialize a `list_t my_list` variable
#define DECLARE_LIST(list_name) \
	list_t list_name = { .head=(lnode_t*)&list_name, .tail=(lnode_t*)&list_name }

#define List_isEmpty(list_ptr) \
	((list_ptr)->head == (lnode_t*)(list_ptr))

/// @brief Get the pointer to the object of type `obj_type` that contains the
/// list node pointer `ptr`, where the object contains a list node member named
/// `member_name`. Example usage, to get the object that contains the `node` list node:
/// ```
/// struct MyList { int your_data_here; lnode_t list_node; };
/// lnode_t* node; // supposing this is set to a valid node in a valid MyList instance
/// struct MyList* my_list = List_getObject(node, struct MyList, list_node);
/// ```
#define List_getObject(ptr, obj_type, member_name) \
	( (obj_type *) ((char *)(ptr) - offsetof(obj_type, member_name)) )

/// @brief Iterate over a list (node is updated at each iteration). Usage:
/// ```c
/// lnode_t* node;
/// List_foreach(list, node) {
//      ; // use node here, for example with List_getObject
/// }
/// ```
/// Or do it 'manually' if you prefer:
/// ```c
/// for (lnode_t* node=list->head ; node!=(lnode_t*)list ; node=node->next)
///     ; // use node here, for example with List_getObject
/// ```
#define List_foreach(list, node) \
	for ((node)=(list)->head ; (node) != (lnode_t*)(list) ; (node)=(node)->next)

void List_init(list_t* list);
void List_pushFront(list_t* list, lnode_t* node);
void List_pushBack(list_t* list, lnode_t* node);
void List_empty(list_t* list);
void List_popFront(list_t* list);
void List_popBack(list_t* list);
void List_pop(list_t* list, lnode_t* node);

#endif
