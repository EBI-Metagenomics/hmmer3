#include "argless.h"

static bool arg_is_opt(char const *arg);
static bool arg_is_opt_compact(char const *arg);
static bool arg_is_key_opt(char const *arg);
static bool arg_is_key_opt_compact(char const *arg);
static bool arg_is_name_opt(char const *arg);
static bool arg_is_name_opt_compact(char const *arg);
static char const *arg_opt_compact_value(char const *arg);
static bool arg_key_opt_eq(char const *arg, char key);
static bool arg_name_opt_eq(char const *arg, char const *name);

static bool argvec_check_valid(int argc, char *argv[],
                               struct argl_option const *opts, bool die);
static void argvec_sort(int argc, char *argv[], struct argl_option const *opts);
static bool argvec_has(int argc, char *argv[], struct argl_option const *opts,
                       char const *name);
static char const *argvec_get(int argc, char *argv[],
                              struct argl_option const *opts, char const *name);
static int argvec_nargs(int argc, char *argv[], struct argl_option const *opts);
static char **argvec_args(int argc, char *argv[],
                          struct argl_option const *opts);

static char const *al_basename(char const *path);

static void echo_start(int indent_width);
static void echof(char const *fmt, ...);
static void echos(char const *str);
static void echoc(char c);
static void echor(char const *str);
static void echor2(char const *str, char const *default_value);
static void echo_flush(void);
static void echo_end(void);

static void help_usage(char const *program, struct argl_option const *opts,
                       int exit_status);
static void help_help(char const *program, char const *args_doc,
                      char const *doc, struct argl_option const *opts,
                      int exit_status);
static void help_version(char const *prog, char const *version,
                         int exit_status);
static void help_requires_arg(char const *program, char const *arg,
                              int exit_status);
static void help_unrecognized_arg(char const *program, char const *arg,
                                  int exit_status);

static struct argl_option const *opts_search(struct argl_option const *opts,
                                             char const *arg);
static struct argl_option const *opts_get(struct argl_option const *opts,
                                          char const *name);
static int opts_count(struct argl_option const *opts);
static bool opt_has_user_default(struct argl_option const *opt);
static char const *opt_get_default(struct argl_option const *opt);
static bool opt_is_flag(struct argl_option const *opt);
static char const *opt_arg_name(struct argl_option const *opt);

enum
{
  ARGL_WINDOWS,
  ARGL_UNIX,
  ARGL_MACOS,
};

#if defined(_WIN16) || defined(_WIN32) || defined(_WIN64) || defined(__WIN32__)
#define ARGL_OS ZS_WINDOWS
#elif defined(__unix__) || defined(__unix)
#define ARGL_OS ZS_UNIX
#elif defined(__APPLE__)
#define ARGL_OS ARGL_MACOS
#endif

#ifdef MS_WINDOWS
#define ARGL_PATH_SEP '\\'
#endif

#ifndef ARGL_PATH_SEP
#define ARGL_PATH_SEP '/'
#endif

#include <string.h>

static char const *al_basename(char const *path)
{
  char *p = strrchr(path, ARGL_PATH_SEP);
  return p ? p + 1 : path;
}

#include <stddef.h>
#include <string.h>

static size_t arg_long_compact_opt_size(char const *arg);

static bool arg_is_opt(char const *arg)
{
  return arg_is_key_opt(arg) || arg_is_name_opt(arg);
}

static bool arg_is_opt_compact(char const *arg)
{
  return arg_is_key_opt(arg) ? arg_is_key_opt_compact(arg)
                             : arg_is_name_opt_compact(arg);
}

static bool arg_is_key_opt(char const *arg)
{
  return strlen(arg) > 1 && arg[0] == '-' && arg[1] != '-';
}

static bool arg_is_key_opt_compact(char const *arg)
{
  return strlen(arg) > 2 && arg[0] == '-' && arg[1] != '-';
}

static bool arg_is_name_opt(char const *arg)
{
  return strlen(arg) > 2 && arg[0] == '-' && arg[1] == '-';
}

static bool arg_is_name_opt_compact(char const *arg)
{
  return strchr(arg, '=');
}

static char const *arg_opt_compact_value(char const *arg)
{
  return arg_is_name_opt_compact(arg) ? strchr(arg, '=') + 1 : arg + 2;
}

