
#include "obx.h"
#include <stdlib.h>

value* GetArg(value *bp) {
     int n = bp[HEAD+0].i;
     char *s = (char *) pointer(bp[HEAD+1]);
     int len = bp[HEAD+2].i;
     const char *t =
          (0 <= n && n < saved_argc ? saved_argv[n] : "");
     obcopy(s, len, t, 0, bp);
     return bp;
}

value *GetEnv(value *bp) {
     char *name = (char *) pointer(bp[HEAD+0]);
     char *s = (char *) pointer(bp[HEAD+2]);
     int len = bp[HEAD+3].i;
     const char *t = getenv(name);
     if (t == NULL) t = "";
     obcopy(s, len, t, 0, bp);
     return bp;
}

int GetArgc(void) {
  return saved_argc;
}


