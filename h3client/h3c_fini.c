#include "h3c_fini.h"
#include "nng/nng.h"

void h3client_fini(void) { nng_fini(); }
