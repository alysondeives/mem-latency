#pragma once
#include <Types.h>
void * MallocAlign( U64 size, U64 align );
void   FreeAlign( char *ptr );
