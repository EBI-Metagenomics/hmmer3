#ifndef CCO_H
#define CCO_H

/* ---- cco_compiler module begin ------------------------------------------- */
#ifndef __has_builtin
#define __has_builtin(x) (0)
#endif
/* ---- cco_compiler module end --------------------------------------------- */

/* ---- cco_of module begin ------------------------------------------------- */
#include <stddef.h>

/**
 * cco_of - cast a member of a structure out to the containing
 * structure
 * @ptr:	the pointer to the member.
 * @type:	the type of the container struct this is embedded in.
 * @member:	the name of the member within the struct.
 *
 */
#define cco_of(ptr, type, member)                                              \
  ({                                                                           \
    char *__mptr = (char *)(ptr);                                              \
    ((type *)(__mptr - offsetof(type, member)));                               \
  })

/**
 * cco_of_safe - cast a member of a structure out to the containing
 * structure
 * @ptr:	the pointer to the member.
 * @type:	the type of the container struct this is embedded in.
 * @member:	the name of the member within the struct.
 *
 * Return NULL if ptr is NULL.
 */
#define cco_of_safe(ptr, type, member)                                         \
  ({                                                                           \
    char *__mptr = (char *)(ptr);                                              \
    __mptr == NULL ? (type *)__mptr                                            \
                   : ((type *)(__mptr - offsetof(type, member)));              \
  })
/* ---- cco_of module end --------------------------------------------------- */

/* ---- cco_hash_support module begin --------------------------------------- */
#include <stdint.h>

/**
 * __cco_fls32 - find last (most-significant) bit set
 * @x: the word to search
 *
 * This is defined the same way as ffs.
 * Note __cco_fls32(1) = 1, __cco_fls32(0x80000000) = 32.
 *
 * Undefined if no set bit exists, so code should check against 0 first.
 */
static inline unsigned __cco_fls32(uint32_t x)
{

#if __has_builtin(__builtin_clz)
  return (unsigned)((int)sizeof(int) * 8 - __builtin_clz(x));
#else
  unsigned r = 32;

  if (!(x & 0xffff0000u))
  {
    x <<= 16;
    r -= 16;
  }
  if (!(x & 0xff000000u))
  {
    x <<= 8;
    r -= 8;
  }
  if (!(x & 0xf0000000u))
  {
    x <<= 4;
    r -= 4;
  }
  if (!(x & 0xc0000000u))
  {
    x <<= 2;
    r -= 2;
  }
  if (!(x & 0x80000000u))
  {
    x <<= 1;
    r -= 1;
  }
  return r;
#endif
}

/**
 * __cco_fls64 - find last (most-significant) set bit in a long word
 * @x: the word to search
 *
 * Undefined if no set bit exists, so code should check against 0 first.
 */
static inline unsigned __cco_fls64(uint64_t x)
{

#if __has_builtin(__builtin_clzl)
  uint32_t h = (uint32_t)(x >> 32);
  return h ? __cco_fls32(h) + 32 : __cco_fls32((uint32_t)x);
#else
  return (unsigned)((int)sizeof(long) * 8 - __builtin_clzl(x));
#endif
}

/*
 * Force a compilation error if condition is true, but also produce a
 * result (of value 0 and type int), so the expression can be used
 * e.g. in a structure initializer (or where-ever else comma expressions
 * aren't permitted).
 */
#define __CCO_BUILD_BUG_ON_ZERO(e) ((int)(sizeof(struct { int : (-!!(e)); })))

/* Are two types/vars the same type (ignoring qualifiers)? */
#define __cco_same_type(a, b)                                                  \
  __builtin_types_compatible_p(__typeof__(a), __typeof__(b))

/* &a[0] degrades to a pointer: a different type from an array */
#define __cco_must_be_array(a)                                                 \
  __CCO_BUILD_BUG_ON_ZERO(__cco_same_type((a), &(a)[0]))

#define __CCO_ARRAY_SIZE(arr)                                                  \
  (sizeof(arr) / sizeof((arr)[0]) + __cco_must_be_array(arr))

