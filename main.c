#include <assert.h>
#include <stdio.h>
#include <stdlib.h>
#include <limits.h>
#include <time.h>

#ifdef __MACH__
#include <mach/clock.h>
#include <mach/mach.h>
#endif

#include "skiplist.h"

/**
 * @brief Acquire a current timestamp
 */
static void time_stamp( struct timespec *stamp )
{
#ifdef __MACH__
	clock_serv_t cclock;
	mach_timespec_t mts;
	host_get_clock_service( mach_host_self(), CALENDAR_CLOCK, &cclock );
	clock_get_time( cclock, &mts );
	mach_port_deallocate( mach_task_self(), cclock );
	stamp->tv_sec = mts.tv_sec;
	stamp->tv_nsec = mts.tv_nsec;
#else
	clock_gettime( CLOCK_MONOTONIC, stamp );
#endif
}

/**
 * @brief Return the difference between two timestamps in nanoseconds
 */
static unsigned long long time_diff_ns( const struct timespec *start, const struct timespec *end )
{
	unsigned long long seconds = end->tv_sec - start->tv_sec;
	unsigned long long nano_seconds = end->tv_nsec - start->tv_nsec;
	return seconds * 1000000000ULL + nano_seconds;
}

/**
 * @brief Prints a 'dot' representation of the skiplist to the given filename.
 */
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
static int int_compare( const uintptr_t a, const uintptr_t b )
{
	return a - b;
}

/**
 * @brief Prints the given integer to the file stream.
 */
static void int_fprintf( FILE *stream, const uintptr_t value )
{
	fprintf( stream, "%d", (int)value );
}

/**
 * @brief TEST_CASE - Sanity test of some key skiplist APIs using integers.
 */
static int simple( void )
{
	unsigned int i;
	node_t *iter;
	skiplist_t *skiplist;

	skiplist = skiplist_create( 5, int_compare, int_fprintf );
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
		if( skiplist_remove( skiplist, i ) )
			return -1;

	for( iter = skiplist_begin( skiplist );
	     iter != skiplist_end();
	     iter = skiplist_next( iter ) )
	{
		uintptr_t value = skiplist_node_value( iter );
		if( value >= 5 && value < 10 )
			return -1;
	}

	for( i = 0; i < skiplist_size( skiplist ); ++i )
		skiplist_at_index( skiplist, i );

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

/**
 * @brief Compares two coordinates.
 */
static int coord_compare( const uintptr_t a, const uintptr_t b )
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
 * @brief Prints the given coordinate to the file stream.
 */
static void coord_fprintf( FILE *stream, const uintptr_t value )
{
	coord_t *coord = (coord_t *) value;
	fprintf( stream, "{x: %u, y: %u}", coord->x, coord->y );
}

/**
 * @brief TEST_CASE - Sanity test of some key skiplist APIs using a pointer to data items.
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
	coord_t tmp;

	skiplist = skiplist_create( 8, coord_compare, coord_fprintf );
	if( !skiplist ) return -1;

	for( i = 0; i < sizeof(coords) / sizeof(coords[0]); ++i )
		if( skiplist_insert( skiplist, (uintptr_t) &coords[i] ) )
			return -1;

	/* Output skiplist for debugging purposes. */
	if( skiplist_to_file( "pointers.dot", skiplist ) )
		return -1;

	/* Confirm skiplist is in the correct order. */
	tmp.x = 0;
	tmp.y = 0;
	for( iter = skiplist_begin( skiplist ); iter != skiplist_end(); iter = skiplist_next( iter ) )
	{
		coord_t *cur = (coord_t *)skiplist_node_value( iter );
		if( cur->x < tmp.x ) return -1;
		if( cur->x == tmp.x && cur->y < tmp.y ) return -1;
		tmp = *cur;
	}

	/* Confirm the skiplist contains what we expect. */
	for( i = 0; i < sizeof(coords) / sizeof(coords[0]); ++i )
		if( !skiplist_contains( skiplist, (uintptr_t) &coords[i] ) )
			return -1;

	/* If we use a different pointer to point to the same values the skiplist should skill contain it. */
	tmp = coords[0];
	if( !skiplist_contains( skiplist, (uintptr_t) &tmp ) )
		return -1;

	/* Free resources. */
	skiplist_destroy( skiplist );

	return 0;
}

/**
 * @brief TEST_CASE - Measures lookup trade off between number of elements in the list and number of links per node.
 */
