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

struct LuaIO_list_s {
  struct LuaIO_list_s *next;
  struct LuaIO_list_s *prev;
};

typedef struct LuaIO_list_s LuaIO_list_t;

static inline void LuaIO_list_init(LuaIO_list_t *list) {
  list->next = list;
  list->prev = list;
}

static inline void LuaIO_list__insert(LuaIO_list_t *new, 
                                      LuaIO_list_t *prev, 
                                      LuaIO_list_t *next) {
  next->prev = new;
  new->next = next;
  new->prev = prev;
  prev->next = new;
}

static inline void LuaIO_list_insert_head(LuaIO_list_t *new,
                                          LuaIO_list_t *head) {
  LuaIO_list__insert(new, head, head->next);
}

static inline void LuaIO_list_insert_tail(LuaIO_list_t *new, 
                                          LuaIO_list_t *head) {
  LuaIO_list__insert(new, head->prev, head);
}

static inline void LuaIO_list__remove(LuaIO_list_t *prev, 
                                      LuaIO_list_t *next) {
  prev->next = next;
  next->prev = prev;
}

static inline void LuaIO_list_remove(LuaIO_list_t *entry) {
  LuaIO_list__remove(entry->prev, entry->next);
}

static inline void LuaIO_list_remove_init(LuaIO_list_t *entry) {
  LuaIO_list__remove(entry->prev, entry->next);
  LuaIO_list_init(entry);
}

static inline void LuaIO_list_move_head(LuaIO_list_t *list, 
                                        LuaIO_list_t *head) {
  LuaIO_list__remove(list->prev, list->next);
  LuaIO_list_insert_head(list, head);
}

static inline void LuaIO_list_move_tail(LuaIO_list_t *list, 
                                        LuaIO_list_t *head) {
  LuaIO_list__remove(list->prev, list->next);
  LuaIO_list_insert_tail(list, head);
}

static inline int LuaIO_list_is_empty(const LuaIO_list_t *head) {
  return head->next == head;
}

static inline int LuaIO_list_is_single(const LuaIO_list_t *head) {
  return !LuaIO_list_is_empty(head) && (head->next == head->prev);
}

static inline int LuaIO_list_is_last(const LuaIO_list_t *list, 
                                     const LuaIO_list_t *head) {
  return list->next == head;
}

/**
  LuaIO_list_entry - get the struct for this entry
  @ptr:	the &LuaIO_list_t pointer.
  @type: the type of the struct this is embedded in.
  @member:	the name of the LuaIO_list_t within the struct.
 */
#define LuaIO_list_entry(ptr, type, member) \
  container_of(ptr, type, member)

/**
  LuaIO_list_first_entry - get the first element from a list
  @ptr:	the list head to take the element from.
  @type: the type of the struct this is embedded in.
  @member:	the name of the LuaIO_list_t within the struct.
  Note that if the list is empty, it returns NULL.
 */
#define LuaIO_list_first_entry(ptr, type, member) \
  (!LuaIO_list_is_empty(ptr) ? LuaIO_list_entry((ptr)->next, type, member) : NULL)

/**
  LuaIO_list_next_entry - get the next element in list
  @pos:	the type * to cursor
  @member:	the name of the LuaIO_list_t within the struct.
 */
#define LuaIO_list_next_entry(pos, member) \
  LuaIO_list_entry((pos)->member.next, typeof(*(pos)), member)

/**
  LuaIO_list_prev_entry - get the prev element in list
  @pos:	the type * to cursor
  @member:	the name of the LuaIO_list_t within the struct.
 */
#define LuaIO_list_prev_entry(pos, member) \
  LuaIO_list_entry((pos)->member.prev, typeof(*(pos)), member)

/**
  LuaIO_list_foreach	-	iterate over a list
  @pos:	the &LuaIO_list_t to use as a loop cursor.
  @head:	the head for your list.
 */
#define LuaIO_list_foreach(pos, head) \
  for (pos = (head)->next; pos != (head); pos = pos->next)

