/* Copyright © 2015 coord.cn. All rights reserved.
 * @author: QianYe(coordcn@163.com)
 * @license: MIT license
 * @overview: 
 */

#include "alloc.h"
#include "hash.h"
#include "uv.h"

static LuaIO_pool_t LuaIO_hash_item_pool;
static LuaIO_pool_t* LuaIO_hash_item_pool_ptr;

/*double slots size and rehash*/
static void LuaIO_hash_rehash(LuaIO_hash_t* hash) {
  size_t old_size = 1 << hash->bits;
  size_t bits = hash->bits++;
  size_t size = 1 << bits;
  hash->max_items = size;
  LuaIO_hlist_head_t* old_slots = hash->slots;
  LuaIO_hlist_head_t* new_slots = (LuaIO_hlist_head_t*)LuaIO_malloc(sizeof(LuaIO_hlist_head_t) * size);
  for (size_t i = 0; i < size; i++) {
    new_slots[i].first = NULL;
  }

  LuaIO_hlist_head_t* current_slot;
  LuaIO_hlist_node_t* pos;
  LuaIO_hash_item_t* item;
  size_t index;
  for (size_t i = 0; i < old_size; i++) {
    current_slot = &old_slots[i];
    while (!LuaIO_hlist_is_empty(current_slot)) {
      pos = current_slot->first;
      LuaIO_hlist_remove(pos);
      item = (LuaIO_hash_item_t*)LuaIO_hlist_entry(pos, LuaIO_hash_item_t, node);
      index = LuaIO_hash_slot(item->hash, bits);
      LuaIO_hlist_insert_head(&item->node, &new_slots[index]);
    }
  }

  hash->slots = new_slots;
  LuaIO_free(old_slots);
}

LuaIO_hash_t* LuaIO_hash__create(size_t bits, 
                                 size_t max_age, 
                                 LuaIO_hash_free_fn free_fn, 
                                 LuaIO_hash_strcmp_fn strcmp_fn, 
                                 LuaIO_hash_fn hash_fn) {
  if (!LuaIO_hash_item_pool_ptr) {
    LuaIO_pool_init(&LuaIO_hash_item_pool, LUAIO_HASH_ITEM_POOL_MAX_FREE_CHUNKS);
    LuaIO_hash_item_pool_ptr = &LuaIO_hash_item_pool;
  }

  LuaIO_hash_t* hash = (LuaIO_hash_t*)LuaIO_malloc(sizeof(LuaIO_hash_t));
  hash->bits = bits;
  size_t size = 1 << bits;
  hash->max_items = size;
  hash->max_age = max_age;
  hash->free = free_fn;
  hash->str_cmp = strcmp_fn;
  hash->hash = hash_fn;
  hash->items = 0;
  LuaIO_hlist_head_t* slots = (LuaIO_hlist_head_t*)LuaIO_malloc(sizeof(LuaIO_hlist_head_t) * size);
  for (size_t i = 0; i < size; i++) {
    slots[i].first = NULL;
  }
  hash->slots = slots;

  return hash;
}

void LuaIO_hash_destroy(LuaIO_hash_t* hash) {
  size_t size = 1 << hash->bits;
  LuaIO_hlist_head_t* slots = hash->slots;

  LuaIO_hlist_head_t* current_slot;
  LuaIO_hlist_node_t* pos;
  LuaIO_hash_item_t* item;
  for (size_t i = 0; i < size; i++) {
    current_slot = &slots[i];
    while (!LuaIO_hlist_is_empty(current_slot)) {
      pos = current_slot->first;
      LuaIO_hlist_remove(pos);
      item = (LuaIO_hash_item_t*)LuaIO_hlist_entry(pos, LuaIO_hash_item_t, node);
      if(item->pointer) hash->free(item->pointer); 
      if(item->key) LuaIO_free(item->key);
      LuaIO_pfree(&LuaIO_hash_item_pool, item);
    }
  }

  LuaIO_free(slots);
  LuaIO_free(hash);
}