#define __CCO_UNSIGNED(x)                                                      \
  _Generic((x),                                                                \
      char: (unsigned char)(x),                                                \
      signed char: (unsigned char)(x),                                         \
      short: (unsigned short)(x),                                              \
      int: (unsigned int)(x),                                                  \
      long: (unsigned long)(x),                                                \
      long long: (unsigned long long)(x),                                      \
      default: (unsigned int)(x))

/**
 * __cco_ilog2 - log base 2 of 32-bit or a 64-bit unsigned value
 * @n: parameter
 *
 * constant-capable log of base 2 calculation
 * - this can be used to initialise global variables from constant data, hence
 * the massive ternary operator construction
 *
 * selects the appropriately-sized optimised version depending on sizeof(n)
 */
#define __cco_ilog2(x)                                                         \
  (__builtin_constant_p(x) ? ((x) < 2 ? 0 : 63 - __builtin_clzll(x))           \
   : sizeof(x) <= 4        ? __cco_ilog2_u32(x)                                \
                           : __cco_ilog2_u64(x))

static inline unsigned __cco_ilog2_u32(uint32_t n)
{
  return __cco_fls32(n) - 1;
}

static inline unsigned __cco_ilog2_u64(uint64_t n)
{
  return __cco_fls64(n) - 1;
}

#define __CCO_GOLDEN_RATIO_32 0x61C88647
#define __CCO_GOLDEN_RATIO_64 0x61C8864680B583EBull

static inline uint32_t cco_hash_32(uint32_t val, unsigned bits)
{
  /* High bits are more random, so use them. */
  return val * __CCO_GOLDEN_RATIO_32 >> (32 - bits);
}

static inline uint32_t cco_hash_64(uint64_t val, unsigned bits)
{
  /* High bits are more random, so use them. */
  return (uint32_t)(val * __CCO_GOLDEN_RATIO_64 >> (64 - bits));
}

/* Use cco_hash_32 when possible to allow for fast 32bit hashing in 64bit
 * kernels.
 */
#define cco_hash_min(x, bits)                                                  \
  (sizeof(x) <= 4 ? cco_hash_32(__CCO_UNSIGNED(x), bits)                       \
                  : cco_hash_64(__CCO_UNSIGNED(x), bits))

#define CCO_HASH_SIZE(name) (__CCO_ARRAY_SIZE(name))
#define CCO_HASH_BITS(name) __cco_ilog2(CCO_HASH_SIZE(name))
/* ---- cco_hash_support module end ----------------------------------------- */

/* ---- cco_node module begin ----------------------------------------------- */
#include <stddef.h>

struct cco_node
{
  struct cco_node *next;
};

#define CCO_NODE_INIT()                                                        \
  {                                                                            \
    NULL                                                                       \
  }

static inline void cco_node_add_next(struct cco_node *where,
                                     struct cco_node *novel)
{
  novel->next = where->next;
  where->next = novel;
}

static inline void cco_node_del(struct cco_node *prev, struct cco_node *node)
{
  prev->next = node->next;
  node->next = NULL;
}

static inline void cco_node_init(struct cco_node *node) { node->next = NULL; }
/* ---- cco_node module end ------------------------------------------------- */

/* ---- cco_hnode module begin ---------------------------------------------- */
#include <stddef.h>

struct cco_hnode
{
  struct cco_hnode *next, **pprev;
};

#define CCO_HNODE_INIT()                                                       \
  {                                                                            \
    NULL, NULL                                                                 \
  }

/**
 * cco_hnode_unhashed - Has node been removed from list and reinitialized?
 * @h: Node to be checked
 *
 * Not that not all removal functions will leave a node in unhashed
 * state.
 */
static inline int cco_hnode_unhashed(const struct cco_hnode *h)
{
  return !h->pprev;
}

static inline void __cco_hnode_del(struct cco_hnode *n)
{
  struct cco_hnode *next = n->next;
  struct cco_hnode **pprev = n->pprev;

  *pprev = next;
  if (next) next->pprev = pprev;
}

static inline void cco_hnode_init(struct cco_hnode *h)
{
  h->next = NULL;
  h->pprev = NULL;
}

