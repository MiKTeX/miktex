
/*
    pyavl -- HEADER FILE "avl.h"
    Interface to manipulate "objects" of type 'avl_tree' and 'avl_iterator'
*/

#ifndef __AVL__
#define __AVL__

#include <stdarg.h>
#include <stdio.h>
#include <stdlib.h>

#define avl_del mp_avl_del
#define avl_ins mp_avl_ins
#define avl_tree mp_avl_tree
#define avl_entry mp_avl_entry
#define avl_find mp_avl_find
#define avl_create mp_avl_create
#define avl_destroy mp_avl_destroy

typedef enum
{ avl_false, avl_true } avl_bool_t;

#ifndef MPW_C
#include <inttypes.h>
typedef int8_t avl_code_t;
typedef int8_t avl_bal_t;
typedef uint32_t avl_size_t;
#else
#include <MacTypes.h>
typedef SInt8 avl_code_t;
typedef SInt8 avl_bal_t;
typedef UInt32 avl_size_t;
#endif

typedef int (*avl_compare_func) (void *param, const void *lhs,
								 const void *rhs);
typedef void *(*avl_item_copy_func) (const void *item);
typedef void *(*avl_item_dispose_func) (void *item);
typedef void (*avl_item_func) (const void *item, void *param);
typedef void *(*avl_alloc_func) (size_t);
typedef void (*avl_dealloc_func) (void *);

#ifdef      AVL_FOR_PYTHON
#undef        AVL_CMPERR
#undef		  AVL_NULLCHECKS
#define       AVL_CMPERR 1
#define       AVL_NULLCHECKS 1
#else
#ifndef		AVL_CMPERR
#define       AVL_CMPERR 0
#endif
#ifndef		AVL_NULLCHECKS
#define       AVL_NULLCHECKS 0
#endif
#endif

#if AVL_CMPERR != 0
extern avl_code_t avl_errcmp_occurred(void *);
#endif

/* At minimum, shallow copy */

const void *avl_default_item_copy(const void *);
void *avl_default_item_dispose(void *);

#define AVL_STACK_CAPACITY  32	/* for avl_split() function */

typedef enum
{
  AVL_ITERATOR_INI_PRE,
  AVL_ITERATOR_INI_POST,
  AVL_ITERATOR_INI_INTREE
} avl_ini_t;

typedef struct avl_tree_ *avl_tree;
typedef struct avl_iterator_ *avl_iterator;

typedef struct avl_itersource_ avl_itersource_struct, *avl_itersource;

struct avl_itersource_
{
  void *p;
  /* return nonzero on error */
    avl_code_t(*f) (avl_itersource from, void **to);
};

typedef struct
{
  avl_compare_func compare;
  avl_item_copy_func copy;
  avl_item_dispose_func dispose;
  avl_alloc_func alloc;
  avl_dealloc_func dealloc;
} avl_config_struct, *avl_config;

/* -------------------------------------------------------------------------------------------------/
                                        Public Functions
---------------------------------------------------------------------------------------------------*/

/*
    --- CREATE ---
    Return a new tree and set its config.
    Return NULL on allocation failure.
	* 'alloc' defaults to malloc from stdlib
 	* 'dealloc' defaults to free from stdlib
	* 'param' user param/refcon
*/

avl_tree avl_create(avl_compare_func compare,
					avl_item_copy_func copy,
					avl_item_dispose_func dispose,
					avl_alloc_func alloc,
					avl_dealloc_func dealloc, void *param);

/*
    --- RESET ---
    Empty tree 't' as in 'avl_empty()' and modify its config.
*/

void
avl_reset(avl_tree t,
		  avl_compare_func compare,
		  avl_item_copy_func copy,
		  avl_item_dispose_func dispose,
		  avl_alloc_func alloc, avl_dealloc_func dealloc);

/*
    --- EMPTY ---
    Empty tree 't', calling its dispose_func for each item in 't'.
    The config is untouched.
*/

void avl_empty(avl_tree t);

/*
    --- DESTROY ---
    Empty tree 't' and free the handle.
*/

void avl_destroy(avl_tree t);

/*
    --- DUPLICATE (COPY) ---
    Return a copy of tree 't', using its copy_func for each item in 't'.
    Upon failure to allocate room for some item, return NULL.
*/

avl_tree avl_dup(avl_tree t, void *param);

/*
    --- EMPTYNESS ---
    Return 'avl_true' iff tree 't' is empty (i.e. the handle is NULL or 't' contains no item).
*/

avl_bool_t avl_isempty(avl_tree t);

/*
    --- SIZE ---
    Return number of items contained in tree 't'.
*/

avl_size_t avl_size(avl_tree t);

