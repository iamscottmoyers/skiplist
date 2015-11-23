#include <stdio.h>
#include <assert.h>
#include <stdlib.h>
#include <string.h>

#include "skiplist.h"

/**
 * @brief Count the number of leading zeros in the given number.
 *
 * @param [in] n  The number to count the leading zeros for.
 *
 * @return The number of leading zeros in 'n'.
 */
static unsigned int clz( unsigned int n )
{
	return __builtin_clz( n );
}

/**
 * @brief Initialize the skiplist's random number generator
 *
 * @param [out] rng  The random number generator state.
 */
static void skiplist_rng_init( skiplist_rng_t *rng )
{
	assert( rng );

	/* Arbitrary initialization values the random number generator.
	   These should never be set to 0. */
	rng->m_w = 0xcafef00d;
	rng->m_z = 0xabcd1234;
}

/**
 * @brief Generate a random 32 bit number
 *
 * This is a simple multiply-with-carry pseudo-random number generator.
 * Speed is the main factor for picking a good algorithm here rather than
 * the periodicity. Also, the algorithm should ideally have the property
 * that each bit is equally likely to be a 1 or a 0 on each invocation.
 *
 * @param [in,out] rng  The random number generator state.
 *
 * @return              A random 32 bit number.
 */
static unsigned int skiplist_rng_gen_u32( skiplist_rng_t *rng )
{
	assert( rng );

	rng->m_z = 36969 * (rng->m_z & 65535) + (rng->m_z >> 16);
	rng->m_w = 18000 * (rng->m_w & 65535) + (rng->m_w >> 16);

	return (rng->m_z << 16) + rng->m_w;
}

skiplist_t *skiplist_create( unsigned int size_estimate_log2 )
{
	skiplist_t *skiplist;

	assert( size_estimate_log2 > 0 );
	assert( size_estimate_log2 < MAX_LIST_DEPTH );

	skiplist = malloc( sizeof( skiplist_t ) + sizeof( link_t ) * (size_estimate_log2 - 1) );
	if( NULL != skiplist )
	{
		skiplist_rng_init( &skiplist->rng );
		skiplist->num_nodes = 0;
		skiplist->head.levels = size_estimate_log2;
		memset( skiplist->head.link, 0, sizeof( link_t ) * size_estimate_log2 );
	}

	return skiplist;
}

void skiplist_destroy( skiplist_t *skiplist )
{
	if( NULL != skiplist )
	{
		node_t *cur;
		node_t *next;

		for( cur = skiplist->head.link[0].next; NULL != cur; cur = next )
		{
			next = cur->link[0].next;
			free( cur );
		}

		free( skiplist );
	}
}

unsigned int skiplist_contains( const skiplist_t *skiplist, unsigned int value )
{
	unsigned int i;
	const node_t *cur;

	cur = &skiplist->head;
	for( i = cur->levels; i-- != 0; )
	{
		for( ; NULL != cur->link[i].next; cur = cur->link[i].next )
		{
			if( cur->link[i].next->value > value )
			{
				break;
			}
			else if( cur->link[i].next->value == value )
			{
				return 1;
			}
		}
	}

	return 0;
}

static unsigned int skiplist_compute_node_level( skiplist_t *skiplist )
{
	unsigned int node_levels;

	/* The number of levels that we insert the node into is
	   calculated using the number of leading zeros in a random number.

	   Assuming each bit is equally likely to be a 0 or a 1 then
	   each successive level will have half the probability of being
	   chosen than the previous one. This gives us the correct
	   distribution for O(log(n)) insertion. */
	node_levels = clz( skiplist_rng_gen_u32( &skiplist->rng ) ) + 1;
	if( node_levels > skiplist->head.levels )
	{
		node_levels = skiplist->head.levels;
	}

	return node_levels;
}

static void skiplist_find_insert_path( skiplist_t *skiplist, unsigned int value,
                                       node_t *path[], unsigned int distances[] )
{
	unsigned int i;
	node_t *cur;

	/* 'value' will be positioned before the first node that is greater than 'value'.
	   Start searching from the highest level, this level spans the most number of
	   nodes per next pointer. */
	cur = &skiplist->head;
	for( i = cur->levels; i-- != 0; )
	{
		assert( i < cur->levels );
		distances[i] = 1;

		/* Search through the current level in the skiplist... */
		while( NULL != cur->link[i].next )
		{
			unsigned int j;
			assert( i < cur->levels );

			/* ... until we find a value greater
			   than our input value... */
			if( cur->link[i].next->value > value )
			{
				/* ... then move on to the lower levels. */
				break;
			}

			/* Increment the distance from previous nodes... */
			for( j = i + 1; j < skiplist->head.levels; ++j )
			{
				distances[j] += cur->link[i].width;
			}

			/* ... and advance the next pointer. */
			cur = cur->link[i].next;
		}

		/* Store the path to the insertion point, so we can update all list
		   entries if the value needs to be inserted. */
		path[i] = cur;
	}
}