static int link_trade_off_lookup( void )
{
#define MAX_LINKS (SKIPLIST_MAX_LINKS)
#define INSERTIONS_LOG2 (16)
	unsigned int i;
	FILE *fp;
	const char *seperator;

	fp = fopen( "link_trade_off_lookup.gplot", "w" );
	if( !fp ) return -1;
	fprintf(fp, "set term qt\n");
	fprintf(fp, "set key off\n");
	fprintf(fp, "set logscale\n");
	fprintf(fp, "set grid xtics ytics mxtics mytics\n");
	fprintf(fp, "set style textbox opaque noborder\n");
	fprintf(fp, "set title \"Average Lookup Time for Skiplists with Varying Link Counts\"\n");
	fprintf(fp, "set xlabel \"Number of Elements in the Skiplist\"\n");
	fprintf(fp, "set ylabel \"Average Time for One Lookup (ns)\"\n");
	fprintf(fp, "plot " );

	seperator = "";
	for( i = 0; i < MAX_LINKS; ++i )
	{
		fprintf(fp, "%s\"link_trade_off_lookup.dat\" using 1:%u with lines lt -1,"
		        "\"\" using 1:%u:($0*0+%u) with labels center boxed notitle",
		        seperator, i + 2, i + 2, i + 1);
		seperator = ",\\\n\t";
	}
	fprintf(fp, "\n");
	fprintf(fp, "pause -1\n");
	fclose( fp );

	fp = fopen( "link_trade_off_lookup.dat", "w" );
	if( !fp ) return -1;

	for( i = 1; i < (1 << INSERTIONS_LOG2); i <<= 1 )
	{
		unsigned int links;
		fprintf( fp, "%u", i );
		for( links = 1; links <= MAX_LINKS; ++links )
		{
			unsigned int j;
			skiplist_t *skiplist;
			struct timespec start, end;

			skiplist = skiplist_create( links, int_compare, int_fprintf );
			if( !skiplist ) return -1;

			for( j = 0; j < i; ++j )
				if( skiplist_insert( skiplist, j ) )
					return -1;

			time_stamp( &start );
			for( j = 0; j < i; ++j )
				if( !skiplist_contains( skiplist, j ) )
					return -1;
			time_stamp( &end );
			fprintf( fp, "\t%f", time_diff_ns( &start, &end ) / (double)i );
			skiplist_destroy( skiplist );
		}
		fprintf( fp, "\n" );
	}

	fclose( fp );

#undef MAX_LINKS
#undef INSERTIONS_LOG2
	return 0;
}

/**
 * @brief TEST_CASE - Measures insertion trade off between number of elements in the list and number of links per node.
 */
static int link_trade_off_insert( void )
{
#define MAX_LINKS (SKIPLIST_MAX_LINKS)
#define INSERTIONS_LOG2 (16)

	struct timespec stamps[INSERTIONS_LOG2 + 2];
	FILE *fp;
	unsigned int links;
	unsigned int i;
	const char *seperator;

	fp = fopen( "link_trade_off_insert.gplot", "w" );
	if( !fp ) return -1;
	fprintf(fp, "set term qt\n");
	fprintf(fp, "set key off\n");
	fprintf(fp, "set logscale\n");
	fprintf(fp, "set grid xtics ytics mxtics mytics\n");
	fprintf(fp, "set style textbox opaque noborder\n");
	fprintf(fp, "set title \"Average Insertion Time for Skiplists with Varying Link Counts\"\n");
	fprintf(fp, "set xlabel \"Number of Elements in the Skiplist\"\n");
	fprintf(fp, "set ylabel \"Average Time for One Insertion (ns)\"\n");
	fprintf(fp, "plot " );
	seperator = "";
	for( i = 0; i < MAX_LINKS; ++i )
	{
		fprintf(fp, "%s\"link_trade_off_insert_%u.dat\" using 1:2 with lines lt -1,"
		        "\"\" using 1:2:($0*0+%u) with labels center boxed notitle",
		        seperator, i + 1, i + 1);
		seperator = ",\\\n\t";
	}
	fprintf(fp, "\n");
	fprintf(fp, "pause -1\n");
	fclose( fp );

	for( links = MAX_LINKS; links > 0; --links )
	{
		skiplist_t *skiplist;
		char filename[64];
		unsigned int next;

		sprintf( filename, "link_trade_off_insert_%u.dat", links );
		fp = fopen( filename, "w" );
		if( !fp ) return -1;

		skiplist = skiplist_create( links, int_compare, int_fprintf );
		if( !skiplist ) return -1;

		next = 0;
		for( i = 0; i < (1 << INSERTIONS_LOG2); ++i )
		{
			/* Sample at powers of 2. */
			if( (i & (i - 1)) == 0 )
			{
				time_stamp( &stamps[next] );

				/* Stop trying if it's taking too long. */
				if( next && time_diff_ns( &stamps[next - 1], &stamps[next] ) > 300000000LLU )
					break;

				++next;
			}

			if( skiplist_insert( skiplist, rand() ) )
				return -1;
		}
		time_stamp( &stamps[next] );
		++next;

		skiplist_destroy( skiplist );

		for( i = 1; i < next; ++i )
		{
			const unsigned int node_count = 1 << (i - 1);
			fprintf(fp, "%u\t%f\n", node_count,
			        time_diff_ns( &stamps[0], &stamps[i] ) / (double)node_count );
		}

		fclose( fp );
	}

#undef MAX_LINKS
#undef INSERTIONS_LOG2
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
		TEST_CASE( pointers ),
		TEST_CASE( link_trade_off_lookup ),
		TEST_CASE( link_trade_off_insert )
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
