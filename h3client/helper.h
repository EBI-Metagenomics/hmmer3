#ifndef HELPER_H
#define HELPER_H

#define array_size(arr) (sizeof(arr) / sizeof((arr)[0]))

#define GOOD 0
#define BAD 1

struct expect
{
  unsigned nhits;
  double ln_evalue;
};

struct query
{
  char const *name;
  char const *desc;
  char const *seq;
  struct expect expect;
};

extern struct query const ross[2];
extern struct query const corrupt[1];

#endif
