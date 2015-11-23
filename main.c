#include <assert.h>
#include <sys/time.h>
#include <stdio.h>
#include <stdlib.h>

#include "skiplist.h"

static void simple_test()
{
	int err;
	unsigned int i;
	skiplist_t *skiplist;

	skiplist = skiplist_create( 5 );

	for( i = 0; i < 10; ++i )
	{
		err = skiplist_insert( skiplist, i );
		assert( 0 == err );
		assert( skiplist_contains( skiplist, i ) );
	}

	for( i = 0; i < 100; ++i )
	{
		unsigned int value = rand();
		err = skiplist_insert( skiplist, value );
		assert( 0 == err );
		assert( skiplist_contains( skiplist, value ) );
	}

	for( i = 10; i-- != 5; )
	{
		err = skiplist_remove( skiplist, i );
		assert( 0 == err );
	}

	for( i = 0; i < skiplist_size( skiplist ); ++i )
	{
		skiplist_at_index( skiplist, i );
	}

	skiplist_destroy( skiplist );
}

static double time_diff(struct timeval x, struct timeval y)
{
	double x_us , y_us , diff;

	x_us = (double)x.tv_sec*1000000 + (double)x.tv_usec;
	y_us = (double)y.tv_sec*1000000 + (double)y.tv_usec;

	diff = (double)y_us - (double)x_us;

	return diff;
}

static void random_insert_scalability()
{
	FILE *plot;
	unsigned int i;
	unsigned int depth;
	skiplist_t *skiplist;

	plot = fopen( "random_insert.plot", "w" );
	assert( plot );

	fprintf( plot, "set logscale x;\nset logscale y;\nplot ");

	for( depth = 1; depth < 32; depth += 2 )
	{
		FILE *out;
		char filename[128];

		sprintf( filename, "random_insert.%02d.dat", depth );
		out = fopen( filename, "w" );
		assert( out );

		fprintf( plot, "\"%s\" using 1:2 with lines,", filename );

		for( i = 2; i < (1 << 15); i *= 2 )
		{
			struct timeval start, stop;
			unsigned int j;

			srand( 100 );
			skiplist = skiplist_create( depth );

			gettimeofday( &start, NULL );
			for( j = 0; j < i; ++j )
			{
				skiplist_insert( skiplist, rand() );
			}
			gettimeofday( &stop, NULL );

			fprintf( out, "%d %g\n", i, time_diff( start, stop ) );
			skiplist_destroy( skiplist );
		}
		fclose( out );
	}

	fprintf( plot, "\n" );
	fclose( plot );
}

static void random_lookup_scalability()
{
	FILE *plot;
	unsigned int i;
	unsigned int depth;
	skiplist_t *skiplist;

	plot = fopen( "random_lookup.plot", "w" );
	assert( plot );

	fprintf( plot, "set logscale x;\nset logscale y;\nplot ");

	for( depth = 1; depth < 32; depth += 2 )
	{
		FILE *out;
		char filename[128];

		sprintf( filename, "random_lookup.%02d.dat", depth );
		out = fopen( filename, "w" );
		assert( out );

		fprintf( plot, "\"%s\" using 1:2 with lines,", filename );

		for( i = 2; i < (1 << 14); i *= 2 )
		{
			struct timeval start, stop;
			unsigned int j;

			srand( 100 );
			skiplist = skiplist_create( depth );

			/* Fill with 'i' nodes */
			for( j = 0; j < i; ++j )
			{
				skiplist_insert( skiplist, rand() );
			}

			/* Get the average time for 1000 random lookups */
			gettimeofday( &start, NULL );
			for( j = 0; j < 1000; ++j )
			{
				skiplist_at_index( skiplist, rand() % i );
			}
			gettimeofday( &stop, NULL );

			fprintf( out, "%d %g\n", i, (time_diff( start, stop ) / 100.0) );
			skiplist_destroy( skiplist );
		}
		fclose( out );
	}

	fprintf( plot, "\n" );
	fclose( plot );
}

static void insertion_performance( unsigned int count, unsigned int depth )
{
	unsigned int i;
	skiplist_t *skiplist;
	struct timeval start, stop;

	srand( 100 );
	skiplist = skiplist_create( depth );

	gettimeofday( &start, NULL );
	for( i = 0; i < count; ++i )
	{
		skiplist_insert( skiplist, rand() );
	}
	gettimeofday( &stop, NULL );

	printf( "%d %d %g\n", count, depth, time_diff( start, stop ) );

	skiplist_destroy( skiplist );
}

int main( int argc, char *argv[] )
{
/*	random_insert_scalability();*/
/*	random_lookup_scalability();*/
/*	insertion_performance( 50000000, 20 );*/
	simple_test();
	return 0;
}
