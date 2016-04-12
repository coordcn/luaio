/* @author: QianYe(coordcn@163.com)
 * @overview: doubly linked list from linux kernel
 * @reference: stddef.h offsetof
 *             kernel.h container_of
 *             poison.h LIST_POISON1 LIST_POISON2
 */

#ifndef LUAIO_LIST_H
#define LUAIO_LIST_H

#include "stddef.h"

#ifndef offset_of
#define offset_of(type, member) ((intptr_t)((char*)(&(((type*)(0))->member))))
#endif

#ifndef container_of
#define container_of(ptr, type, member) ((type*)((char*)(ptr) - offset_of(type, member)))
#endif

/**
  Simple doubly linked list implementation.
  Some of the internal functions ("__xxx") are useful when
  manipulating whole lists rather than single entries, as
  sometimes we already know the next/prev entries and we can
  generate better code by using them directly rather than
  using the generic single-entry routines.
 */

struct luaio_list_s {
  struct luaio_list_s *next;
  struct luaio_list_s *prev;
};

typedef struct luaio_list_s luaio_list_t;

static inline void luaio_list_init(luaio_list_t *list) {
  list->next = list;
  list->prev = list;
}

static inline void luaio_list__insert(luaio_list_t *new, 
                                      luaio_list_t *prev, 
                                      luaio_list_t *next) {
  next->prev = new;
  new->next = next;
  new->prev = prev;
  prev->next = new;
}

static inline void luaio_list_insert_head(luaio_list_t *new,
                                          luaio_list_t *head) {
  luaio_list__insert(new, head, head->next);
}

static inline void luaio_list_insert_tail(luaio_list_t *new, 
                                          luaio_list_t *head) {
  luaio_list__insert(new, head->prev, head);
}

static inline void luaio_list__remove(luaio_list_t *prev, 
                                      luaio_list_t *next) {
  prev->next = next;
  next->prev = prev;
}

static inline void luaio_list_remove(luaio_list_t *entry) {
  luaio_list__remove(entry->prev, entry->next);
}

static inline void luaio_list_remove_init(luaio_list_t *entry) {
  luaio_list__remove(entry->prev, entry->next);
  luaio_list_init(entry);
}

static inline void luaio_list_move_head(luaio_list_t *list, 
                                        luaio_list_t *head) {
  luaio_list__remove(list->prev, list->next);
  luaio_list_insert_head(list, head);
}

static inline void luaio_list_move_tail(luaio_list_t *list, 
                                        luaio_list_t *head) {
  luaio_list__remove(list->prev, list->next);
  luaio_list_insert_tail(list, head);
}

static inline int luaio_list_is_empty(const luaio_list_t *head) {
  return head->next == head;
}

static inline int luaio_list_is_single(const luaio_list_t *head) {
  return !luaio_list_is_empty(head) && (head->next == head->prev);
}

static inline int luaio_list_is_last(const luaio_list_t *list, 
                                     const luaio_list_t *head) {
  return list->next == head;
}

/**
  luaio_list_entry - get the struct for this entry
  @ptr:	the &luaio_list_t pointer.
  @type: the type of the struct this is embedded in.
  @member:	the name of the luaio_list_t within the struct.
 */
#define luaio_list_entry(ptr, type, member) \
  container_of(ptr, type, member)

/**
  luaio_list_first_entry - get the first element from a list
  @ptr:	the list head to take the element from.
  @type: the type of the struct this is embedded in.
  @member:	the name of the luaio_list_t within the struct.
  Note that if the list is empty, it returns NULL.
 */
#define luaio_list_first_entry(ptr, type, member) \
  (!luaio_list_is_empty(ptr) ? luaio_list_entry((ptr)->next, type, member) : NULL)

/**
  luaio_list_next_entry - get the next element in list
  @pos:	the type * to cursor
  @member:	the name of the luaio_list_t within the struct.
 */
#define luaio_list_next_entry(pos, member) \
  luaio_list_entry((pos)->member.next, typeof(*(pos)), member)

/**
  luaio_list_prev_entry - get the prev element in list
  @pos:	the type * to cursor
  @member:	the name of the luaio_list_t within the struct.
 */
