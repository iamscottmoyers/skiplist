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

	/* Arbitrary initialization values for the random number generator.
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

static skiplist_node_t *skiplist_node_allocate( unsigned int levels )
{
	skiplist_node_t *node;

	/* Allocate a node with space at the end for each level link.
	   levels - 1 is used as one link is included in the size of skiplist_node_t. */
	node = malloc( sizeof( skiplist_node_t ) + sizeof( skiplist_link_t ) * (levels - 1) );

	return node;
}

static void skiplist_node_deallocate( skiplist_node_t *node )
{
	assert( node );

	free( node );
}

static void skiplist_node_init( skiplist_node_t *node, unsigned int levels, uintptr_t value )
{
	assert( node );

	node->levels = levels;
	node->value = value;
}

static skiplist_node_t *skiplist_node_create( unsigned int levels, uintptr_t value )
{
	skiplist_node_t *node;

	node = skiplist_node_allocate( levels );

	if( NULL != node )
	{
		skiplist_node_init( node, levels, value );
	}

	return node;
}

static skiplist_t *skiplist_allocate( unsigned int size_estimate_log2 )
{
	skiplist_t *skiplist;

	/* number of links - 1 to take into account the 1 sized array at the end. */
	skiplist = malloc( sizeof( skiplist_t ) + sizeof( skiplist_link_t ) * (size_estimate_log2 - 1) );

	return skiplist;
}

static void skiplist_deallocate( skiplist_t *skiplist )
{
	assert( skiplist );

	free( skiplist );
}

void skiplist_init( skiplist_t *skiplist,
                    skiplist_properties_t properties, unsigned int size_estimate_log2,
                    skiplist_compare_pfn compare, skiplist_fprintf_pfn print )
{
	assert( skiplist );

	skiplist_rng_init( &skiplist->rng );
	skiplist->properties = properties;
	skiplist->compare = compare;
	skiplist->print = print;
	skiplist->num_nodes = 0;
	skiplist->head.levels = size_estimate_log2;
	memset( skiplist->head.link, 0, sizeof( skiplist_link_t ) * size_estimate_log2 );
}

static skiplist_t *skiplist_create_clean( skiplist_properties_t properties, unsigned int size_estimate_log2,
                                            skiplist_compare_pfn compare, skiplist_fprintf_pfn print )
{
	skiplist_t *skiplist;

	skiplist = skiplist_allocate( size_estimate_log2 );

	if( NULL != skiplist )
	{
		skiplist_init( skiplist, properties, size_estimate_log2, compare, print );
	}

	return skiplist;
}

static int skiplist_create_is_clean( skiplist_properties_t properties, unsigned int size_estimate_log2,
                                     skiplist_compare_pfn compare, skiplist_fprintf_pfn print )
{
	if( size_estimate_log2 <= 0 )
	{
		return 0;
	}

	if( size_estimate_log2 > SKIPLIST_MAX_LINKS )
	{
		return 0;
	}

	if( NULL == compare )
	{
		return 0;
	}

	if( NULL == print )
	{
		return 0;
	}

	if( SKIPLIST_PROPERTY_NONE != properties && SKIPLIST_PROPERTY_UNIQUE != properties )
	{
		return 0;
	}

	return 1;
}

skiplist_t *skiplist_create( skiplist_properties_t properties, unsigned int size_estimate_log2,
                             skiplist_compare_pfn compare, skiplist_fprintf_pfn print )
{
	skiplist_t *skiplist = NULL;

	if( skiplist_create_is_clean( properties, size_estimate_log2, compare, print ) )
	{
		skiplist = skiplist_create_clean( properties, size_estimate_log2, compare, print );
	}

	return skiplist;
}

void skiplist_destroy( skiplist_t *skiplist )
{
	if( NULL != skiplist )
	{
		skiplist_node_t *cur;
		skiplist_node_t *next;

		for( cur = skiplist->head.link[0].next; NULL != cur; cur = next )
		{
			next = cur->link[0].next;
			skiplist_node_deallocate( cur );
		}

		skiplist_deallocate( skiplist );
	}
}

