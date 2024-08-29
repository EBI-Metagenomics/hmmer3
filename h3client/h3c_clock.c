#include "h3c_clock.h"
#include <errno.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/time.h>
#include <time.h>

// Explanation:
// https://github.com/nanomsg/nng/blob/a8a8bd435690e24e55dcdad485e97225902cb0c8/src/platform/posix/posix_config.h#L25
#ifndef USE_CLOCKID
#if defined(__APPLE__)
#define USE_CLOCKID CLOCK_REALTIME
#elif defined(CLOCK_MONOTONIC)
#define USE_CLOCKID CLOCK_MONOTONIC
#elif defined(CLOCK_REALTIME)
#define USE_CLOCKID CLOCK_REALTIME
#else
#error "Could not set clock id."
#endif
#endif

long h3c_clock(void)
{
  struct timespec ts = {0};

  if (clock_gettime(USE_CLOCKID, &ts) != 0)
  {
    fprintf(stderr, "clock_gettime failed: %s", strerror(errno));
    abort();
  }

  long msec = ts.tv_sec;
  msec *= 1000;
  msec += (ts.tv_nsec / 1000000);
  return msec;
}
