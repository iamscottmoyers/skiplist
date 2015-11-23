#include <assert.h>
#include <sys/time.h>
#include <stdio.h>
#include <stdlib.h>

#include "skiplist.h"

static int compare( const uintptr_t a, const uintptr_t b )
{
	return a - b;
}

static int simple_test(void)
{
	unsigned int i;
	node_t *iter;
	skiplist_t *skiplist;

	skiplist = skiplist_create( 5, compare );
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
		TEST_CASE( simple_test )
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
