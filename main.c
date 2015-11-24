#include <assert.h>
#include <sys/time.h>
#include <stdio.h>
#include <stdlib.h>
#include <limits.h>

#include "skiplist.h"

static int skiplist_to_file( const char *fname, const skiplist_t *skiplist )
{
	FILE *fp = fopen( fname, "w" );
	if( !fp ) return -1;
	skiplist_fprintf( fp, skiplist );
	fclose( fp );
	return 0;
}

/**
 * @brief Compares two integers.
 */
static int compare_int( const uintptr_t a, const uintptr_t b )
{
	return a - b;
}

/**
 * @brief Sanity test of some key skiplist APIs using integers.
 */
static int simple( void )
{
	unsigned int i;
	node_t *iter;
	skiplist_t *skiplist;

	skiplist = skiplist_create( 5, compare_int );
	if( !skiplist )
		return -1;

	for( i = 0; i < 10; ++i )
	{
		if( skiplist_insert( skiplist, i ) )
			return -1;
		if( !skiplist_contains( skiplist, i ) )
			return -1;
	}

	for( i = 0; i < 100; ++i )
	{
		unsigned int value = rand();
		if( skiplist_insert( skiplist, value ) )
			return -1;
		if( !skiplist_contains( skiplist, value ) )
			return -1;
	}

	for( i = 5; i < 10; ++i )
	{
		if( skiplist_remove( skiplist, i ) )
			return -1;
	}

	for( iter = skiplist_begin( skiplist );
	     iter != skiplist_end();
	     iter = skiplist_next( iter ) )
	{
		uintptr_t value = skiplist_node_value( iter );
		if( value >= 5 && value < 10 )
			return -1;
	}

	for( i = 0; i < skiplist_size( skiplist ); ++i )
	{
		skiplist_at_index( skiplist, i );
	}

	if( skiplist_to_file( "simple.dot", skiplist ) )
		return -1;

	skiplist_destroy( skiplist );

	return 0;
}

/** @brief XY coordinate datastructure for the 'pointers' test. */
typedef struct coord_t
{
	unsigned int x; /**< x coordinate. */
	unsigned int y; /**< y coordinate. */
} coord_t;

static int compare_coord( const uintptr_t a, const uintptr_t b )
{
	coord_t *ca = (coord_t *)a;
	coord_t *cb = (coord_t *)b;
	if( ca->x < cb->x ) return -1;
	if( ca->x > cb->x ) return 1;
	if( ca->y < cb->y ) return -1;
	if( ca->y > cb->y ) return 1;
	return 0;
}

/**
 * @brief Sanity test of some key skiplist APIs using a pointer to data items.
 */
static int pointers( void )
{
	skiplist_t *skiplist;
	const coord_t coords[] =
	{
		/* Simple in order insertion. */
		{5,5},
		{7,5},

		/* Duplicate x with increasing y. */
		{5,6},
		{5,8},

		/* Duplicate x with decreasing y. */
		{7,4},
		{7,0},

		/* Decreasing x. */
		{4,5},
		{3,5},

		/* Increasing x. */
		{9,0},
		{10,0},

		/* Duplicate values. */
		{9,0},
		{5,5},

		/* Zero. */
		{0,0},

		/* Huge. */
		{UINT_MAX,UINT_MAX}
	};

	unsigned int i;
	node_t *iter;
	coord_t prev;

	skiplist = skiplist_create( 8, compare_coord );
	if( !skiplist ) return -1;

	for( i = 0; i < sizeof(coords) / sizeof(coords[0]); ++i )
	{
		if( skiplist_insert( skiplist, (uintptr_t) &coords[i] ) )
			return -1;
	}

	if( skiplist_to_file( "pointers.dot", skiplist ) )
		return -1;

	prev.x = 0;
	prev.y = 0;
	for( iter = skiplist_begin( skiplist );
	     iter != skiplist_end();
	     iter = skiplist_next( iter ) )
	{
		coord_t *cur = (coord_t *)skiplist_node_value( iter );
		if( cur->x < prev.x ) return -1;
		if( cur->x == prev.x && cur->y < prev.y ) return -1;
		prev = *cur;
	}

	skiplist_destroy( skiplist );

	return 0;
}

/** Function pointer for a test case. */
typedef int (*test_pfn)(void);

/**
 * @brief Structure for a test case.
 */
typedef struct test_case_t
{
	/** Function pointer to call to execute the test. */
	test_pfn pfn;

	/** The name of the test. */
	const char *name;
} test_case_t;

/** Macro for defining the test name the same as the function name. */
#define TEST_CASE(_name) {_name, #_name}

int main( int argc, char *argv[] )
{
	unsigned int i;
	const test_case_t tests[] =
	{
		TEST_CASE( simple ),
		TEST_CASE( pointers )
	};

	(void)argc;
	(void)argv;

	for( i = 0; i < sizeof(tests) / sizeof(tests[0]); ++i )
	{
		int result;
		printf( "%s...", tests[i].name );
		fflush( stdout );
		result = tests[i].pfn();
		printf( result ? "Fail\n" : "Pass\n" );
	}

	return 0;
}
