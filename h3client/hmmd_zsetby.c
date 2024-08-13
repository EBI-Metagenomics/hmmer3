#include "hmmd_zsetby.h"
#include "h3c_errno.h"

int h3client_hmmd_zsetby_parse(enum hmmd_zsetby *dst,
                               unsigned char const **data)
{
  int rc = 0;
  switch (**data)
  {
  case 0:
    *dst = HMMD_ZSETBY_NTARGETS;
    break;
  case 1:
    *dst = HMMD_ZSETBY_OPTION;
    break;
  case 2:
    *dst = HMMD_ZSETBY_FILEINFO;
    break;
  default:
    rc = H3C_EUNPACK;
    break;
  }
  (*data) += 1;
  return rc;
}
