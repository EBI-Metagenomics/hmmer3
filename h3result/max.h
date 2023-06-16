#ifndef H3RESULT_MAX_H
#define H3RESULT_MAX_H

#define h3result_max(a, b)                                                     \
  ({                                                                           \
    __typeof__(a) _a = (a);                                                    \
    __typeof__(b) _b = (b);                                                    \
    _a > _b ? _a : _b;                                                         \
  })

#endif