/**
  LuaIO_list_foreach_reverse	-	iterate over a list backwards
  @pos:	the &LuaIO_list_t to use as a loop cursor.
  @head:	the head for your list.
 */
#define LuaIO_list_foreach_reverse(pos, head) \
  for (pos = (head)->prev; pos != (head); pos = pos->prev)

/**
  LuaIO_list_foreach_entry	-	iterate over list of given type
  @pos:	the type * to use as a loop cursor.
  @head:	the head for your list.
  @member:	the name of the LuaIO_list_t within the struct.
 */
#define LuaIO_list_foreach_entry(pos, head, member)				\
  for (pos = LuaIO_list_entry((head)->next, typeof(*pos), member);	\
       &pos->member != (head); 	\
       pos = LuaIO_list_entry(pos->member.next, typeof(*pos), member))

/**
  LuaIO_list_foreach_entry_reverse - iterate backwards over list of given type.
  @pos:	the type * to use as a loop cursor.
  @head:	the head for your list.
  @member:	the name of the list_struct within the struct.
 */
#define LuaIO_list_foreach_entry_reverse(pos, head, member)			\
  for (pos = LuaIO_list_entry((head)->prev, typeof(*pos), member);	\
       &pos->member != (head); 	\
       pos = LuaIO_list_entry(pos->member.prev, typeof(*pos), member))

/**
  Double linked lists with a single pointer list head.
  Mostly useful for hash tables where the two pointer list head is
  too wasteful.
  You lose the ability to access the tail in O(1).
 */

struct LuaIO_hlist_node_s {
  struct LuaIO_hlist_node_s *next;
  struct LuaIO_hlist_node_s **pprev;
};

struct LuaIO_hlist_head_s {
  struct LuaIO_hlist_node_s *first;
};

typedef struct LuaIO_hlist_node_s LuaIO_hlist_node_t;
typedef struct LuaIO_hlist_head_s LuaIO_hlist_head_t;

#define LuaIO_hlist_head_init(ptr) ((ptr)->first = NULL)

static inline void LuaIO_hlist_node_init(LuaIO_hlist_node_t *node) {
  node->next = NULL;
  node->pprev = NULL;
}

static inline int LuaIO_hlist_is_empty(LuaIO_hlist_head_t *head) {
  return !head->first;
}

static inline int LuaIO_hlist_is_single(LuaIO_hlist_head_t *head) {
  return head->first && !head->first->next;
}

static inline void LuaIO_hlist_remove(LuaIO_hlist_node_t *node) {
  if (node->pprev) {
    LuaIO_hlist_node_t *next = node->next;
    LuaIO_hlist_node_t **pprev = node->pprev;
    *pprev = next;
    if(next) next->pprev = pprev;
  }
}

static inline void LuaIO_hlist_remove_init(LuaIO_hlist_node_t *node) {
  LuaIO_hlist_remove(node);
  LuaIO_hlist_node_init(node);
}

static inline void LuaIO_hlist_insert_head(LuaIO_hlist_node_t *node,
                                           LuaIO_hlist_head_t *head) {
  LuaIO_hlist_node_t *first = head->first;
  node->next = first;
  if (first) first->pprev = &node->next;
  head->first = node;
  node->pprev = &head->first;
}

#define LuaIO_hlist_entry(ptr, type, member) \
  container_of(ptr, type, member)

#define LuaIO_hlist_for_each(pos, head) \
  for (pos = (head)->first; pos ; pos = pos->next)

/**
  LuaIO_hlist_for_each_entry	- iterate over list of given type
  @pos:	the type * to use as a loop cursor.
  @head:	the head for your list.
  @member:	the name of the LuaIO_hlist_node_t within the struct.
 */
#define LuaIO_hlist_for_each_entry(pos, head, member)				\
  for (pos = hlist_entry((head)->first, typeof(*(pos)), member);\
       pos;							\
       pos = hlist_entry((pos)->member.next, typeof(*(pos)), member))

#endif /*LUAIO_LIST_H*/
