#ifndef DEFER_RETURN_H
#define DEFER_RETURN_H

#define defer_return(x)                                                        \
  do                                                                           \
  {                                                                            \
    rc = x;                                                                    \
    goto defer;                                                                \
  } while (0)

#endif