static bool arg_key_opt_eq(char const *arg, char key) { return arg[1] == key; }

static bool arg_name_opt_eq(char const *arg, char const *name)
{
  size_t size = 0;
  if (arg_is_name_opt_compact(arg))
    size = arg_long_compact_opt_size(arg);
  else
    size = strlen(arg + 2);

  return size == strlen(name) && !strncmp(arg + 2, name, size);
}

static size_t arg_long_compact_opt_size(char const *arg)
{
  char const *p = strchr(arg, '=');
  return p - &arg[2];
}

#include <stddef.h>
#include <string.h>

static struct argl_option const *opts_search(struct argl_option const *opts,
                                             char const *arg)
{
  for (int i = 0; i < opts_count(opts); ++i)
  {
    if (arg_is_key_opt(arg) && arg_key_opt_eq(arg, opts[i].key))
      return &opts[i];
    if (arg_is_name_opt(arg) && arg_name_opt_eq(arg, opts[i].name))
      return &opts[i];
  }
  return NULL;
}

static struct argl_option const *opts_get(struct argl_option const *opts,
                                          char const *name)
{
  for (int i = 0; i < opts_count(opts); ++i)
  {
    if (!strcmp(name, opts[i].name)) return &opts[i];
  }
  return NULL;
}

static int opts_count(struct argl_option const *opts)
{
  struct argl_option const *opt = opts;
  int size = 0;
  while (opt->name)
  {
    size++;
    opt++;
  }
  return size;
}

static bool opt_has_user_default(struct argl_option const *opt)
{
  return !opt_is_flag(opt) && opt->arg_def.s.default_value;
}

static char const *opt_get_default(struct argl_option const *opt)
{
  if (opt_is_flag(opt)) return "false";
  return opt->arg_def.s.default_value;
}

static bool opt_is_flag(struct argl_option const *opt)
{
  return opt->arg_def.type == ARGL_FLAG_TYPE;
}

static char const *opt_arg_name(struct argl_option const *opt)
{
  return opt->arg_def.s.name;
}

#include <stdlib.h>
#include <string.h>

static bool argvec_check_valid(int argc, char *argv[],
                               struct argl_option const *opts, bool die)
{
  bool force_arg = false;
  char const *prg = al_basename(argv[0]);
  for (int i = 1; i < argc; ++i)
  {
    if (!strcmp(argv[i], "--")) force_arg = true;
    if (force_arg || !arg_is_opt(argv[i])) continue;

    struct argl_option const *opt = opts_search(opts, argv[i]);
    if (!opt)
    {
      if (die) help_unrecognized_arg(prg, argv[i], EXIT_FAILURE);
      return false;
    }

    if (!opt_is_flag(opt))
    {
      if (arg_is_opt_compact(argv[i])) continue;
      if (i + 1 == argc || arg_is_opt(argv[i + 1]) ||
          !strcmp(argv[i + 1], "--"))
      {
        if (die) help_requires_arg(prg, argv[i], EXIT_FAILURE);
        return false;
      }
      ++i;
    }
    else
    {
      if (arg_is_opt_compact(argv[i]))
      {
        if (die) help_unrecognized_arg(prg, argv[i], EXIT_FAILURE);
        return false;
      }
    }
  }
  return true;
}

static void argvec_sort(int argc, char *argv[], struct argl_option const *opts)
{
  bool force_arg = false;
  char *first_arg = 0;
  for (int i = 1; i < argc && first_arg != argv[i]; ++i)
  {
    if (!strcmp(argv[i], "--")) force_arg = true;
    if (force_arg || !arg_is_opt(argv[i]))
    {
      char *curr = argv[i];
      size_t size = argc - 1 - i;
      memmove(argv + i, argv + i + 1, sizeof(argv[0]) * size);
      argv[argc - 1] = curr;
      if (!first_arg) first_arg = curr;
      --i;
    }
    else
    {
      struct argl_option const *opt = opts_search(opts, argv[i]);
      i += !opt_is_flag(opt) && !arg_is_opt_compact(argv[i]);
    }
  }
}

static int option_index(int argc, char *argv[], struct argl_option const *opts,
                        char const *name);

