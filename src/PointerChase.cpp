// ////////////////////////////////////////////////////////////////////////////////
// ////////////////////////////////////////////////////////////////////////////////
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
#define FIVEHUNDREDTHOUSAND HUNDREDTHOUSAND HUNDREDTHOUSAND HUNDREDTHOUSAND HUNDREDTHOUSAND HUNDREDTHOUSAND
#define MILLION FIVEHUNDREDTHOUSAND FIVEHUNDREDTHOUSAND
////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////
#include <iostream>
#include <TimeDiff.h>
#include <PointerChase.h>
////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////
using namespace std;
////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////
char ** PointerChase( char ** buf, char ** p, timespec * clks ) {
   cout << "... PointerChase(): entering" << endl;
   clock_gettime( CLOCK_MONOTONIC, clks ); clks++;
   POINTERCHASE
   clock_gettime( CLOCK_MONOTONIC, clks ); clks++;
   const double diff = 1e9 * DiffTimespec( clks-2, clks-1, double( NUMCHASE ) );
   cout << "... PointerChase(): leaving, diff = " << diff << " nsec" << endl;
   return p;
}
////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////
