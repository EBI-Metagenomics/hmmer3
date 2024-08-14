#ifndef BUFF_H
#define BUFF_H

#include <stddef.h>

struct buff
{
  size_t size;
  size_t capacity;
  unsigned char data[];
};

struct buff *h3client_buff_new(size_t capacity);
int h3client_buff_ensure(struct buff **buff, size_t capacity);
void h3client_buff_del(struct buff const *buff);

#endif