/*
    --- FIRST (MINIMUM) ---
    Return first item in in-order traversal of 't'.
    Return NULL if 't' is empty.
*/

void *avl_first(avl_tree t);

/*
    --- LAST (MAXIMUM) ---
    Return last item in in-order traversal of 't'.
    Return NULL if 't' is empty.
*/

void *avl_last(avl_tree t);

/*
    --- FIND MATCHING ITEM ---
    Find item matching 'item' parameter in tree 't'.
    Return NULL if it's not found.
    If there are multiple matches, the first one that is encountered
    during the search is returned; it may not be the one with lowest rank.
*/

void *avl_find(const void *item, avl_tree t);

/*
    --- INDEX (RANK) OF ITEM ---
    Return smallest index 'i' s.t. 't[i]' matches 'item',
    or zero if 'item' is not found.
*/

avl_size_t avl_index(const void *item, avl_tree t);

/*
    --- SPAN ITEMS ---
    Return integers 'i,j' s.t. 't[i,j]'
    i smallest index s.t. t[i] >= lo_item, or t->count+1 and
    j greatest one s.t. t[j] <= hi_item, or 0.
    If 'hi_item' is less than 'lo_item' those are swapped.
    Return codes:
 		 0		success
 		-1		error: tree had no root
 		-2		error: compare failed
*/

avl_code_t
avl_span(const void *lo_item,
		 const void *hi_item,
		 avl_tree t, avl_size_t * lo_idx, avl_size_t * hi_idx);

/*
    --- FIND AT LEAST ---
    Return smallest item in 't' that is GEQ 'item', or NULL.
*/

void *avl_find_atleast(const void *item, avl_tree t);

/*
    --- FIND AT MOST ---
    Return largest item in 't' that is LEQ 'item', or NULL.
*/

void *avl_find_atmost(const void *item, avl_tree t);

/*
    --- FIND BY INDEX (RANK) ---
    Find item in 't' by index, that is return 't[idx]'.
    If 'idx' is not in '[1,avl_size(t)]' then return NULL.
 	If a compare failed then return NULL.
*/

void *avl_find_index(avl_size_t idx, avl_tree t);

/*
    --- INSERTION ---
    Insert 'item' in tree 't' with regard to its compare_func.
    Say 'avl_ins(item,t,avl_true)' to insert 'item' in 't'
    even if it is there already.
    If 'item' is a duplicate and 'allow_duplicates' is avl_false,
    nothing is done.
    Return codes:
            -1      error: allocation of new node failed
			-2		error: compare failed, tree unchanged
             0      nothing was done, no error
            +1      operation successful
            +2      the same and height(t) increased by one.
*/

avl_code_t avl_ins(void *item, avl_tree t, avl_bool_t allow_duplicates);

/*
    --- DELETION ---
    Remove 'item' from tree 't', calling its dispose_func.
    To make a backup of 'item' involving its copy_func,
    say 't(item,backup)' where 'backup' is some pointer to pointer to item.
    Otherwise set it to NULL.
    Return codes:
             0      item not found
			-2		error: compare failed, tree unchanged
            +1      operation successful
            +2      the same and height(t) decreased by one.
*/

avl_code_t avl_del(void *item, avl_tree t, void **backup);

/*
    --- DELETE FIRST ---
    Remove first item in in-order traversal from tree 't'.
    Note that only one item is removed.
    Return +1 or +2 as above.
*/

avl_code_t avl_del_first(avl_tree t, void **backup);

/*
    --- DELETE LAST ---
    Remove last item in in-order traversal from tree 't'.
    Note that only one item is removed.
    Return +1 or +2 as above.
*/

avl_code_t avl_del_last(avl_tree t, void **backup);

/*
    --- INSERT IN FRONT OF INDEX ---
    Insert 'item' in tree 't' so that afterwards,
    't[idx]=item' except if 'idx<=0' or 'idx>size(t)+1'.
    To append 'item' to 't' regardless of order, 
    say 'avl_ins_index(item,size+1,t)'.
*/

avl_code_t avl_ins_index(void *item, avl_size_t idx, avl_tree t);

/*
    --- DELETE ITEM BY INDEX ---
    Remove item of rank 'idx' from tree 't' and 
    return +1 or +2 as above except if 'idx' is not in
    '[1,avl_size(t)]' in which case return 0.
*/

avl_code_t avl_del_index(avl_size_t idx, avl_tree t, void **backup);

/*
    --- IN-PLACE CONCATENATION ---
    Pre-condition: 't0' and 't1' are valid avl_trees
    Note that the code does not check whether the maximal item in 't0' is LEQ than
    the minimal item in 't1'.
    Post-condition: 't0' handles the concatenation of
    't0' and 't1' which becomes empty (but its config is untouched).
*/

