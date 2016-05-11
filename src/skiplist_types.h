#ifndef SKIPLIST_TYPES_H
#define SKIPLIST_TYPES_H

/**
 * The maximum number of next pointers per node in this skip list
 * implementation is 32, due to the random number generator only
 * generating 32 bit numbers. This value allows for efficient
 * O(log(n)) skiplist insertion where n is up to 2^32 nodes.
 *
 * Always picking the maximum number of links will increase memory
 * consumption and has a high constant overhead due to the number
 * of links that need to be maintained. So where possible pick a
 * smaller number.
 */
#define SKIPLIST_MAX_LINKS (32)

/**
 * @brief Skiplist property bitset.
 */
typedef unsigned int skiplist_properties_t;

/**
 * @brief Only insert values if they're unique. i.e. Behave like a set.
 */
#define SKIPLIST_PROPERTY_UNIQUE (1 << 0)

/**
 * @brief No properties for the skiplist, by default duplicate entries are allowed.
 */
#define SKIPLIST_PROPERTY_NONE (0)

/**
 * @brief Represents a link between two nodes in a skiplist.
 */
typedef struct skiplist_link_t
{
	/** The width of the link. i.e. if we follow this link,
	    how many nodes have we advanced. */
	unsigned int width;

	/** A pointer to the next node in this link's level. */
	struct skiplist_node_t *next;
} skiplist_link_t;

/**
 * @brief Represents a single node in a skiplist.
 */
typedef struct skiplist_node_t
{
	/** The value for this node. */
	uintptr_t value;

	/** The number of next pointers in this node. */
	unsigned int levels;

	/** An array of links, one entry for each level in the node. */
	skiplist_link_t link[1];
} skiplist_node_t;

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
 * @brief Function pointer callback for comparing nodes.
 *
 * @retval 0   if @p a and @p b are equal
 * @retval < 0 if @p a is less than @p b
 * @retval > 0 if @p a is greater than @p b
 */
typedef int (*skiplist_compare_pfn)( const uintptr_t a, const uintptr_t b );

/**
 * @brief Function pointer callback for printing the value of a node.
 *
 * @param [in] stream  A pointer to the file stream to print to.
 * @param [in] value   The value to print.
 */
typedef void (*skiplist_fprintf_pfn)( FILE *stream, const uintptr_t value );

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

	/** Properties for this skiplist, i.e. Unique entries or not. */
	skiplist_properties_t properties;

	/** Function pointer for comparing nodes. */
	skiplist_compare_pfn compare;

	/** Function pointer for printing nodes. */
	skiplist_fprintf_pfn print;

	/** The number of nodes in this skiplist. */
	unsigned int num_nodes;

	/** The head node. */
	skiplist_node_t head;
} skiplist_t;

typedef enum skiplist_error_t
{
	/* SKIPLIST_ERROR_SUCCESS must always be 0. */
	SKIPLIST_ERROR_SUCCESS = 0,

	SKIPLIST_ERROR_OUT_OF_MEMORY,
	SKIPLIST_ERROR_INVALID_INPUT,
	SKIPLIST_ERROR_OPENING_FILE
} skiplist_error_t;

#endif
