#include "f2c.h"
#include "signal1.h"
#ifdef __cplusplus
extern "C" {
#endif

 ftnint
#ifdef KR_headers
signal_(sigp, proc) integer *sigp; sig_pf proc;
#else
signal_(integer *sigp, sig_pf proc)
#endif
{
	int sig;
	sig = (int)*sigp;

        #ifdef _WIN64
            return (ftnint)(__int64)signal(sig, proc);
        #else
            return (ftnint)(long)signal(sig, proc);
        #endif
	}
#ifdef __cplusplus
}
#endif
