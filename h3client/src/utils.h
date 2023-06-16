#ifndef UTILS_H
#define UTILS_H

#include <stdbool.h>
#include <stdint.h>
#include <stdlib.h>

uint64_t h3client_eatu64(unsigned char const **data);
int64_t h3client_eati64(unsigned char const **data);

uint32_t h3client_eatu32(unsigned char const **data);
int32_t h3client_eati32(unsigned char const **data);

uint8_t h3client_eatu8(unsigned char const **data);

double h3client_eatf64(unsigned char const **data);
float h3client_eatf32(unsigned char const **data);

int h3client_eatstr(char **dst, unsigned char const **data);
char *h3client_strskip(char **str);

#define ESCAPE_OVERRUN(rc, cur, end, sz)                                       \
  do                                                                           \
  {                                                                            \
    if ((end) < (cur) + (sz))                                                  \
    {                                                                          \
      rc = H3CLIENT_EPARSE;                                                    \
      goto cleanup;                                                            \
    }                                                                          \
  } while (0);

bool h3client_expect_n_strings(size_t size, char const *ptr, unsigned n);

struct lip_file;

bool h3client_expect_key(struct lip_file *f, char const *key);
bool h3client_expect_array_size(struct lip_file *f, unsigned size);
bool h3client_expect_map_size(struct lip_file *f, unsigned size);

int h3client_read_string(struct lip_file *f, char **str);

char *h3client_strxdup(char *dst, char const *src);

#define MAX(a, b) (((a) > (b)) ? (a) : (b))

static inline unsigned zero_clip(int x) { return x > 0 ? (unsigned)x : 0; }

#define DEL(ptr)                                                               \
  do                                                                           \
  {                                                                            \
    free((ptr));                                                               \
    (ptr) = 0;                                                                 \
  } while (0);

#endif
