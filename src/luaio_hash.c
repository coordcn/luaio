/* Copyright © 2015 coord.cn. All rights reserved.
 * @author: QianYe(coordcn@163.com)
 * @license: MIT license
 * @overview: 
 */

#include "uv.h"
#include "luaio_hash.h"

/*double slots size and rehash*/
static void luaio_hash_rehash(luaio_hash_t *hash) {
  size_t old_size = 1 << hash->bits;
  size_t bits = hash->bits++;
  size_t size = 1 << bits;
  hash->max_items = size;
  luaio_hlist_head_t *old_slots = hash->slots;
  luaio_hlist_head_t *new_slots = luaio_malloc(sizeof(luaio_hlist_head_t) * size);
  for (size_t i = 0; i < size; i++) {
    new_slots[i].first = NULL;
  }

  luaio_hlist_head_t *current_slot;
  luaio_hlist_node_t *pos;
  luaio_hash_item_t *item;
  size_t index;
  for (size_t i = 0; i < old_size; i++) {
    current_slot = &old_slots[i];
    while (!luaio_hlist_is_empty(current_slot)) {
      pos = current_slot->first;
      luaio_hlist_remove(pos);
      item = luaio_hlist_entry(pos, luaio_hash_item_t, node);
      index = luaio_hash_slot(item->hash, bits);
      luaio_hlist_insert_head(&item->node, &new_slots[index]);
    }
  }

  hash->slots = new_slots;
  luaio_free(old_slots);
}

luaio_hash_t *luaio_hash__create(size_t bits, 
                                 size_t max_age, 
                                 luaio_hash_strcmp_fn strcmp_fn, 
                                 luaio_hash_fn hash_fn) {
  luaio_hash_t *hash = luaio_malloc(sizeof(luaio_hash_t));
  if (hash == NULL) return NULL;

  hash->bits = bits;
  size_t size = 1 << bits;
  hash->max_items = size;
  hash->max_age = max_age;
  hash->str_cmp = strcmp_fn;
  hash->hash = hash_fn;
  hash->items = 0;
  luaio_hlist_head_t *slots = luaio_malloc(sizeof(luaio_hlist_head_t) * size);
  if (slots == NULL) {
    luaio_free(hash);
    return NULL;
  }

  for (size_t i = 0; i < size; i++) {
    slots[i].first = NULL;
  }
  hash->slots = slots;
  return hash;
}

void luaio_hash_destroy(luaio_hash_t *hash) {
  if (hash == NULL) return;

  size_t size = 1 << hash->bits;
  luaio_hlist_head_t *slots = hash->slots;

  luaio_hlist_head_t *current_slot;
  luaio_hlist_node_t *pos;
  luaio_hash_item_t *item;
  for (size_t i = 0; i < size; i++) {
    current_slot = &slots[i];
    while (!luaio_hlist_is_empty(current_slot)) {
      pos = current_slot->first;
      luaio_hlist_remove(pos);
      item = luaio_hlist_entry(pos, luaio_hash_item_t, node);
      if(item->pointer) luaio_pfree(item->pointer); 
      if(item->key) luaio_pfree(item->key);
      luaio_pfree(item);
    }
  }

  luaio_free(slots);
  luaio_free(hash);
}