/**
 * cco_hnode_del_init - Delete the specified hnode from its list and
 * initialize
 * @n: Node to delete.
 *
 * Note that this function leaves the node in unhashed state.
 */
static inline void cco_hnode_del_init(struct cco_hnode *n)
{
  if (!cco_hnode_unhashed(n))
  {
    __cco_hnode_del(n);
    cco_hnode_init(n);
  }
}
/* ---- cco_hnode module end ------------------------------------------------ */

/* ---- cco_hlist module begin ---------------------------------------------- */
struct cco_hlist
{
  struct cco_hnode *first;
};

#define CCO_HLIST_INIT(ptr) ((ptr)->first = NULL)

/**
 * cco_hlist_empty - Is the specified hlist structure an empty hlist?
 * @h: Structure to check.
 */
static inline int cco_hlist_empty(const struct cco_hlist *h)
{
  return !h->first;
}

/**
 * __cco_hlist_add - add a new entry at the beginning of the hlist
 * @n: new entry to be added
 * @h: hlist head to add it after
 *
 * Insert a new entry after the specified head.
 * This is good for implementing stacks.
 */
static inline void __cco_hlist_add(struct cco_hnode *n, struct cco_hlist *h)
{
  struct cco_hnode *first = h->first;
  n->next = first;
  if (first) first->pprev = &n->next;
  h->first = n;
  n->pprev = &h->first;
}

/**
 * cco_hlist_unhashed - Has node been removed from list and reinitialized?
 * @h: Node to be checked
 *
 * Not that not all removal functions will leave a node in unhashed
 * state.  For example, hlist_nulls_del_init_rcu() does leave the
 * node in unhashed state, but hlist_nulls_del() does not.
 */
static inline int cco_hlist_unhashed(struct cco_hnode const *h)
{
  return !h->pprev;
}

#define cco_hlist_entry(ptr, type, member) cco_of(ptr, type, member)

#define cco_hlist_entry_safe(ptr, type, member)                                \
  ({                                                                           \
    __typeof__(ptr) ____ptr = (ptr);                                           \
    ____ptr ? cco_hlist_entry(____ptr, type, member) : NULL;                   \
  })

/**
 * __cco_hlist_for_each_entry	- iterate over list of given type
 * @pos:	the type * to use as a loop cursor.
 * @head:	the head for your list.
 * @member:	the name of the hnode within the struct.
 */
#define __cco_hlist_for_each_entry(pos, head, member)                          \
  for (pos = cco_hlist_entry_safe((head)->first, __typeof__(*(pos)), member);  \
       pos; pos = cco_hlist_entry_safe((pos)->member.next, __typeof__(*(pos)), \
                                       member))

/**
 * __cco_hlist_for_each_entry_safe - iterate over list of given type safe
 * against removal of list entry
 * @pos:	the type * to use as a loop cursor.
 * @n:		a &struct hnode to use as temporary storage
 * @head:	the head for your list.
 * @member:	the name of the hnode within the struct.
 */
#define __cco_hlist_for_each_entry_safe(pos, n, head, member)                  \
  for (pos = cco_hlist_entry_safe((head)->first, __typeof__(*pos), member);    \
       pos && ({                                                               \
         n = pos->member.next;                                                 \
         1;                                                                    \
       });                                                                     \
       pos = cco_hlist_entry_safe(n, __typeof__(*pos), member))
/* ---- cco_hlist module end ------------------------------------------------ */

/* ---- cco_hash module begin ----------------------------------------------- */
#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>

#define CCO_HASH_DECLARE(name, bits) struct cco_hlist name[1 << (bits)]

/**
 * cco_hash_init - initialize a hash table
 * @ht: hashtable to be initialized
 *
 * Calculates the size of the hashtable from the given parameter, otherwise
 * same as hash_init_size.
 *
 * This has to be a macro since CCO_HASH_BITS() will not work on pointers since
 * it calculates the size during preprocessing.
 */
static inline void __cco_hash_init(struct cco_hlist *ht, unsigned sz)
{
  for (unsigned i = 0; i < sz; i++)
    CCO_HLIST_INIT(&ht[i]);
}