void avl_cat(avl_tree t0, avl_tree t1);

/*
    --- SPLITTING ---
    Pre-condition: 't0' and 't1' are existing handles.
    Post-condition: items in 't0' all compare LEQ than 'item'
    and items in 't1' all compare GEQ than 'item'.
    This implementation removes one item.
 	Return codes:
 		 0		item not found, no-op
 		-2		compare failed, tree unchanged
 		+1		success
*/

avl_code_t avl_split(const void *item, avl_tree t, avl_tree t0, avl_tree t1);

/*
    --- IN-ORDER TRAVERSAL ---
    Walk tree 't' in in-order, applying 'proc' at each node.
    The 'param' pointer is passed to 'proc', like this:
    '(*proc) (item_at_node,param)'.
*/

void avl_walk(avl_tree t, avl_item_func proc, void *param);

/*
    --- SLICE ---
    Create a _new tree_ from the slice 't[lo_idx,hi_idx)'
    provided 'lo_idx <= hi_idx' and these indices
    are both in range. If a new tree can't be created
    or if some item can't be allocated, return NULL.
    Otherwise if the indices are inconsistent return NULL.
*/

avl_tree
avl_slice(avl_tree t, avl_size_t lo_idx, avl_size_t hi_idx, void *param);

/* ----------------------------------------------------------/
                            ITERATORS
                        
    An iterator assigned to a tree 't' is still usable after
    any item is inserted into 't' and after any item
    not located at this iterator's current position is
    deleted. The 'avl_iterator_del()' function may be used
    to remove the item at the iterator's current position.
------------------------------------------------------------*/

/*
    --- ITERATOR --- SEEK
    Find 'item' in this iterator's tree as in 'avl_find()'
    and make it the current position.
*/

void avl_iterator_seek(const void *item, avl_iterator iter);

/*
    --- ITERATOR --- COUNT
    Return size of this iterator's tree
*/

avl_size_t avl_iterator_count(avl_iterator iter);

/*
    --- ITERATOR --- SEEK BY INDEX
    Set the current position of 'iter' to 't[idx]'
    where 't' is the tree that is iterated over.
*/

void avl_iterator_seek_index(avl_size_t idx, avl_iterator iter);

/*
    --- ITERATOR --- CURRENT POSITION
    Return item at current position of 'iter'.
*/

void *avl_iterator_cur(avl_iterator iter);

/*
    --- ITERATOR --- INDEX
    Return rank of current item of 'iter' (as a result of computation)
    except it returns 0 or size of tree plus one if 'iter' is a pre- or post- iterator.
*/

avl_size_t avl_iterator_index(avl_iterator iter);

/*
    --- ITERATOR --- CREATE
    Return a new cursor for tree 't'. 
    If allocation of an iterator struct is impossible, return NULL.
    Say 'avl_iterator_new(t, ini)' with 'ini==AVL_ITERATOR_INI_PRE' or 'ini==AVL_ITERATOR_INI_POST'
    or say 'avl_iterator_new(t, AVL_ITERATOR_INI_INTREE, item_pointer)'
    to set the iterator's current position via 'avl_iterator_seek(item_pointer,the_iterator)'.
    In the latter case, the iterator is flagged
    as pre-iterator if the item is not found.
*/

avl_iterator avl_iterator_new(avl_tree t, avl_ini_t ini, ...);

/*
    --- ITERATOR --- KILL
    Cleanup: free the iterator struct.
*/

void avl_iterator_kill(avl_iterator iter);

/*
    --- ITERATOR --- SUCCESSOR
    Get next item pointer in iterator or NULL.
    'iter' is flagged as post-iterator if it's in post-position.
*/

void *avl_iterator_next(avl_iterator iter);

/*
    --- ITERATOR --- PREDECESSOR
    Get next item pointer in iterator or NULL.
    'iter' is flagged as pre-iterator if it's in pre-position.
*/

void *avl_iterator_prev(avl_iterator iter);

/*
    --- ITERATOR --- DELETION
    Remove item at current position of iterator 'iter' from its tree, if there is one.
    Current position is set to next item or iterator is flagged as post-iterator.
*/

avl_code_t avl_iterator_del(avl_iterator iter, void **backup);

/*
    --- VERIFICATION ---
    Return avl_true iff 't' is a valid avl_tree.
    Note that 'avl_verify(NULL)==avl_false'.
*/

#ifdef HAVE_AVL_VERIFY
avl_bool_t avl_verify(avl_tree t);
#endif /* HAVE_AVL_VERIFY */

/*
    --- LOAD ---
    More general version of avl_slice
*/

avl_tree
avl_xload(avl_itersource src,
		  void **pres, avl_size_t len, avl_config conf, void *param);

#endif /* __AVL__ */
