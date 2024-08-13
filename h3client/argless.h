#ifndef ARGLESS_H
#define ARGLESS_H

#include <stdbool.h>

struct argl_option;

struct argl
{
  struct argl_option const *options;
  char const *args_doc;
  char const *doc;
  char const *version;

  int argc;
  char **argv;
};

void argl_parse(struct argl *, int argc, char *argv[]);
bool argl_has(struct argl const *, char const *name);
char const *argl_get(struct argl const *, char const *name);
int argl_nargs(struct argl const *);
char **argl_args(struct argl const *);
void argl_usage(struct argl const *);
void argl_usage2(struct argl const *, int exit_status);
char const *argl_program(struct argl const *);

struct argl_text
{
  char const *name;
  char const *default_value;
};

struct argl_flag
{
  int : 0;
};

struct argl_def
{
  int type;
  union
  {
    struct argl_text s;
    struct argl_flag b;
  };
};

enum
{
  ARGL_TEXT_TYPE = 0,
  ARGL_FLAG_TYPE = 1,
};

#define ARGL_TEXT(NAME, DEFAULT)                                               \
  (struct argl_def)                                                            \
  {                                                                            \
    ARGL_TEXT_TYPE,                                                            \
    {                                                                          \
      .s = { NAME, DEFAULT }                                                   \
    }                                                                          \
  }
#define ARGL_FLAG()                                                            \
  (struct argl_def)                                                            \
  {                                                                            \
    ARGL_FLAG_TYPE,                                                            \
    {                                                                          \
      {                                                                        \
        0                                                                      \
      }                                                                        \
    }                                                                          \
  }

struct argl_option
{
  char const *name;
  char const key;
  struct argl_def arg_def;
  char const *arg_doc;
};

#define ARGL_NULL ((void *)0)

#define ARGL_HELP_KEY '?'
#define ARGL_USAGE_KEY -1
#define ARGL_VERSION_KEY 'V'

#define ARGL_HELP_OPT                                                          \
  {                                                                            \
    "help", ARGL_HELP_KEY, ARGL_FLAG(), "Give this help list"                  \
  }
#define ARGL_USAGE_OPT                                                         \
  {                                                                            \
    "usage", ARGL_USAGE_KEY, ARGL_FLAG(), "Give a short usage message"         \
  }

#define ARGL_VERSION_OPT                                                       \
  {                                                                            \
    "version", ARGL_VERSION_KEY, ARGL_FLAG(), "Print program version"          \
  }

#define ARGL_END                                                               \
  {                                                                            \
    0                                                                          \
  }

#define ARGL_DEFAULT ARGL_HELP_OPT, ARGL_USAGE_OPT, ARGL_VERSION_OPT

#endif
