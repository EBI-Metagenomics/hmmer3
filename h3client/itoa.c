#include "itoa.h"

static void reverse(signed char buf[], int size)
{
  signed char *end = buf + size - 1;

  while (buf < end)
  {
    signed char tmp = *buf;
    *buf = *end;
    *end = tmp;
    buf++;
    end--;
  }
}

// ACK: nanoprintf
static int itoa_rev(char *buf, int i)
{
  int n = 0;
  const int sign = (i >= 0) ? 1 : -1;
  do
  {
    *buf++ = (char)('0' + (sign * (i % 10)));
    i /= 10;
    ++n;
  } while (i);
  return n;
}

int h3client_itoa(char *buf, int i)
{
  int n = itoa_rev(buf, i);
  reverse((signed char *)buf, n);
  return n;
}