#define cco_hash_init(ht) __cco_hash_init(ht, CCO_HASH_SIZE(ht))

/**
 * cco_hash_add - add an object to a hashtable
 * @ht: hashtable to add to
 * @node: the &struct hnode of the object to be added
 * @key: the key of the object to be added
 */
#define cco_hash_add(ht, node, key)                                            \
  __cco_hlist_add(node, &ht[cco_hash_min(key, CCO_HASH_BITS(ht))])

static inline bool __cco_hash_empty(struct cco_hlist *ht, unsigned sz)
{
  for (unsigned i = 0; i < sz; i++)
    if (!cco_hlist_empty(&ht[i])) return false;

  return true;
}

/**
 * cco_hash_empty - check whether a hashtable is empty
 * @ht: hashtable to check
 *
 * This has to be a macro since CCO_HASH_BITS() will not work on pointers since
 * it calculates the size during preprocessing.
 */
#define cco_hash_empty(ht) __cco_hash_empty(ht, CCO_HASH_SIZE(ht))

/**
 * cco_hash_del - remove an object from a hashtable
 * @node: &struct hnode of the object to remove
 */
static inline void cco_hash_del(struct cco_hnode *node)
{
  cco_hnode_del_init(node);
}

/**
 * cco_hash_hashed - check whether an object is in any hashtable
 * @node: the &struct hlist_node of the object to be checked
 */
static inline bool cco_hash_hashed(struct cco_hnode const *node)
{
  return !cco_hlist_unhashed(node);
}

/**
 * cco_hash_for_each_safe - iterate over a hashtable safe against removal of
 * hash entry
 * @name: hashtable to iterate
 * @bkt: integer to use as bucket loop cursor
 * @tmp: a &struct hnode used for temporary storage
 * @obj: the type * to use as a loop cursor for each entry
 * @member: the name of the hnode within the struct
 */
#define cco_hash_for_each_safe(name, bkt, tmp, obj, member)                    \
  for ((bkt) = 0, obj = NULL; obj == NULL && (bkt) < CCO_HASH_SIZE(name);      \
       (bkt)++)                                                                \
  __cco_hlist_for_each_entry_safe(obj, tmp, &name[bkt], member)

/**
 * cco_hash_for_each_possible - iterate over all possible objects hashing to the
 * same bucket
 * @name: hashtable to iterate
 * @obj: the type * to use as a loop cursor for each entry
 * @member: the name of the hnode within the struct
 * @key: the key of the objects to iterate over
 */
#define cco_hash_for_each_possible(name, obj, member, key)                     \
  __cco_hlist_for_each_entry(                                                  \
      obj, &name[cco_hash_min(key, CCO_HASH_BITS(name))], member)

/**
 * cco_hash_for_each_possible_safe - iterate over all possible objects hashing
 * to the same bucket safe against removals
 * @name: hashtable to iterate
 * @obj: the type * to use as a loop cursor for each entry
 * @tmp: a &struct hnode used for temporary storage
 * @member: the name of the hnode within the struct
 * @key: the key of the objects to iterate over
 */
#define cco_hash_for_each_possible_safe(name, obj, tmp, member, key)           \
  __cco_hlist_for_each_entry_safe(                                             \
      obj, tmp, &name[cco_hash_min(key, CCO_HASH_BITS(name))], member)

/**
 * cco_hash_for_each - iterate over a hashtable
 * @name: hashtable to iterate
 * @bkt: integer to use as bucket loop cursor
 * @obj: the type * to use as a loop cursor for each entry
 * @member: the name of the hnode within the struct
 */
#define cco_hash_for_each(name, bkt, obj, member)                              \
  for ((bkt) = 0, obj = NULL; obj == NULL && (bkt) < CCO_HASH_SIZE(name);      \
       (bkt)++)                                                                \
  __cco_hlist_for_each_entry(obj, &name[bkt], member)
/* ---- cco_hash module end ------------------------------------------------- */

/* ---- cco_iter module begin ----------------------------------------------- */
struct cco_iter
{
  struct cco_node *curr;
  struct cco_node const *end;
};

