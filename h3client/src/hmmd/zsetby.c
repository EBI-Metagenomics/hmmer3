#include "hmmd/zsetby.h"
#include "h3client/h3client.h"

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
    rc = H3CLIENT_EUNPACK;
    break;
  }
  (*data) += 1;
  return rc;
}
