#include "hmmd_status.h"
#include "h3c_errno.h"
#include "utils.h"
#include <stdlib.h>
#include <string.h>

void h3client_hmmd_status_init(struct hmmd_status *st)
{
  memset(st, 0, sizeof(*st));
}

void h3client_hmmd_status_parse(struct hmmd_status *status, size_t *read_size,
                                unsigned char const *data)
{
  unsigned char const *ptr = data;
  status->status = h3client_eatu32(&ptr);
  status->msglen = h3client_eatu64(&ptr);
  *read_size = (size_t)(ptr - data);
}