static inline struct cco_node *cco_iter_next(struct cco_iter *iter)
{
  if (!iter || iter->curr == iter->end) return NULL;
  struct cco_node *node = iter->curr;
  iter->curr = node->next;
  return node;
}

#define cco_iter_entry(pos, type, member) cco_of_safe(pos, type, member)

#define cco_iter_next_entry(iter, entry, member)                               \
  cco_of_safe(cco_iter_next(iter), __typeof__(*entry), member)

#define cco_iter_for_each(pos, iter, member)                                   \
  for (pos = cco_iter_next(iter); pos; pos = cco_iter_next(iter))

#define cco_iter_for_each_safe(pos, tmp, iter, member)                         \
  for (pos = cco_iter_next(iter), tmp = cco_iter_next(iter); pos;              \
       pos = tmp, tmp = iter_next(iter))

#define cco_iter_for_each_entry(entry, iter, member)                           \
  for (entry = cco_iter_next_entry(iter, entry, member); entry;                \
       entry = cco_iter_next_entry(iter, entry, member))

#define cco_iter_for_each_entry_safe(entry, tmp, iter, member)                 \
  for (entry = cco_iter_next_entry(iter, entry, member),                       \
      tmp = cco_iter_next_entry(iter, entry, member);                          \
       entry; entry = tmp, tmp = cco_iter_next_entry(iter, entry, member))
/* ---- cco_iter module end ------------------------------------------------- */

/* ---- cco_queue module begin ---------------------------------------------- */
#include <stdbool.h>

struct cco_queue
{
  struct cco_node head;
  struct cco_node *tail;
};

#define CCO_QUEUE_INIT(name)                                                   \
  {                                                                            \
    (struct cco_node){&name.head}, &name.head                                  \
  }

static inline bool cco_queue_empty(struct cco_queue const *queue)
{
  return &queue->head == queue->tail;
}

static inline void cco_queue_init(struct cco_queue *queue)
{
  queue->tail = queue->head.next = &queue->head;
}

static inline struct cco_iter cco_queue_iter(struct cco_queue *queue)
{
  return (struct cco_iter){queue->tail, &queue->head};
}

static inline struct cco_node *cco_queue_pop(struct cco_queue *queue)
{
  struct cco_node *node = queue->tail;
  queue->tail = queue->tail->next;
  if (queue->tail == &queue->head) queue->head.next = &queue->head;
  return node;
}

static inline void cco_queue_put(struct cco_queue *queue,
                                 struct cco_node *novel)
{
  if (cco_queue_empty(queue)) queue->tail = novel;

  struct cco_node *next = queue->head.next;
  next->next = novel;
  novel->next = &queue->head;
  queue->head.next = novel;
}

static inline void cco_queue_put_first(struct cco_queue *queue,
                                       struct cco_node *novel)
{
  struct cco_node *tail = queue->tail;
  novel->next = tail;
  queue->tail = novel;
  if (queue->head.next == &queue->head) queue->head.next = novel;
}
/* ---- cco_queue module end ------------------------------------------------ */

/* ---- cco_stack module begin ---------------------------------------------- */
#include <stdbool.h>

struct cco_stack
{
  struct cco_node head;
};

#define CCO_STACK_INIT()                                                       \
  {                                                                            \
    CCO_NODE_INIT()                                                            \
  }

static inline bool cco_stack_empty(struct cco_stack const *stack)
{
  return stack->head.next == NULL;
}

static inline void cco_stack_init(struct cco_stack *stack)
{
  cco_node_init(&stack->head);
}

static inline struct cco_iter cco_stack_iter(struct cco_stack *stack)
{
  return (struct cco_iter){stack->head.next, NULL};
}

static inline struct cco_node *cco_stack_pop(struct cco_stack *stack)
{
  struct cco_node *node = stack->head.next;
  cco_node_del(&stack->head, node);
  return node;
}

static inline void cco_stack_put(struct cco_stack *stack,
                                 struct cco_node *novel)
{
  cco_node_add_next(&stack->head, novel);
}
/* ---- cco_stack module end ------------------------------------------------ */

#endif
