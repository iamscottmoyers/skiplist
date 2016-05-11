#ifndef SKIPLIST_H
#define SKIPLIST_H

#include <stdint.h>

#include "skiplist_types.h"

/**
 * @brief Creates a new skiplist.
 *
 * @param [in]  properties          The properties for this skiplist. i.e.
 *                                  Unique entries or not.
 * @param [in]  size_estimate_log2  An estimate of log2() of the maximum number
 *                                  of elements that will appear in the list at
 *                                  the same time.
 * @param [in]  compare             Function for comparing the values that will
 *                                  be used in this skiplist.
 * @param [in]  print               Function for printing the value of the data
 *                                  in the skiplist.
 * @param [out] error               Will point to the error status of the
 *                                  function on return. May be set to NULL.
 *                                  SKIPLIST_ERROR_SUCCESS if successful.
 *                                  SKIPLIST_ERROR_INVALID_INPUT if this
 *                                  function was called with invalid input
 *                                  values.
 *                                  SKIPLIST_ERROR_OUT_OF_MEMORY if this
 *                                  function failed to allocate memory.
 *
 * @return If successful a new skiplist is returned, otherwise NULL.
 */
skiplist_t *skiplist_create( skiplist_properties_t properties,
                             unsigned int size_estimate_log2,
                             skiplist_compare_pfn compare,
                             skiplist_fprintf_pfn print,
                             skiplist_error_t * const error );

/**
 * @brief Destroys a skiplist that was created via skiplist_create().
 *
 * @param [in] skiplist  The skiplist to destroy.
 *
 * @retval SKIPLIST_ERROR_SUCCESS If the skiplist was successfully destroyed.
 * @retval SKIPLIST_ERROR_INVALID_VALUE If @p skiplist was NULL or otherwise invalid.
 */
skiplist_error_t skiplist_destroy( skiplist_t *skiplist );

/**
 * @brief Determines whether the given value already exists in the skiplist set.
 *
 * @param [in]  skiplist  The skiplist to search.
 * @param [in]  value     The value to search for.
 * @param [out] error     Will point to the error status of the function on
 *                        return. May be set to NULL.
 *                        SKIPLIST_ERROR_SUCCESS if successful.
 *                        SKIPLIST_ERROR_INVALID_INPUT if this function was
 *                        called with invalid input values.
 *
 * @retval 1              If the value exists in the skiplist set.
 * @retval 0              If the value doesn't exist in the skiplist set,
 *                        or input values were invalid.
 */
unsigned int skiplist_contains( const skiplist_t *skiplist, uintptr_t value, skiplist_error_t * const error );

/**
 * @brief Insert a value into a skiplist.
 *
 * @param [in] skiplist  The skiplist to insert @p value into.
 * @param [in] value     The value to insert into @p skiplist.
 *
 * @retval SKIPLIST_ERROR_SUCCESS if successful.
 * @retval SKIPLIST_ERROR_OUT_OF_MEMORY if a memory allocation failed
 * @retval SKIPLIST_ERROR_INVALID_INPUT if input values were invalid.
 */
skiplist_error_t skiplist_insert( skiplist_t *skiplist, uintptr_t value );

/**
 * @brief Removes a value from a skiplist.
 *
 * @param [in] skiplist  The skiplist to remove @p value from.
 * @param [in] value     The value to remove from @p skiplist.
 *                       Must exist in the skiplist for this function to
 *                       return successfully.
 *
 * @retval SKIPLIST_ERROR_SUCCESS if the value was successfully removed.
 * @retval SKIPLIST_ERROR_INVALID_INPUT if input values were invalid.
 */
skiplist_error_t skiplist_remove( skiplist_t *skiplist, uintptr_t value );

/**
 * @brief Prints the skiplist in DOT format to stdout.
 *
 * @param [in] skiplist  The skiplist to print.
 *
 * @retval SKIPLIST_ERROR_SUCCESS if the skiplist was successfully printed.
 * @retval SKIPLIST_ERROR_INVALID_INPUT if the input values were invalid.
 */
skiplist_error_t skiplist_printf( const skiplist_t *skiplist );

/**
 * @brief Prints the skiplist in DOT format to the provided file stream.
 *
 * @param [in] stream    The filestream to print to.
 * @param [in] skiplist  The skiplist to print.
 *
 * @retval SKIPLIST_ERROR_SUCCESS if the skiplist was successfully printed.
 * @retval SKIPLIST_ERROR_INVALID_INPUT if the input values were invalid.
 */
skiplist_error_t skiplist_fprintf( FILE *stream, const skiplist_t *skiplist );

/**
 * @brief Prints the skiplist in DOT format to a file called @p filename.
 *
 * @param [in] filename  The name of the file to print to.
 * @param [in] skiplist  The skiplist to print.
 *
 * @retval SKIPLIST_ERROR_SUCCESS if the skiplist was successfully printed to @p filename.
 * @retval SKIPLIST_ERROR_INVALID_INPUT if the input values were invalid.
 * @retval SKIPLIST_ERROR_OPENING_FILE if the function was unable to open @p filename for writing.
 */
skiplist_error_t skiplist_fprintf_filename( const char *filename, const skiplist_t *skiplist );

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
uintptr_t skiplist_at_index( const skiplist_t *skiplist, unsigned int index );

/**
 * @brief Returns a pointer to the start of the skiplist.
 *
 * @param [in] skiplist  The skiplist to return the first element for
 * @return A pointer to the first node in the skiplist, NULL if the skiplist is empty, or invalid input.
 */
skiplist_node_t *skiplist_begin( skiplist_t *skiplist );

/**
 * @brief Returns a pointer to one past the end of the skiplist.
 *
 * In the current implementation this value will be NULL.
 *
 * @return  A NULL pointer.
 */
skiplist_node_t *skiplist_end( void );

/**
 * @brief Returns a pointer to the node after @p cur in the skiplist.
 *
 * @param [in] A pointer to the current node.
 *
 * @return  A pointer to the node after @p cur. NULL if invalid input.
 */
skiplist_node_t *skiplist_next( const skiplist_node_t *cur );

/**
 * @brief Returns the value at the given node.
 *
 * @param [in] node  The node to return the value for.
 *
 * @return The value at the given node. 0 on invalid input.
 */
uintptr_t skiplist_node_value( const skiplist_node_t *node );

/**
 * @brief Returns the number of nodes in the skiplist.
 *
 * @param [in] skiplist  A pointer to the skiplist to count the nodes in.
 *
 * @return The number of nodes in @p skiplist. 0 on invalid input.
 */
unsigned int skiplist_size( const skiplist_t *skiplist );

#endif
