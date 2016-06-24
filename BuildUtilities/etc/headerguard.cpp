#include <Rpc.h>
#include <cstdio>

int
main ()
{
  UUID uuid;
  UuidCreate (&uuid);
  unsigned char x[16];
  memcpy (x, &uuid, 16);
  printf ("GUARD_%02X%02X%02X%02X%02X%02X%02X%02X%02X%02X%02X%02X%02X%02X%02X%02X_",
	  x[0],
	  x[1],
	  x[2],
	  x[3],
	  x[4],
	  x[5],
	  x[6],
	  x[7],
	  x[8],
	  x[9],
	  x[10],
	  x[11],
	  x[12],
	  x[13],
	  x[14],
	  x[15]);
	  
  return (0);
}
