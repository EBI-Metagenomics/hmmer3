#include "aye.h"
#include "h3c_socket.h"
#include "h3r_result.h"
#include "sequences.h"
#include <fcntl.h>
#include <stdlib.h>
#include <unistd.h>

#define IP "127.0.0.1"
#define PORT 51371

static long fletcher16(int fd, uint8_t *buf, size_t bufsize)
{
  ssize_t n = 0;
  uint16_t sum1 = 0;
  uint16_t sum2 = 0;
  while ((n = read(fd, buf, bufsize)) > 0)
  {
    for (int i = 0; i < (int)n; ++i)
    {
      sum1 = (sum1 + buf[i]) % 255;
      sum2 = (sum2 + sum1) % 255;
    }
  }
  if (n < 0)
  {
    fprintf(stderr, "Hash failure (line %d)", __LINE__);
    exit(1);
  }

  return (sum2 << 8) | sum1;
}

static long hash(char const *filepath)
{
  static uint8_t buffer[8 * 1024];
  int fd = open(filepath, O_RDONLY);
  if (!fd)
  {
    fprintf(stderr, "Failed to open file (line %d): %s", __LINE__, filepath);
    exit(1);
  }

  long chk = fletcher16(fd, buffer, sizeof(buffer));

  aye(!close(fd));
  return chk;
}

int main(void)
{
  aye_begin();
  struct h3c_socket *x = h3c_socket_new();
  aye(x);
  aye(!h3c_socket_dial(x, IP, PORT, 5));
  aye(!h3c_socket_warmup(x));

  static char const cmd[] = "--hmmdb 1 --acc --cut_ga";
  aye(!h3c_socket_send(x, cmd, good_sequence));

  struct h3r *result = h3r_new();
  aye(!h3c_socket_recv(x, result));

  int file = open("h3result.mp", O_WRONLY | O_CREAT | O_TRUNC, 0644);
  aye(file >= 0);
  aye(!h3r_pack(result, file));
  aye(!close(file));

  long h = hash("h3result.mp");
  printf("Hash: %ld\n", h);
  fflush(stdout);
  aye(h == 19340L || h == 59239L || h == 0L);

  aye(!h3c_socket_hangup(x));
  h3c_socket_del(x);
  remove("h3result.mp");
  h3r_del(result);
  return aye_end();
}
