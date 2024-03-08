#pragma once

#if defined(LSPCPP_USEGC)
#define GC_THREADS
#include <gc.h>
#endif

class GCThreadContext
{
public:
    GCThreadContext();
    ~GCThreadContext();

private:
#if defined(LSPCPP_USEGC)
    GC_stack_base gsb;
#endif

};