#ifndef SKIPLIST_H
#define SKIPLIST_H

/**
 * @brief Forward declaration of a skiplist structure.
 *
 * Required to be able to use the skiplist interface without exposing the internal
 * implementation details.
 */
typedef struct skiplist_t skiplist_t;

/**
 * @brief Forward declaration of a node structure.
 *
 * Required to be able to use the skiplist interface without exposing the internal
 * implementation details.
 */
typedef struct node_t node_t;

/**
 * @brief Creates a new skiplist.
 *
 * @param [in] size_estimate_log2  An estimate of log2() of the maximum number
 *                                 of elements that will appear in the list at
 *                                 the same time.
 *
 * @return If successfully a new skiplist is returned, otherwise NULL.
 */
skiplist_t *skiplist_create( unsigned int size_estimate_log2 );

/**
 * @brief Destroys a skiplist that was created via skiplist_create().
 *
 * @param [in] skiplist  The skiplist to destroy.
 */
void skiplist_destroy( skiplist_t *skiplist );

/**
 * @brief Determines whether the given value already exists in the skiplist set.
 *
 * @param [in] skiplist  The skiplist to search.
 * @param [in] value     The value to search for.
 *
 * @retval 1             If the value exists in the skiplist set.
 * @retval 0             If the value doesn't exist in the skiplist set.
 */
unsigned int skiplist_contains( const skiplist_t *skiplist, unsigned int value );

/**
 * @brief Insert a value into a skiplist.
 *
 * @param [in] skiplist  The skiplist to insert @p value into.
 * @param [in] value     The value to insert into @p skiplist.
 *
 * @retval 0  if successful.
 * @retval -1 if a memory allocation failed.
 */
int skiplist_insert( skiplist_t *skiplist, unsigned int value );

/**
 * @brief Removes a value from a skiplist.
 *
 * @param [in] skiplist  The skiplist to remove @p value from.
 * @param [in] value     The value to remove from @p skiplist.
 *
 * @retval 0  if the value was successfully removed.
 * @retval -1 if the value did not exist in the skiplist.
 */
int skiplist_remove( skiplist_t *skiplist, unsigned int value );

/**
 * @brief Prints the skiplist in DOT format to stdout.
 *
 * @param [in] skiplist  The skiplist to print.
 */
void skiplist_printf( const skiplist_t *skiplist );

/**
 * @brief Returns the value of the node at the given index.
 *
 * @pre @p index must be less than the number of elements in @p skiplist (@see skiplist_size()).
 *
 * @param [in] skiplist  The skiplist to lookup index @p index in.
 * @param [in] index     The index to find the value for.
 *
 * @return The value at index @p index.
 */
unsigned int skiplist_at_index( const skiplist_t *skiplist, unsigned int index );

/**
 * @brief Returns a pointer to the start of the skiplist.
 *
 * @param [in] skiplist  The skiplist to return the first element for
 * @return A pointer to the first node in the skiplist, NULL if the skiplist is empty.
 */
node_t *skiplist_begin( skiplist_t *skiplist );

/**
 * @brief Returns a pointer to one past the end of the skiplist.
 *
 * In the current implementation this value will be NULL.
 *
 * @return  A NULL pointer.
 */
node_t *skiplist_end( void );

/**
 * @brief Returns a pointer to the node after @p cur in the skiplist.
 *
 * @param [in] A pointer to the current node.
 *
 * @return  A pointer to the node after @p cur.
 */
node_t *skiplist_next( const node_t *cur );

/**
 * @brief Returns the number of nodes in the skiplist.
 *
 * @param [in] skiplist  A pointer to the skiplist to count the nodes in.
 *
 * @return The number of nodes in @p skiplist.
 */
unsigned int skiplist_size( const skiplist_t *skiplist );

#endif