void LuaIO_hash_str_set(LuaIO_hash_t* hash, char* key, size_t n, void* pointer) {
  if (hash->items >= hash->max_items) {
    LuaIO_hash_rehash(hash);
  }

  size_t hash_key = hash->hash(key, n);
  size_t index = LuaIO_hash_slot(hash_key, hash->bits);
  LuaIO_hlist_head_t* slot = &hash->slots[index];
  LuaIO_hlist_node_t* pos;
  LuaIO_hash_item_t* item;
  int cmp;

  if (!LuaIO_hlist_is_empty(slot)) {
    LuaIO_hlist_for_each(pos, slot) {
      item = (LuaIO_hash_item_t*)LuaIO_hlist_entry(pos, LuaIO_hash_item_t, node);
      if (item->key_length == n) {
        cmp = hash->str_cmp(item->key, key, n);
        if (!cmp) {
          hash->free(item->pointer);
          item->pointer = pointer;
          if (hash->max_age) {
            item->expires = uv_now(uv_default_loop()) + hash->max_age;
          } else {
            item->expires = 0;
          }
          return;
        }
      }
    }
  }

  item = (LuaIO_hash_item_t*)LuaIO_palloc(&LuaIO_hash_item_pool, sizeof(LuaIO_hash_item_t));
  item->pointer = pointer;
  item->hash = hash_key;
  if (hash->max_age) {
    item->expires = uv_now(uv_default_loop()) + hash->max_age;
  } else {
    item->expires = 0;
  }
  item->key_length = n;
  item->key = key;
  LuaIO_hlist_insert_head(&item->node, slot);
  hash->items++;
}

void* LuaIO_hash_str_get(LuaIO_hash_t* hash, const char* key, size_t n) {
  size_t hash_key = hash->hash(key, n);
  size_t index = LuaIO_hash_slot(hash_key, hash->bits);
  LuaIO_hlist_head_t* slot = &hash->slots[index];
  LuaIO_hlist_node_t* pos;
  LuaIO_hash_item_t* item;
  int cmp;

  if (!LuaIO_hlist_is_empty(slot)) {
    LuaIO_hlist_for_each(pos, slot) {
      item = (LuaIO_hash_item_t*)LuaIO_list_entry(pos, LuaIO_hash_item_t, node);
      if (item->key_length == n) {
        cmp = hash->str_cmp(item->key, key, n);
        if (!cmp) {
          if (item->expires && (item->expires < uv_now(uv_default_loop()))) {
            return NULL;
          }
          return item->pointer;
        }
      }
    }
  }

  return NULL;
}

void LuaIO_hash_str_remove(LuaIO_hash_t* hash, char* key, size_t n) {
  size_t hash_key = hash->hash(key, n);
  size_t index = LuaIO_hash_slot(hash_key, hash->bits);
  LuaIO_hlist_head_t* slot = &hash->slots[index];
  LuaIO_hlist_node_t* pos;
  LuaIO_hash_item_t* item;
  int cmp;

  if (!LuaIO_hlist_is_empty(slot)) {
    LuaIO_hlist_for_each(pos, slot) {
      item = (LuaIO_hash_item_t*)LuaIO_list_entry(pos, LuaIO_hash_item_t, node);
      if (item->key_length == n) {
        cmp = hash->str_cmp(item->key, key, n);
        if (!cmp) {
          LuaIO_hlist_remove(&item->node);
          hash->free(item->pointer);
          LuaIO_free(item->key);
          LuaIO_pfree(&LuaIO_hash_item_pool, item);
          hash->items--;
          return;
        }
      }
    }
  }
}

void LuaIO_hash_int_set(LuaIO_hash_t* hash, size_t key, void* pointer) {
  if (hash->items >= hash->max_items) {
    LuaIO_hash_rehash(hash);
  }

  size_t index = LuaIO_hash_slot(key, hash->bits);
  LuaIO_hlist_head_t* slot = &hash->slots[index];
  LuaIO_hlist_node_t* pos;
  LuaIO_hash_item_t* item;

  if (!LuaIO_hlist_is_empty(slot)) {
    LuaIO_hlist_for_each(pos, slot) {
      item = (LuaIO_hash_item_t*)LuaIO_list_entry(pos, LuaIO_hash_item_t, node);
      if (item->hash == key) {
        hash->free(item->pointer);
        item->pointer = pointer;
        if (hash->max_age) {
          item->expires = uv_now(uv_default_loop()) + hash->max_age;
        } else {
          item->expires = 0;
        }
        return;
      }
    }
  }

  item = (LuaIO_hash_item_t*)LuaIO_palloc(&LuaIO_hash_item_pool, sizeof(LuaIO_hash_item_t));
  item->pointer = pointer;
  item->hash = key;
  if (hash->max_age) {
    item->expires = uv_now(uv_default_loop()) + hash->max_age;
  } else {
    item->expires = 0;
  }
  LuaIO_hlist_insert_head(&item->node, slot);
  hash->items++;
}

