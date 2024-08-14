#ifndef HMMD_STATUS_H
#define HMMD_STATUS_H

#include <stddef.h>
#include <stdint.h>

struct hmmd_status
{
  uint32_t status;
  uint64_t msglen;
};

void h3client_hmmd_status_init(struct hmmd_status *);

void h3client_hmmd_status_parse(struct hmmd_status *status, size_t *read_size,
                                unsigned char const *data);

#endif
