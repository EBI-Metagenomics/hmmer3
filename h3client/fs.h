#ifndef FS_H
#define FS_H

#include <stdbool.h>
#include <stdint.h>
#include <stdio.h>

#define FS_MAP(X)                                                              \
  X(OK, "not an error")                                                        \
  X(ECHMOD, "chmod failed")                                                    \
  X(ECLOSE, "close failed")                                                    \
  X(ECREAT, "creat failed")                                                    \
  X(EFCLOSE, "fclose failed")                                                  \
  X(EFCNTL, "fcntl failed")                                                    \
  X(EFCOPYFILE, "fcopyfile failed")                                            \
  X(EFGETS, "fgets failed")                                                    \
  X(EFILENO, "fileno failed")                                                  \
  X(EFOPEN, "fopen failed")                                                    \
  X(EFPUTC, "fputc failed")                                                    \
  X(EFREAD, "fread failed")                                                    \
  X(EFSEEK, "fseek failed")                                                    \
  X(EFSTAT, "fstat failed")                                                    \
  X(EFSYNC, "fsync failed")                                                    \
  X(EFTELL, "ftell failed")                                                    \
  X(EFWRITE, "fwrite failed")                                                  \
  X(EINVAL, "invalid value")                                                   \
  X(EMKSTEMP, "mkstemp failed")                                                \
  X(ENOMEM, "not enough memory")                                               \
  X(EOPEN, "open failed")                                                      \
  X(EREADLINK, "readlink failed")                                              \
  X(ERMDIR, "rmdir failed")                                                    \
  X(ESENDFILE, "sendfile failed")                                              \
  X(ESTAT, "stat failed")                                                      \
  X(ETMPFILE, "tmpfile failed")                                                \
  X(ETRUNCPATH, "truncated path")                                              \
  X(EUNLINK, "unlink failed")

enum fs_rc
{
#define X(A, _) FS_##A,
  FS_MAP(X)
#undef X
};

enum fs_who
{
  FS_OWNER,
  FS_GROUP,
  FS_ALL,
};

enum fs_perm
{
  FS_READ,
  FS_WRITE,
  FS_EXEC,
};

enum fs_algo
{
  FS_FLETCHER16,
};

int fs_size(char const *filepath, long *size);
int fs_size_fp(FILE *fp, long *size);
int fs_size_fd(int fd, long *size);

int fs_getperm(char const *path, int who, int perm, bool *value);
int fs_setperm(char const *path, int who, int perm, bool value);

int fs_tell(FILE *restrict fp, long *offset);
int fs_seek(FILE *restrict fp, long offset, int whence);

int fs_copy(char const *dst, char const *src);
int fs_copy_fp(FILE *restrict dst, FILE *restrict src);
int fs_unlink(char const *filepath);
int fs_rmdir(char const *dirpath);
int fs_mkstemp(unsigned size, char *filepath);
int fs_move(char const *restrict dst, char const *restrict src);

int fs_refopen(FILE *fp, char const *mode, FILE **out);
int fs_fileno(FILE *fp, int *fd);
int fs_getpath(FILE *fp, unsigned size, char *filepath);

bool fs_exists(char const *filepath);
int fs_touch(char const *filepath);

int fs_readall(char const *filepath, long *size, unsigned char **data);
int fs_writeall(char const *filepath, long size, unsigned char *data);

char const *fs_strerror(int rc);

int fs_join(FILE *a, FILE *b, FILE *out);
int fs_split(FILE *in, long cut, FILE *a, FILE *b);
int fs_readlines(char const *filepath, long *cnt, char **lines[]);
int fs_writelines(char const *filepath, long cnt, char *lines[]);

int fs_ljoin(FILE *left, FILE *right);
int fs_rjoin(FILE *left, FILE *right);

int fs_sort(char const *filepath);
int fs_cksum(char const *filepath, int algo, long *chk);

#endif
