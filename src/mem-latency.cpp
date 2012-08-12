#include <string>
#include <vector>
#include <cstdlib>
#include <iomanip>
#include <fstream>
#include <iostream>
#include <assert.h>
#include <stdlib.h>
#include <getopt.h>
////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////
#ifdef __NUMA_ENABLED__
#include <numa.h>
#include <errno.h>
#define handle_error_en(en, msg) do { errno = en; perror(msg); exit(EXIT_FAILURE); } while (0)
#endif
////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////
#include <boost/random/uniform_int.hpp>
#include <boost/random/mersenne_twister.hpp>
#include <boost/random/variate_generator.hpp>
////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////
// the base pointer chase is "ONE"
// issue this line of code up to 100 thousand times
// see POINTERCHASE & NUMCHASE below
#define ONE p = (char**)(*p);
#define TWO ONE ONE
#define FOUR TWO TWO
#define FIVE    ONE ONE ONE ONE ONE
#define TEN     FIVE FIVE
#define FIFTY   TEN TEN TEN TEN TEN
#define HUNDRED FIFTY FIFTY
#define FIVEHUNDRED HUNDRED HUNDRED HUNDRED HUNDRED HUNDRED
#define THOUSAND FIVEHUNDRED FIVEHUNDRED
#define FIVETHOUSAND THOUSAND THOUSAND THOUSAND THOUSAND THOUSAND
#define TENTHOUSAND FIVETHOUSAND FIVETHOUSAND
#define FIFTYTHOUSAND TENTHOUSAND TENTHOUSAND TENTHOUSAND TENTHOUSAND TENTHOUSAND
#define HUNDREDTHOUSAND FIFTYTHOUSAND FIFTYTHOUSAND
////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////
// shortcuts to use the defines above
// POINTERCHASE is the code to be executed
// NUMCHASE is used later to compute the latency
#define POINTERCHASE TENTHOUSAND
#define NUMCHASE     10000
////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////
#include <Types.h>
#include <Defines.h>
#include <TimeDiff.h>
#include <TestParams.h>
#include <MallocAlign.h>
////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////
using namespace std;
////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////
void SetProcessAffinity( U32 cpuIdx, const bool verbose ) {

   #ifndef __MAC_OSX__
   #ifdef __NUMA_ENABLED__
   DEBUG_PRINT( "SetProcessAffinity()" );
   int s;
   cpu_set_t cpuset;
   // pid_t pid = getpid();
   
   // set affinity mask
   if( verbose ) {
      cout << "... assigning process to cpu " << cpuIdx << endl;
   }
   CPU_ZERO( &cpuset );
   CPU_SET( cpuIdx, &cpuset );

   // set affinity
   s = sched_setaffinity( 0, sizeof( cpu_set_t ), &cpuset );
   if( s != 0 ) {
      handle_error_en( s, "sched_setaffinity" );
      cout << endl;
      exit( -1 );
   }
   
   numa_set_localalloc();
   // s = set_mempolicy( MPOL_BIND, unsigned long *nodemask, unsigned long maxnode);
   #endif
   #endif
}
char ** AllocAlignAndInitBuffer( const TestParams & params ) {

   // calculate number of bytes to allocate
   // 2x just to leave some headroom when making the pointer chase
   const U64 numByte = 2 * params.maxsize;
   const U64 numPtrs = numByte / sizeof( char* );
   
   // node bind "to"
   SetProcessAffinity( params.to, params.verbose );
   
   // alloc
   if( params.verbose ) {
      cout << "... allocating " << numByte << " bytes" << endl;
   }
   char ** buf = (char **) MallocAlign( numByte, params.cacheLineSize );
   
   // touch every page and fill with random data:
   // prevent any OS magic from preventing page allocation on the desired node
   if( params.verbose ) {
      cout << "... initializing with random values to touch every page" << endl;
   }
   for( U32 ptrIdx = 0; ptrIdx < numPtrs; ptrIdx++ ) {
      // buf[ ptrIdx ] = reinterpret_cast< char* >( rand() );
      buf[ ptrIdx ] = reinterpret_cast< char* >( 1 + ( ptrIdx % params.cacheLineSize ) );
   }
   
   // node bind "from"
   SetProcessAffinity( params.from, params.verbose );
   
   return buf;
}
////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////
void FillBufferRandomPtrMod( char ** buf, const U64 size, const U64 numByte ) {
   
   DEBUG_PRINT( "FillBufferRandomPtrMod( buf, " << size << ", " << numByte << ");" );

   boost::mt19937 gen;
   gen.seed( rand() );
   boost::uniform_int< U64 >uniform_int_dist( 0ULL , ~0ULL );
   boost::variate_generator< boost::mt19937 &, boost::uniform_int< U64 > > rand_U64( gen, uniform_int_dist );
   
   const U64 cacheLineSize = 64;
   const U64 cacheLinesAllocd = numByte / cacheLineSize;
   const U64 numCacheLines    = size    / cacheLineSize;
   const U64 numPtrs = numByte / sizeof( char* );
   const U64 minDistance = 32;
   
   vector< bool > used( cacheLinesAllocd, false );
   
   DEBUG_PRINT( "... zeroing the buffer" );
   for( U64 ptrIdx = 0; ptrIdx < numPtrs; ptrIdx++ ) { buf[ ptrIdx ] = NULL; }
   
   U64 currIdx = 0ULL;
   U64 nextIdx = 0ULL;
   used[ 0ULL ] = true;
   
   U64 currOffset;
   U64 nextOffset;
   
   char * baseAddr = ( char * ) &buf[ 0 ];

   DEBUG_PRINT( "... initializing the pointer loop" );
   for( U64 cacheLineIdx = 0; cacheLineIdx < numCacheLines; cacheLineIdx++ ) {
      
      bool minDistanceNotMet = true;
      while( used[ nextIdx ] || minDistanceNotMet ) {
         nextIdx = rand_U64() % cacheLinesAllocd;
         
         U64 distance = 0ULL;
         if( nextIdx > currIdx ) { distance = nextIdx - currIdx; }
         else                    { distance = currIdx - nextIdx; }
         
         if( distance < minDistance ) { minDistanceNotMet = true;  }
         else                         { minDistanceNotMet = false; }
      }
      used[ nextIdx ] = true;
      
      currOffset = currIdx * cacheLineSize / sizeof( char * );
      nextOffset = nextIdx * cacheLineSize / sizeof( char * );
      
      buf[ currOffset ] = ( char * ) &buf[ nextOffset ];
      
      currIdx = nextIdx;
   }
   
   currOffset = currIdx * cacheLineSize / sizeof( char* );
   buf[ currOffset ] = baseAddr;

}
char ** MeasureLatency( char ** buf, const U64 size, const TestParams & params, timespec * clks ) {
   
   DEBUG_PRINT( "MeasureLatency()" );
   
   const U64 numByte = 2 * params.maxsize;
   const U32 numTrials = params.numTrials;
   
   FillBufferRandomPtrMod( buf, size, numByte );
   
   char ** p = buf;
   
   for( U32 trialIdx = 0; trialIdx < numTrials; trialIdx++ ) {
      clock_gettime( CLOCK_MONOTONIC, clks ); clks++;
      POINTERCHASE
      clock_gettime( CLOCK_MONOTONIC, clks ); clks++;
   }
   DEBUG_PRINT( "p = 0x" << SETHEX( 0 ) << (U64) p << ", *p = 0x" << SETHEX( 0 ) << (U64) *p );
   return p;
}
////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////
void PrintUsage( const char * cmd ) {
   cout << cmd << " [-h] [-s <int>]" << endl;
   cout << "-h, --help:      print this message" << endl;
   cout << "-m, --minimum:   sets size of smallest memory pool" << endl;
   cout << "-s, --size:      sets size of largest memory pool to be tested for latency" << endl;
   cout << "-n, --numtrials: number of tests per memory pool size (default is 1000)" << endl;
   cout << "-f, --from:      set cpu index where test runs" << endl;
   cout << "-t, --to:        set cpu index where memory is allocated" << endl;
   cout << "-v, --verbose:   more info while running" << endl;
   cout << "-c, --csv:       output as csv (comma separated value)" << endl;
   cout << endl;
   cout << "example:" << endl;
   cout << " " << cmd << " --size 1024K" << endl;
}
U64 ParseSizeOpt( const string & optarg ) {
   
   U64 size = strtol( optarg.c_str(), NULL, 10 );

   char lastchar = optarg[ optarg.size() - 1 ];
   
   if( lastchar == 'G' ) {
      size *= 1024*1024*1024;
   }
   else if( lastchar == 'M' ) {
      size *= 1024*1024;
   }
   else if( lastchar == 'K' ) {
      size *= 1024;
   }
   cout << "size = " << size << endl;
   return size;
}
bool ParseCmdLineArgs( TestParams & params, int argc, char ** argv ) {

   cout << "Executing: ";
   for( int i = 0; i < argc; i++ ) {
      cout << argv[i] << " ";
   }
   cout << endl;

   int c;
   int option_index;
   
   static struct option long_options[] = {
       // const char *name, int has_arg, int *flag, int val
       { "minimum",   required_argument, 0, 'm'  },
       { "size",      required_argument, 0, 's'  },
       { "numtrials", required_argument, 0, 'n'  },
       { "from",      required_argument, 0, 'f'  },
       { "to",        required_argument, 0, 't'  },
       { "help",      no_argument,       0, 'h'  },
       { "verbose",   no_argument,       0, 'v'  },
       { "csv",       no_argument,       0, 'c'  },
       { 0,           0,                 0,  0   }
   };

   while( true ) {

      c = getopt_long( argc, argv, "cm:s:n:f:t:vh", long_options, &option_index );

      if( c == -1 ) {
         // end of options
         break;
      }
      
      if( c == 's' ) {
         params.maxsize = ParseSizeOpt( optarg );
      }
      else if( c == 'm' ) {
         params.minsize = ParseSizeOpt( optarg );
      }
      else if( c == 'c' ) {
         params.csv = true;
      }
      else if( c == 'n' ) {
         params.numTrials = strtol( optarg, NULL, 10 );
      }
      else if( c == 'f' ) {
         params.from = strtol( optarg, NULL, 10 );
      }
      else if( c == 't' ) {
         params.to = strtol( optarg, NULL, 10 );
      }
      else if( c == 'v' ) {
         params.verbose = true;
      }
      else {
         PrintUsage( argv[0] );
         return false;
      }
   }
   return true;
}
////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////
int main( int argc, char ** argv ) {

   TestParams params;
   
   if( !ParseCmdLineArgs( params, argc, argv ) ) {
      exit( -1 );
   }
   const U64 maxSize = params.maxsize;
   
   U64 size = params.maxsize;

   cout << "=== mem-latency ===" << endl;
   DEBUG_PRINT( "sizeof( char* ) = " << sizeof( char* ) );
   
   srand( 1 );
   
   char ** buf = AllocAlignAndInitBuffer( params );
   char ** pos;

   timespec * clks = (timespec*) calloc( 2 * params.numTrials, sizeof( timespec ) );
   
   while( size > ( params.minsize >> 1 ) ) {
      size >>= 1;
   }
   
   do {
      
      size <<= 1;
      
      pos = MeasureLatency( buf, size, params, clks );
      if( pos == 0ULL ) {
         cout << "pos = " << pos << endl;
      }
      AnalyzeRunTimeWallTime( clks, params, size, NUMCHASE );
      
   } while( size < maxSize );
   
   
   FreeAlign( (char*) buf );
   free( clks );
   
   DEBUG_PRINT( "Exiting main()" );
   DEBUG_PRINT( "" );
   return 0;
}
