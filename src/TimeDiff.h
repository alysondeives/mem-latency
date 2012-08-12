#include <time.h>
#include <Types.h>
#include <TestParams.h>
////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////
#ifdef __MAC_OSX__
#define CLOCK_MONOTONIC          0
#include <mach/mach_time.h>
void clock_gettime( const U32 unused, timespec * tsp );
#endif
double DiffTimespec( const timespec * beg, const timespec * end, const double numtrials );
void AnalyzeRunTimeWallTime( const timespec * clks, const TestParams & params, U64 size, const U32 numTrials );
