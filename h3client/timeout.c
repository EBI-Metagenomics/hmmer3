#include "timeout.h"
#include "nng/nng.h"
#include "nng/supplemental/util/platform.h"

int h3client_timeout(long deadline)
{
  long timeout = deadline - nng_clock();
  if (timeout < 0) timeout = 0;
  return (int)timeout;
}