static bool argvec_has(int argc, char *argv[], struct argl_option const *opts,
                       char const *name)
{
  return option_index(argc, argv, opts, name) != -1;
}

static char const *argvec_get(int argc, char *argv[],
                              struct argl_option const *opts, char const *name)
{
  int i = option_index(argc, argv, opts, name);
  if (i == -1) return "";

  return arg_is_opt_compact(argv[i]) ? arg_opt_compact_value(argv[i])
                                     : argv[i + 1];
}

static int argvec_nargs(int argc, char *argv[], struct argl_option const *opts)
{
  char **p = argvec_args(argc, argv, opts);
  return &argv[argc] - p;
}

static char **argvec_args(int argc, char *argv[],
                          struct argl_option const *opts)
{
  argvec_sort(argc, argv, opts);
  int i = 1;
  for (; i < argc; ++i)
  {
    if (arg_is_opt(argv[i]))
    {
      struct argl_option const *opt = opts_search(opts, argv[i]);
      if (opt) i += !opt_is_flag(opt) && !arg_is_opt_compact(argv[i]);
    }
    else
      break;
  }
  return argv + i;
}

static int option_index(int argc, char *argv[], struct argl_option const *opts,
                        char const *name)
{
  for (int i = 1; i < argc; ++i)
  {
    if (arg_is_opt(argv[i]))
    {
      struct argl_option const *opt = opts_search(opts, argv[i]);
      if (opt)
      {
        if (!strcmp(opt->name, name)) return i;
        i += !opt_is_flag(opt) && !arg_is_opt_compact(argv[i]);
      }
    }
  }
  return -1;
}

#include <stdarg.h>
#include <stdbool.h>
#include <stddef.h>
#include <stdio.h>
#include <string.h>

#define NCOL 80

static char buf[NCOL + 1] = {0};
static size_t pos = 0;
static size_t width = 0;

static void echo_empty_spaces(int n);
static void vecho(const char *fmt, va_list va);

static void echo_start(int indent_width)
{
  width = (size_t)indent_width;
  pos = 0;
  buf[0] = '\0';
}

static void echof(char const *fmt, ...)
{
  va_list va;
  va_start(va, fmt);
  size_t r = vsnprintf(buf + pos, sizeof(buf) - pos, fmt, va);
  va_end(va);

  if (pos + r < sizeof buf)
  {
    pos += r;
    return;
  }

  buf[pos] = '\0';
  echo_flush();

  echo_empty_spaces((int)width);
  va_start(va, fmt);
  vecho(fmt, va);
  va_end(va);
}

static void echos(char const *str) { echof("%s", str); }

static void echoc(char c) { echof("%c", c); }

static void echor(char const *str)
{
  size_t size = strlen(str) + 2;
  if (sizeof(buf) > pos + size)
  {
    echo_empty_spaces((int)(width - pos + 2));
    echos(str);
  }
  else
  {
    echo_flush();
    echof("  %s", str);
  }
}

static void echor2(char const *str, char const *default_value)
{
  size_t size = strlen(str) + 2;
  if (sizeof(buf) > pos + size)
  {
    echo_empty_spaces((int)(width - pos + 2));
    echof("%s [%s]", str, default_value);
  }
  else
  {
    echo_flush();
    echof("  %s [%s]", str, default_value);
  }
}

static void echo_end(void)
{
  if (pos > 0) puts(buf);
}

static void echo_empty_spaces(int n)
{
  while (--n > 0)
    echof("%c", ' ');
}

static void echo_flush(void)
{
  if (pos > 0) puts(buf);
  pos = 0;
  buf[0] = '\0';
}

static void vecho(const char *fmt, va_list va)
{
  pos += vsnprintf(buf + pos, sizeof(buf) - pos, fmt, va);
}

#include <ctype.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

static void display_try_info(char const *progam_name);