void luaio_hash_str_set(luaio_hash_t *hash, char *key, size_t n, void *pointer) {
  if (hash->items >= hash->max_items) {
    luaio_hash_rehash(hash);
  }

  size_t hash_key = hash->hash(key, n);
  size_t index = luaio_hash_slot(hash_key, hash->bits);
  luaio_hlist_head_t *slot = &hash->slots[index];
  luaio_hlist_node_t *pos;
  luaio_hash_item_t *item;
  int cmp;

  if (!luaio_hlist_is_empty(slot)) {
    luaio_hlist_for_each(pos, slot) {
      item = luaio_hlist_entry(pos, luaio_hash_item_t, node);
      if (item->key_length == n) {
        cmp = hash->str_cmp(item->key, key, n);
        if (!cmp) {
          luaio_pfree(item->pointer);
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

  item = luaio_palloc(sizeof(luaio_hash_item_t));
  item->pointer = pointer;
  item->hash = hash_key;
  if (hash->max_age) {
    item->expires = uv_now(uv_default_loop()) + hash->max_age;
  } else {
    item->expires = 0;
  }
  item->key_length = n;
  item->key = key;
  luaio_hlist_insert_head(&item->node, slot);
  hash->items++;
}

void *luaio_hash_str_get(luaio_hash_t *hash, const char *key, size_t n) {
  size_t hash_key = hash->hash(key, n);
  size_t index = luaio_hash_slot(hash_key, hash->bits);
  luaio_hlist_head_t *slot = &hash->slots[index];
  luaio_hlist_node_t *pos;
  luaio_hash_item_t *item;
  int cmp;

  if (!luaio_hlist_is_empty(slot)) {
    luaio_hlist_for_each(pos, slot) {
      item = luaio_list_entry(pos, luaio_hash_item_t, node);
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

void luaio_hash_str_remove(luaio_hash_t *hash, const char *key, size_t n) {
  size_t hash_key = hash->hash(key, n);
  size_t index = luaio_hash_slot(hash_key, hash->bits);
  luaio_hlist_head_t *slot = &hash->slots[index];
  luaio_hlist_node_t *pos;
  luaio_hash_item_t *item;
  int cmp;

  if (!luaio_hlist_is_empty(slot)) {
    luaio_hlist_for_each(pos, slot) {
      item = luaio_list_entry(pos, luaio_hash_item_t, node);
      if (item->key_length == n) {
        cmp = hash->str_cmp(item->key, key, n);
        if (!cmp) {
          luaio_hlist_remove(&item->node);
          luaio_pfree(item->pointer);
          luaio_pfree(item->key);
          luaio_pfree(item);
          hash->items--;
          return;
        }
      }
    }
  }
}

void luaio_hash_int_set(luaio_hash_t *hash, size_t key, void *pointer) {
  if (hash->items >= hash->max_items) {
    luaio_hash_rehash(hash);
  }

  size_t index = luaio_hash_slot(key, hash->bits);
  luaio_hlist_head_t *slot = &hash->slots[index];
  luaio_hlist_node_t *pos;
  luaio_hash_item_t *item;

  if (!luaio_hlist_is_empty(slot)) {
    luaio_hlist_for_each(pos, slot) {
      item = luaio_list_entry(pos, luaio_hash_item_t, node);
      if (item->hash == key) {
        luaio_pfree(item->pointer);
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

  item = luaio_palloc(sizeof(luaio_hash_item_t));
  item->pointer = pointer;
  item->hash = key;
  if (hash->max_age) {
    item->expires = uv_now(uv_default_loop()) + hash->max_age;
  } else {
    item->expires = 0;
  }
  luaio_hlist_insert_head(&item->node, slot);
  hash->items++;
}

void *luaio_hash_int_get(luaio_hash_t *hash, size_t key) {
  size_t index = luaio_hash_slot(key, hash->bits);
  luaio_hlist_head_t *slot = &hash->slots[index];
  luaio_hlist_node_t *pos;
  luaio_hash_item_t *item;

  if (!luaio_hlist_is_empty(slot)) {
    luaio_hlist_for_each(pos, slot) {
      item = luaio_list_entry(pos, luaio_hash_item_t, node);
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

void luaio_hash_int_remove(luaio_hash_t *hash, size_t key) {
  size_t index = luaio_hash_slot(key, hash->bits);
  luaio_hlist_head_t *slot = &hash->slots[index];
  luaio_hlist_node_t *pos;
  luaio_hash_item_t *item;

  if (!luaio_hlist_is_empty(slot)) {
    luaio_hlist_for_each(pos, slot) {
      item = luaio_list_entry(pos, luaio_hash_item_t, node);
      if (item->hash == key) {
        luaio_hlist_remove(&item->node);
        luaio_pfree(item->pointer);
        luaio_pfree(item);
        hash->items--;
        return;
      }
    }
  }
}

void luaio_hash_int_set_value(luaio_hash_t *hash, size_t key, int value) {
  if (hash->items >= hash->max_items) {
    luaio_hash_rehash(hash);
  }

  size_t index = luaio_hash_slot(key, hash->bits);
  luaio_hlist_head_t *slot = &hash->slots[index];
  luaio_hlist_node_t *pos;
  luaio_hash_item_t *item;

  if (!luaio_hlist_is_empty(slot)) {
    luaio_hlist_for_each(pos, slot) {
      item = luaio_list_entry(pos, luaio_hash_item_t, node);
      if (item->hash == key) {
        item->value = value;
        return;
      }
    }
  }

  item = luaio_palloc(sizeof(luaio_hash_item_t));
  item->value = value;
  item->hash = key;
  luaio_hlist_insert_head(&item->node, slot);
  hash->items++;
}

int luaio_hash_int_get_value(luaio_hash_t *hash, size_t key, int *value) {
  size_t index = luaio_hash_slot(key, hash->bits);
  luaio_hlist_head_t *slot = &hash->slots[index];
  luaio_hlist_node_t *pos;
  luaio_hash_item_t *item;

  if (!luaio_hlist_is_empty(slot)) {
    luaio_hlist_for_each(pos, slot) {
      item = luaio_list_entry(pos, luaio_hash_item_t, node);
      if (item->hash == key) {
        *value = item->value;
        return 0;
      }
    }
  }

  return 1;
}

void luaio_hash_int_remove_value(luaio_hash_t *hash, size_t key) {
  size_t index = luaio_hash_slot(key, hash->bits);
  luaio_hlist_head_t *slot = &hash->slots[index];
  luaio_hlist_node_t *pos;
  luaio_hash_item_t *item;

  if (!luaio_hlist_is_empty(slot)) {
    luaio_hlist_for_each(pos, slot) {
      item = luaio_list_entry(pos, luaio_hash_item_t, node);
      if (item->hash == key) {
        luaio_hlist_remove(&item->node);
        luaio_pfree(item);
        hash->items--;
        return;
      }
    }
  }
}

int luaio_hash_int_get_and_remove_value(luaio_hash_t *hash, size_t key, int *value) {
  size_t index = luaio_hash_slot(key, hash->bits);
  luaio_hlist_head_t *slot = &hash->slots[index];
  luaio_hlist_node_t *pos;
  luaio_hash_item_t *item;

  if (!luaio_hlist_is_empty(slot)) {
    luaio_hlist_for_each(pos, slot) {
      item = luaio_list_entry(pos, luaio_hash_item_t, node);
      if (item->hash == key) {
        *value = item->value;
        luaio_hlist_remove(&item->node);
        luaio_pfree(item);
        hash->items--;
        return 0;
      }
    }
  }

  return 1;
}
