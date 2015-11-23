#ifndef SKIPLIST_TYPES_H
#define SKIPLIST_TYPES_H

/* The maximum number of next pointers per node in this skip list
   implementation is 32, due to the random number generator only
   generating 32 bit numbers. This value allows for efficient
   O(log(n)) skiplist insertion where n is up to 2^32 nodes. */
#define MAX_LIST_DEPTH (32)

/**
 * @brief Represents a link between two nodes in a skiplist.
 */
typedef struct link_t
{
	/** The width of the link. i.e. if we follow this link,
	    how many nodes have we advanced. */
	unsigned int width;

	/** A pointer to the next node in this link's level. */
	struct node_t *next;
} link_t;

/**
 * @brief Represents a single node in a skiplist.
 */
typedef struct node_t
{
	/** The value for this node. */
	uintptr_t value;

	/** The number of next pointers in this node. */
	unsigned int levels;

	/** An array of links, one entry for each level in the node. */
	link_t link[1];
} node_t;

/**
 * @brief Holds the state for the skiplist's random number generator.
 */
typedef struct skiplist_rng_t
{
	/** RNG state 'w' */
	unsigned int m_w;

	/** RNG state 'z' */
	unsigned int m_z;
} skiplist_rng_t;

/**
 * @brief Function pointer typedef for comparing nodes
 *
 * @retval 0   if @p a and @p b are equal
 * @retval < 0 if @p a is less than @p b
 * @retval > 0 if @p a is greater than @p b
 */
typedef int (*skiplist_compare_pfn)( const uintptr_t a, const uintptr_t b );

/**
 * @brief The skiplist datastructure.
 */
typedef struct skiplist_t
{
	/** Holds the random number generator state for this skiplist.
	    Each skiplist has its own state so that:
	    1. The behaviour is deterministic within a skiplist.
	    2. Seperate skiplists may be used in different threads
	       without the need for any locking.

	    Individual skiplists are not threadsafe, a single skiplist
	    cannot be used by multiple threads at the same time without
	    ensuring mutual exclusion before calling into this module.
	 */
	skiplist_rng_t rng;

	/** Function pointer for comparing nodes. */
	skiplist_compare_pfn compare;

	/** The number of nodes in this skiplist. */
	unsigned int num_nodes;

	/** The head node. */
	node_t head;
} skiplist_t;

#endif
