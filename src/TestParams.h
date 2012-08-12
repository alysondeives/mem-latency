#pragma once
class TestParams {
   public:
   
   bool csv;
   bool verbose;
   U64 minsize;
   U64 maxsize;
   U64 minalloc;
   U32 numTrials;
   U32 from;
   U32 to;
   U32 cacheLineSize;
   
   TestParams() {
      // defaults:
      // memsize = from 256 bytes to 32M
      minsize  = 256;
      maxsize  = 32 * 1024 * 1024;
      minalloc =  1 * 1024 * 1024;
      cacheLineSize = 64;
      numTrials = 1000;
      from = 0;
      to   = 0;
      csv  = false;
      verbose = false;
   }   
};