void* LuaIO_hash_int_get(LuaIO_hash_t* hash, size_t key) {
  size_t index = LuaIO_hash_slot(key, hash->bits);
  LuaIO_hlist_head_t* slot = &hash->slots[index];
  LuaIO_hlist_node_t* pos;
  LuaIO_hash_item_t* item;

  if (!LuaIO_hlist_is_empty(slot)) {
    LuaIO_hlist_for_each(pos, slot) {
      item = (LuaIO_hash_item_t*)LuaIO_list_entry(pos, LuaIO_hash_item_t, node);
      if (item->hash == key) {
        if (item->expires && (item->expires < uv_now(uv_default_loop()))) {
          return NULL;
        }
        return item->pointer;
      }
    }
  }

  return NULL;
}

void LuaIO_hash_int_remove(LuaIO_hash_t* hash, size_t key) {
  size_t index = LuaIO_hash_slot(key, hash->bits);
  LuaIO_hlist_head_t* slot = &hash->slots[index];
  LuaIO_hlist_node_t* pos;
  LuaIO_hash_item_t* item;

  if (!LuaIO_hlist_is_empty(slot)) {
    LuaIO_hlist_for_each(pos, slot) {
      item = (LuaIO_hash_item_t*)LuaIO_list_entry(pos, LuaIO_hash_item_t, node);
      if (item->hash == key) {
        LuaIO_hlist_remove(&item->node);
        hash->free(item->pointer);
        LuaIO_pfree(&LuaIO_hash_item_pool, item);
        hash->items--;
        return;
      }
    }
  }
}

void LuaIO_hash_int_set_value(LuaIO_hash_t* hash, size_t key, int value) {
  if (hash->items >= hash->max_items) {
    LuaIO_hash_rehash(hash);
  }

  size_t index = LuaIO_hash_slot(key, hash->bits);
  LuaIO_hlist_head_t* slot = &hash->slots[index];
  LuaIO_hlist_node_t* pos;
  LuaIO_hash_item_t* item;

  if (!LuaIO_hlist_is_empty(slot)) {
    LuaIO_hlist_for_each(pos, slot) {
      item = (LuaIO_hash_item_t*)LuaIO_list_entry(pos, LuaIO_hash_item_t, node);
      if (item->hash == key) {
        item->value = value;
        return;
      }
    }
  }

  item = (LuaIO_hash_item_t*)LuaIO_palloc(&LuaIO_hash_item_pool, sizeof(LuaIO_hash_item_t));
  item->value = value;
  item->hash = key;
  LuaIO_hlist_insert_head(&item->node, slot);
  hash->items++;
}

int LuaIO_hash_int_get_value(LuaIO_hash_t* hash, size_t key, int* value) {
  size_t index = LuaIO_hash_slot(key, hash->bits);
  LuaIO_hlist_head_t* slot = &hash->slots[index];
  LuaIO_hlist_node_t* pos;
  LuaIO_hash_item_t* item;

  if (!LuaIO_hlist_is_empty(slot)) {
    LuaIO_hlist_for_each(pos, slot) {
      item = (LuaIO_hash_item_t*)LuaIO_list_entry(pos, LuaIO_hash_item_t, node);
      if (item->hash == key) {
        *value = item->value;
        return 0;
      }
    }
  }

  return 1;
}

void LuaIO_hash_int_remove_value(LuaIO_hash_t* hash, size_t key) {
  size_t index = LuaIO_hash_slot(key, hash->bits);
  LuaIO_hlist_head_t* slot = &hash->slots[index];
  LuaIO_hlist_node_t* pos;
  LuaIO_hash_item_t* item;

  if (!LuaIO_hlist_is_empty(slot)) {
    LuaIO_hlist_for_each(pos, slot) {
      item = (LuaIO_hash_item_t*)LuaIO_list_entry(pos, LuaIO_hash_item_t, node);
      if (item->hash == key) {
        LuaIO_hlist_remove(&item->node);
        LuaIO_pfree(&LuaIO_hash_item_pool, item);
        hash->items--;
        return;
      }
    }
  }
}

int LuaIO_hash_int_get_and_remove_value(LuaIO_hash_t* hash, size_t key, int* value) {
  size_t index = LuaIO_hash_slot(key, hash->bits);
  LuaIO_hlist_head_t* slot = &hash->slots[index];
  LuaIO_hlist_node_t* pos;
  LuaIO_hash_item_t* item;

  if (!LuaIO_hlist_is_empty(slot)) {
    LuaIO_hlist_for_each(pos, slot) {
      item = (LuaIO_hash_item_t*)LuaIO_list_entry(pos, LuaIO_hash_item_t, node);
      if (item->hash == key) {
        *value = item->value;
        LuaIO_hlist_remove(&item->node);
        LuaIO_pfree(&LuaIO_hash_item_pool, item);
        hash->items--;
        return 0;
      }
    }
  }

  return 1;
}