#define luaio_list_prev_entry(pos, member) \
  luaio_list_entry((pos)->member.prev, typeof(*(pos)), member)

/**
  luaio_list_foreach	-	iterate over a list
  @pos:	the &luaio_list_t to use as a loop cursor.
  @head:	the head for your list.
 */
#define luaio_list_foreach(pos, head) \
  for (pos = (head)->next; pos != (head); pos = pos->next)

/**
  luaio_list_foreach_reverse	-	iterate over a list backwards
  @pos:	the &luaio_list_t to use as a loop cursor.
  @head:	the head for your list.
 */
#define luaio_list_foreach_reverse(pos, head) \
  for (pos = (head)->prev; pos != (head); pos = pos->prev)

/**
  luaio_list_foreach_entry	-	iterate over list of given type
  @pos:	the type * to use as a loop cursor.
  @head:	the head for your list.
  @member:	the name of the luaio_list_t within the struct.
 */
#define luaio_list_foreach_entry(pos, head, member)				\
  for (pos = luaio_list_entry((head)->next, typeof(*pos), member);	\
       &pos->member != (head); 	\
       pos = luaio_list_entry(pos->member.next, typeof(*pos), member))

/**
  luaio_list_foreach_entry_reverse - iterate backwards over list of given type.
  @pos:	the type * to use as a loop cursor.
  @head:	the head for your list.
  @member:	the name of the list_struct within the struct.
 */
#define luaio_list_foreach_entry_reverse(pos, head, member)			\
  for (pos = luaio_list_entry((head)->prev, typeof(*pos), member);	\
       &pos->member != (head); 	\
       pos = luaio_list_entry(pos->member.prev, typeof(*pos), member))

/**
  Double linked lists with a single pointer list head.
  Mostly useful for hash tables where the two pointer list head is
  too wasteful.
  You lose the ability to access the tail in O(1).
 */

struct luaio_hlist_node_s {
  struct luaio_hlist_node_s *next;
  struct luaio_hlist_node_s **pprev;
};

struct luaio_hlist_head_s {
  struct luaio_hlist_node_s *first;
};

typedef struct luaio_hlist_node_s luaio_hlist_node_t;
typedef struct luaio_hlist_head_s luaio_hlist_head_t;

#define luaio_hlist_head_init(ptr) ((ptr)->first = NULL)

static inline void luaio_hlist_node_init(luaio_hlist_node_t *node) {
  node->next = NULL;
  node->pprev = NULL;
}

static inline int luaio_hlist_is_empty(luaio_hlist_head_t *head) {
  return !head->first;
}

static inline int luaio_hlist_is_single(luaio_hlist_head_t *head) {
  return head->first && !head->first->next;
}

static inline void luaio_hlist_remove(luaio_hlist_node_t *node) {
  if (node->pprev) {
    luaio_hlist_node_t *next = node->next;
    luaio_hlist_node_t **pprev = node->pprev;
    *pprev = next;
    if(next) next->pprev = pprev;
  }
}

static inline void luaio_hlist_remove_init(luaio_hlist_node_t *node) {
  luaio_hlist_remove(node);
  luaio_hlist_node_init(node);
}

static inline void luaio_hlist_insert_head(luaio_hlist_node_t *node,
                                           luaio_hlist_head_t *head) {
  luaio_hlist_node_t *first = head->first;
  node->next = first;
  if (first) first->pprev = &node->next;
  head->first = node;
  node->pprev = &head->first;
}

#define luaio_hlist_entry(ptr, type, member) \
  container_of(ptr, type, member)

#define luaio_hlist_for_each(pos, head) \
  for (pos = (head)->first; pos ; pos = pos->next)

/**
  luaio_hlist_for_each_entry	- iterate over list of given type
  @pos:	the type * to use as a loop cursor.
  @head:	the head for your list.
  @member:	the name of the luaio_hlist_node_t within the struct.
 */
#define luaio_hlist_for_each_entry(pos, head, member)				\
  for (pos = hlist_entry((head)->first, typeof(*(pos)), member);\
       pos;							\
       pos = hlist_entry((pos)->member.next, typeof(*(pos)), member))

#endif /*LUAIO_LIST_H*/
