#pragma once

#if defined(USEGC)
#define GC_THREADS
#include <gc.h>
#endif

class GCThreadContext
{
public:
    GCThreadContext();
    ~GCThreadContext();

private:
#if defined(USEGC)
    GC_stack_base gsb;
#endif

};