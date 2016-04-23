#ifndef SKIPLIST_INLINE_H
#define SKIPLIST_INLINE_H

#include "skiplist_types.h"

static inline skiplist_node_t *skiplist_begin( skiplist_t *skiplist )
{
	assert( skiplist );
	return skiplist->head.link[0].next;
}

static inline skiplist_node_t *skiplist_end( void )
{
	return NULL;
}

static inline skiplist_node_t *skiplist_next( const skiplist_node_t *cur )
{
	assert( cur );
	return cur->link[0].next;
}

static inline uintptr_t skiplist_node_value( const skiplist_node_t *node )
{
	assert( node );
	return node->value;
}

static inline unsigned int skiplist_size( const skiplist_t *skiplist )
{
	assert( skiplist );
	return skiplist->num_nodes;
}

#endif
