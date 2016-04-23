#ifndef SKIPLIST_INLINE_H
#define SKIPLIST_INLINE_H

#include "skiplist_types.h"

static inline skiplist_node_t *skiplist_begin( skiplist_t *skiplist )
{
	skiplist_node_t *begin = NULL;

	if( NULL != skiplist )
	{
		begin = skiplist->head.link[0].next;
	}

	return begin;
}

static inline skiplist_node_t *skiplist_end( void )
{
	return NULL;
}

static inline skiplist_node_t *skiplist_next( const skiplist_node_t *cur )
{
	skiplist_node_t *next = NULL;

	if( NULL != cur )
	{
		next = cur->link[0].next;
	}

	return next;
}

static inline uintptr_t skiplist_node_value( const skiplist_node_t *node )
{
	uintptr_t value = 0;

	if( NULL != node )
	{
		value = node->value;
	}

	return value;
}

static inline unsigned int skiplist_size( const skiplist_t *skiplist )
{
	unsigned int size = 0;

	if( NULL != skiplist )
	{
		size = skiplist->num_nodes;
	}

	return size;
}

#endif