static void help_usage(char const *prog, struct argl_option const *opts,
                       int exit_status)
{
  echo_start(11);
  echof("Usage: %s", prog);

  echos(" [-");
  for (int i = 0; i < opts_count(opts); ++i)
  {
    if (isprint(opts[i].key)) echoc(opts[i].key);
  }
  echos("]");

  for (int i = 0; i < opts_count(opts); ++i)
  {
    if (isprint(opts[i].key) && !opt_is_flag(opts + i))
    {
      echof(" [-%c %s]", opts[i].key, opt_arg_name(opts + i));
    }
    if (!opt_is_flag(opts + i))
      echof(" [--%s=%s]", opts[i].name, opt_arg_name(opts + i));
    else
      echof(" [--%s]", opts[i].name);
  }

  echo_end();
  exit(exit_status);
}

static void help_help(char const *prog, char const *args_doc, char const *doc,
                      struct argl_option const *opts, int exit_status)
{
  echo_start(0);
  echof("Usage: %s [OPTION...]", prog);
  if (args_doc) echof(" %s", args_doc);
  echoc('\n');
  echo_end();

  echo_start(2);
  echoc(' ');
  char const *start = doc;
  char const *end = strchr(doc, ' ');
  while (end)
  {
    echof(" %.*s", (end - start), start);
    start = end + 1;
    end = strchr(start, ' ');
  }
  if (strlen(start) > 0) echof(" %s", start);
  echo_end();

  echo_start(0);
  echoc('\n');
  echos("Options:");
  echo_end();

  echo_start(28);
  for (int i = 0; i < opts_count(opts); ++i)
  {
    if (isprint(opts[i].key))
      echof("  -%c, --%s", opts[i].key, opts[i].name);
    else
      echof("      --%s", opts[i].name);

    if (opt_has_user_default(opts + i))
      echor2(opts[i].arg_doc, opt_get_default(opts + i));
    else
      echor(opts[i].arg_doc);
    echo_flush();
  }
  echo_end();
  exit(exit_status);
}

static void help_version(char const *prog, char const *version, int exit_status)
{
  echo_start(0);
  echof("%s, version %s", prog, version);
  echo_end();
  exit(exit_status);
}

static void help_requires_arg(char const *prog, char const *arg,
                              int exit_status)
{
  printf("%s: option `%s' requires an argument\n", prog, arg);
  display_try_info(prog);
  exit(exit_status);
}

static void help_unrecognized_arg(char const *prog, char const *arg,
                                  int exit_status)
{
  printf("%s: unrecognized option `%s'\n", prog, arg);
  display_try_info(prog);
  exit(exit_status);
}

static void display_try_info(char const *prg)
{
  printf("Try `%s --help' or `%s --usage' for more information.\n", prg, prg);
}

void argl_parse(struct argl *al, int argc, char *argv[])
{
  al->argc = argc;
  al->argv = argv;
  if (!argvec_check_valid(argc, argv, al->options, true)) return;
  argvec_sort(argc, argv, al->options);

  char const *program = al_basename(argv[0]);

  if (argvec_has(argc, argv, al->options, "usage"))
    help_usage(program, al->options, EXIT_SUCCESS);

  if (argvec_has(argc, argv, al->options, "help"))
    help_help(program, al->args_doc, al->doc, al->options, EXIT_SUCCESS);

  if (argvec_has(argc, argv, al->options, "version"))
    help_version(program, al->version, EXIT_SUCCESS);
}

bool argl_has(struct argl const *al, char const *name)
{
  int n = al->argc - argvec_nargs(al->argc, al->argv, al->options);
  return argvec_has(n, al->argv, al->options, name);
}

char const *argl_get(struct argl const *al, char const *name)
{
  struct argl_option const *opt = opts_get(al->options, name);
  if (!opt) return "";

  if (opt_is_flag(opt)) return argl_has(al, name) ? "true" : "false";

  if (argl_has(al, name))
    return argvec_get(al->argc, al->argv, al->options, name);

  char const *value = opt_get_default(opt);
  return value ? value : "";
}

int argl_nargs(struct argl const *al)
{
  return argvec_nargs(al->argc, al->argv, al->options);
}

char **argl_args(struct argl const *al)
{
  return argvec_args(al->argc, al->argv, al->options);
}

void argl_usage(struct argl const *al) { argl_usage2(al, EXIT_FAILURE); }

void argl_usage2(struct argl const *al, int exit_status)
{
  char const *program = al_basename(al->argv[0]);
  help_usage(program, al->options, exit_status);
}

char const *argl_program(struct argl const *al)
{
  return al_basename(al->argv[0]);
}