int skiplist_insert( skiplist_t *skiplist, unsigned int value )
{
	node_t *update[MAX_LIST_DEPTH];
	unsigned int distances[MAX_LIST_DEPTH];
	int err = 0;

	assert( skiplist );

	skiplist_find_insert_path( skiplist, value, update, distances );

	/* Only insert the new value if the set doesn't already contain it. */
	if( update[0] == &skiplist->head || update[0]->value != value )
	{
		unsigned int node_levels;
		node_t *new_node;

		node_levels = skiplist_compute_node_level( skiplist );

		/* Allocate a node with space at the end for each level link.
		   node_levels - 1 is used as one link is included in the size of node_t. */
		new_node = malloc( sizeof( node_t ) + sizeof( link_t ) * (node_levels - 1) );
		if( NULL == new_node )
		{
			err = -1;
		}
		else
		{
			unsigned int i;

			/* Populate node data. */
			new_node->value = value;
			new_node->levels = node_levels;

			for( i = skiplist->head.levels; i-- != node_levels; )
			{
				++update[i]->link[i].width;
			}

			/* Insert the node into each level of the skiplist. */
			for( i = node_levels; i-- != 0; )
			{
				link_t *update_link = &update[i]->link[i];
				link_t *new_link = &new_node->link[i];

				/* Update the link widths using the distance we are from the previous level. */
				new_link->width = 1 + update_link->width - distances[i];
				update_link->width = distances[i];

				/* Update the next pointers. */
				new_link->next = update_link->next;
				update_link->next = new_node;
			}

			/* Increment node counter. */
			++skiplist->num_nodes;
		}
	}

	return err;
}

static void skiplist_find_remove_path( skiplist_t *skiplist, unsigned int value, node_t *path[] )
{
	unsigned int i;
	node_t *cur;

	/* Find the path to a node that contains 'value'. */
	cur = &skiplist->head;
	for( i = cur->levels; i-- != 0; )
	{
		assert( i < cur->levels );

		/* Search through the current level in the skiplist... */
		while( NULL != cur->link[i].next )
		{
			assert( i < cur->levels );

			/* ... until we find a value greater
			   than or equal to our input value... */
			if( cur->link[i].next->value >= value )
			{
				/* ... then move on to the lower levels. */
				break;
			}

			/* ... and advance the next pointer. */
			cur = cur->link[i].next;
		}

		/* Store the path to the node before the node we need to remove.
		   These pointers need updating to point to the node after the one being removed. */
		path[i] = cur;
	}
}

int skiplist_remove( skiplist_t *skiplist, unsigned int value )
{
	node_t *update[MAX_LIST_DEPTH];
	node_t *remove;
	int err = 0;

	assert( skiplist );

	/* Find all levels that span over the node to remove. */
	skiplist_find_remove_path( skiplist, value, update );

	remove = update[0]->link[0].next;
	if( remove->value != value )
	{
		err = -1;
	}
	else
	{
		unsigned int i;

		for( i = skiplist->head.levels; i-- != 0; )
		{
			link_t *update_link = &update[i]->link[i];
			link_t *remove_link = &remove->link[i];

			/* This level will either connect to the node after the removed node or span over it.
			   If it spans over the removed node just decrement the width of the link, if it
			   connects then update the next pointer and sum the link widths. */
			--update_link->width;
			if( update_link->next == remove )
			{
				update_link->next = remove_link->next;
				update_link->width += remove_link->width;
			}
		}

		/* Free the removed pointer. */
		free( remove );

		/* Decrement node counter. */
		--skiplist->num_nodes;
	}

	return err;
}

void skiplist_fprintf( FILE *stream, const skiplist_t *skiplist )
{
	const node_t *cur;
	unsigned int i;

	assert( stream );
	assert( skiplist );

	printf("digraph {\n");
	printf("rankdir=\"LR\"\n");
	for( i = skiplist->head.levels; i-- != 0; )
	{
		for( cur = &skiplist->head; NULL != cur; cur = cur->link[i].next )
		{
			assert( i < cur->levels );

			if( cur == &skiplist->head )
			{
				printf( "\"HEAD\\lnum_nodes: %u\"", skiplist->num_nodes );
			}
			else
			{
				printf( "\"%p\\lvalue: %u\"", (void *)cur, cur->value );
			}

			printf( "->" );

			if( NULL == cur->link[i].next )
			{
				printf( "TAIL" );
			}
			else
			{
				printf( "\"%p\\lvalue: %u\"", (void *)cur->link[i].next, cur->link[i].next->value );
			}

			printf( "[ label=\"%u\" ];\n", cur->link[i].width );
		}
	}

	printf("}\n");
}

void skiplist_printf( const skiplist_t *skiplist )
{
	skiplist_fprintf( stdout, skiplist );
}

unsigned int skiplist_at_index( const skiplist_t *skiplist, unsigned int index )
{
	unsigned int i;
	unsigned int remaining;
	const node_t *cur;

	assert( index < skiplist->num_nodes );

	/* Indicies in the skiplist start counting from 1 due to the 1 step distance
	   from head to the first element. So increment the index by 1. */
	remaining = index + 1;
	cur = &skiplist->head;
	for( i = cur->levels; i-- != 0 && remaining > 0; )
	{
		/* If we've reached the tail without finding the index or the next step is too far away
		   try the next level down. */
		while( NULL != cur->link[i].next && cur->link[i].width <= remaining )
		{
			/* Otherwise, decrement the width remaining and move to the next node. */
			remaining -= cur->link[i].width;
			cur = cur->link[i].next;
		}
	}

	return cur->value;
}
