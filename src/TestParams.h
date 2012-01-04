#pragma once
class TestParams {
   public:
   
   bool csv;
   bool verbose;
   U64 size;
   U32 numTrials;
   U32 from;
   U32 to;
   U32 cacheLineSize;
   
   TestParams() {
      // defaults:
      // memsize = 64M
      size = 64 * 1024 * 1024;
      cacheLineSize = 64;
      numTrials = 1000;
      from = 0;
      to   = 0;
      csv  = false;
      verbose = false;
   }   
};

