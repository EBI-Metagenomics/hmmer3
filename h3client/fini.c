#include "h3client/fini.h"
#include "nng/nng.h"

void h3client_fini(void) { nng_fini(); }
