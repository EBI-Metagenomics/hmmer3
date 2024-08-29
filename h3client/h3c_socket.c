#include "h3c_socket.h"
#include "body.h"
#include "discard.h"
#include "full_recv.h"
#include "full_send.h"
#include "h3c_errnum.h"
#include "head.h"
#include <arpa/inet.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/socket.h>
#include <unistd.h>

#define MIN_BUFFSIZE 0xFFFF

struct h3c_socket
{
  int fd;
  size_t capacity;
  unsigned char *buffer;
  struct body *body;
};

struct h3c_socket *h3c_socket_new(void)
{
  struct h3c_socket *x = malloc(sizeof(struct h3c_socket));
  if (!x) return NULL;
  x->capacity = MIN_BUFFSIZE;
  if (!(x->buffer = malloc(x->capacity)))
  {
    free(x);
    return NULL;
  }
  if (!(x->body = body_new()))
  {
    free(x->buffer);
    free(x);
    return NULL;
  }
  return x;
}

int h3c_socket_dial(struct h3c_socket *x, char const *ip, int port,
                    long timeout)
{
  if ((x->fd = socket(AF_INET, SOCK_STREAM, 0)) < 0) return 1;

  struct timeval t;
  t.tv_sec = timeout;
  t.tv_usec = 0;

  if (setsockopt(x->fd, SOL_SOCKET, SO_SNDTIMEO, &t, sizeof(t)) < 0)
  {
    perror("Set socket option SO_SNDTIMEO failed");
    close(x->fd);
    return 1;
  }

  struct sockaddr_in srv_addr = {0};
  srv_addr.sin_family = AF_INET;
  srv_addr.sin_port = htons(port);
  if (inet_pton(AF_INET, ip, &srv_addr.sin_addr) <= 0)
  {
    perror("Invalid address or address not supported");
    close(x->fd);
    return 1;
  }

  if (connect(x->fd, (struct sockaddr *)&srv_addr, sizeof(srv_addr)) < 0)
  {
    perror("Connection failed");
    close(x->fd);
    return EXIT_FAILURE;
  }

  return 0;
}

int h3c_socket_warmup(struct h3c_socket *x)
{
  static unsigned char const warm[] = "@--hmmdb 1 "
                                      "--hmmdb_range "
                                      "0..0 --acc\n>warmup"
                                      "\n\n//";
  if (full_send(x->fd, sizeof(warm) - 1, warm)) return H3C_ESEND;
  if (full_recv(x->fd, HEAD_SIZE, x->buffer)) return H3C_ERECV;

  struct head head = {0};
  head_parse(&head, x->buffer);

  // We expect eslEFORMAT error for warmup:
  // https://github.com/EddyRivasLab/easel/blob/07ca83ba9ef0414dba9ce0a9331d465b5eb58f2b/easel.h#L107
  if (head.status != 7)
  {
    discard(x->fd, head.msglen);
    return H3C_EWARMUPCODE;
  }

  return discard(x->fd, head.msglen) ? H3C_EDISCARD : 0;
}

static int setup_buffer(struct h3c_socket *x, size_t required_size)
{
  if (required_size <= MIN_BUFFSIZE || required_size <= x->capacity) return 0;

  void *ptr = realloc(x->buffer, required_size);
  if (!ptr) return H3C_ENOMEM;
  x->capacity = required_size;
  x->buffer = ptr;
  return 0;
}

int h3c_socket_send(struct h3c_socket *x, char const cmd[static 1],
                    char const seq[static 1])
{
  sprintf((char *)x->buffer, "@%s\n>NAME\n%s\n//", cmd, seq);

  if (full_send(x->fd, strlen((char const *)x->buffer), x->buffer))
    return H3C_ESEND;
  if (full_recv(x->fd, HEAD_SIZE, x->buffer)) return H3C_ERECV;

  struct head head = {0};
  head_parse(&head, x->buffer);

  int rc = setup_buffer(x, head.msglen);
  if (rc)
  {
    discard(x->fd, head.msglen);
    return rc;
  }

  if (full_recv(x->fd, head.msglen, x->buffer)) return H3C_ERECV;

  if (head.status) return 0;

  return body_parse(x->body, head.msglen, x->buffer);
}

int h3c_socket_recv(struct h3c_socket *x, struct h3r *r)
{
  return body_store(x->body, r);
}

int h3c_socket_hangup(struct h3c_socket *x)
{
  if (shutdown(x->fd, SHUT_RDWR) < 0)
  {
    perror("Shutdown failed");
    close(x->fd);
    return 1;
  }

  return 0;
}

void h3c_socket_del(struct h3c_socket const *x)
{
  if (x)
  {
    if (close(x->fd)) perror("Socket close failed");
    free(x->buffer);
    body_del(x->body);
  }
  free((void *)x);
}
