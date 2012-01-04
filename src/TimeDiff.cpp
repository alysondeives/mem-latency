#include <math.h>
#include <iostream>
#include <Defines.h>
#include <TimeDiff.h>
#include <TestParams.h>
////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////
using namespace std;
////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////
#ifdef __MAC_OSX__
#define FMTCODE "llu"
void clock_gettime( const U32 unused, timespec * tsp ) {
   //
   // NOTE: sizeof( timespec.tv_nsec ) == 8 on Linux
   // NOTE: sizeof( timespec.tv_sec  ) == 8 on Linux
   //
   tsp->tv_nsec = mach_absolute_time();
}
double DiffTimespec( const timespec * beg, const timespec * end, const double numTrials ) {

   static mach_timebase_info_data_t info = {0,0};
   if( info.denom == 0 ) {
      mach_timebase_info(&info);
   }
   U64 diff = end->tv_nsec - beg->tv_nsec;
   U64 dnano = diff * ( info.numer / info.denom );
   return 1e-9*((double)dnano)/numTrials;
}
#else
#define FMTCODE "lu"
double DiffTimespec( const timespec * beg, const timespec * end, const double numTrials ) {
   U64 dsec;
   U64 dnsec;
   if( ( end->tv_nsec - beg->tv_nsec ) < 0 ) {
      dsec  = end->tv_sec - beg->tv_sec - 1ULL;
      dnsec = 1000000000ULL + end->tv_nsec - beg->tv_nsec;
   }
   else {
      dsec  = end->tv_sec  - beg->tv_sec;
      dnsec = end->tv_nsec - beg->tv_nsec;
   }
   return (((double) dsec) + 1e-9*((double) dnsec))/numTrials;
}
#endif
void AnalyzeRunTimeWallTime( const timespec * clks, const TestParams & params, U64 size, const U32 _numChase ) {

   const bool csv = params.csv;
   const U32 numRuns = params.numTrials;
   
   DEBUG_PRINT( "AnalyzeRunTimeWallTime(): numRuns = " << numRuns );
   DEBUG_PRINT( "" );
   double min  = 10e10;
   double max  = 0.0;
   double mean = 0.0;
   double std  = 0.0;
   double numChase = (double) _numChase;
   double dt;
   double nr = (double) numRuns;
   
   const U64 OneK = 1024;
   const U64 OneM = OneK * OneK;
   
   for( U32 runIdx = 0; runIdx < numRuns; runIdx++ ) {
      const timespec * clk0 = clks + 2*runIdx + 0;
      const timespec * clk1 = clks + 2*runIdx + 1;

      double diff = DiffTimespec( clk0, clk1, numChase );
      
      mean += diff;
      if( diff < min ) { min = diff; }
      if( diff > max ) { max = diff; }
   }
   mean /= nr;
   for( U32 runIdx = 0; runIdx < numRuns; runIdx++ ) {
      const timespec * clk0 = clks + 2*runIdx + 0;
      const timespec * clk1 = clks + 2*runIdx + 1;
      dt = DiffTimespec( clk0, clk1, numChase ) - mean;
      std += (dt*dt);
   }
   std = sqrt( std / nr );

   if( csv ) {
      cout << size << "," << 1e9*mean << "," << 1e9*std << endl;
   }
   else {
      if( size < OneK ) {
         printf( "size = %3" FMTCODE "  B, mean = %5.1f +/- %4.1f [ns]\n", size, 1e9*mean, 1e9*std );
      }
      else if( size < OneM ) {
         size /= OneK;
         printf( "size = %3" FMTCODE " KB, mean = %5.1f +/- %4.1f [ns]\n", size, 1e9*mean, 1e9*std );
      }
      else {
         size /= OneM;
         printf( "size = %3" FMTCODE " MB, mean = %5.1f +/- %4.1f [ns]\n", size, 1e9*mean, 1e9*std );
      }      
   }   
}