unsigned int skiplist_contains( const skiplist_t *skiplist, uintptr_t value )
{
	unsigned int i;
	const skiplist_node_t *cur;

	cur = &skiplist->head;
	for( i = cur->levels; i-- != 0; )
	{
		for( ; NULL != cur->link[i].next; cur = cur->link[i].next )
		{
			int comparison = skiplist->compare( cur->link[i].next->value, value );
			if( comparison > 0 )
			{
				break;
			}
			else if( 0 == comparison )
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

static void skiplist_find_insert_path( skiplist_t *skiplist, uintptr_t value,
                                       skiplist_node_t *path[], unsigned int distances[] )
{
	unsigned int i;
	skiplist_node_t *cur;

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
			if( skiplist->compare( cur->link[i].next->value, value ) > 0 )
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

int skiplist_insert( skiplist_t *skiplist, uintptr_t value )
{
	skiplist_node_t *update[SKIPLIST_MAX_LINKS];
	unsigned int distances[SKIPLIST_MAX_LINKS];
	int err = 0;

	assert( skiplist );

	skiplist_find_insert_path( skiplist, value, update, distances );

	/* Insert the new value, unless this is a skiplist set that already contains it. */
	if( SKIPLIST_PROPERTY_NONE == skiplist->properties ||
	    update[0] == &skiplist->head || skiplist->compare( update[0]->value, value ) )
	{
		unsigned int node_levels;
		skiplist_node_t *new_node;

		node_levels = skiplist_compute_node_level( skiplist );
		new_node = skiplist_node_create( node_levels, value );

		if( NULL == new_node )
		{
			err = -1;
		}
		else
		{
			unsigned int i;

			/* Increment the width of each link that jumps over this node. */
			for( i = skiplist->head.levels; i-- != node_levels; )
			{
				++update[i]->link[i].width;
			}

			/* Insert the node into each level of the skiplist. */
			for( i = node_levels; i-- != 0; )
			{
				skiplist_link_t *update_link = &update[i]->link[i];
				skiplist_link_t *new_link = &new_node->link[i];

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

static void skiplist_find_remove_path( skiplist_t *skiplist, uintptr_t value, skiplist_node_t *path[] )
{
	unsigned int i;
	skiplist_node_t *cur;

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
			if( skiplist->compare( cur->link[i].next->value, value ) >= 0 )
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

int skiplist_remove( skiplist_t *skiplist, uintptr_t value )
{
	skiplist_node_t *update[SKIPLIST_MAX_LINKS];
	skiplist_node_t *remove;
	int err = 0;

	assert( skiplist );

	/* Find all levels that span over the node to remove. */
	skiplist_find_remove_path( skiplist, value, update );

	remove = update[0]->link[0].next;
	if( skiplist->compare( remove->value, value ) )
	{
		err = -1;
	}
	else
	{
		unsigned int i;

		for( i = skiplist->head.levels; i-- != 0; )
		{
			skiplist_link_t *update_link = &update[i]->link[i];
			skiplist_link_t *remove_link = &remove->link[i];

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

		/* Deallocate the memory for the removed node. */
		skiplist_node_deallocate( remove );

		/* Decrement node counter. */
		--skiplist->num_nodes;
	}

	return err;
}

void skiplist_fprintf( FILE *stream, const skiplist_t *skiplist )
{
	const skiplist_node_t *cur;
	unsigned int i;

	assert( stream );
	assert( skiplist );

	fprintf( stream, "digraph {\n" );
	fprintf( stream, "rankdir=\"LR\"\n" );
	for( i = skiplist->head.levels; i-- != 0; )
	{
		for( cur = &skiplist->head; NULL != cur; cur = cur->link[i].next )
		{
			assert( i < cur->levels );

			if( cur == &skiplist->head )
			{
				fprintf( stream, "\"HEAD\\lnum_nodes: %u\"", skiplist->num_nodes );
			}
			else
			{
				fprintf( stream, "\"%p\\lvalue: ", (void *)cur );
				skiplist->print( stream, cur->value );
				fprintf( stream, "\"" );
			}

			fprintf( stream, "->" );

			if( NULL == cur->link[i].next )
			{
				fprintf( stream, "TAIL" );
			}
			else
			{
				fprintf( stream, "\"%p\\lvalue: ", (void *)cur->link[i].next );
				skiplist->print( stream, cur->link[i].next->value );
				fprintf( stream, "\"" );
			}

			fprintf( stream, "[ label=\"%u\" ];\n", cur->link[i].width );
		}
	}

	fprintf( stream, "}\n" );
}

void skiplist_printf( const skiplist_t *skiplist )
{
	skiplist_fprintf( stdout, skiplist );
}

int skiplist_fprintf_filename( const char *filename, const skiplist_t *skiplist )
{
	int err = -1;
	FILE *fp;

	assert( filename );
	assert( skiplist );

	fp = fopen( filename, "w" );
	if( NULL != fp )
	{
		skiplist_fprintf( fp, skiplist );
		fclose( fp );
		err = 0;
	}

	return err;
}

uintptr_t skiplist_at_index( const skiplist_t *skiplist, unsigned int index )
{
	unsigned int i;
	unsigned int remaining;
	const skiplist_node_t *cur;

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
