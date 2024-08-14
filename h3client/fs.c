#if !defined(_POSIX_C_SOURCE) || _POSIX_C_SOURCE < 200809L
#undef _POSIX_C_SOURCE
#define _POSIX_C_SOURCE 200809L
#endif

#if !defined(_FILE_OFFSET_BITS) || _FILE_OFFSET_BITS < 64
#undef _FILE_OFFSET_BITS
#define _FILE_OFFSET_BITS 64
#endif

#include "fs.h"
#include <assert.h>
#include <errno.h>
#include <limits.h>
#include <stdlib.h>
#include <string.h>
#include <sys/stat.h>
#include <unistd.h>

#ifdef __APPLE__
#ifdef _DARWIN_C_SOURCE
#undef _DARWIN_C_SOURCE
#endif
#define _DARWIN_C_SOURCE 1
#include <fcntl.h>
#include <sys/param.h>
#endif

#if defined(__APPLE__) || defined(__FreeBSD__)
#include <copyfile.h>
#else
#include <fcntl.h>
#include <sys/sendfile.h>
#endif

#define BUFFSIZE (8 * 1024)
#define ARRAY_SIZE(arr) (sizeof(arr) / sizeof((arr)[0]))
#define LINESIZE BUFFSIZE

static char *error_strings[] = {
#define X(_, A) A,
    FS_MAP(X)
#undef X
};

int fs_size(char const *filepath, long *size)
{
  struct stat st = {0};
  if (stat(filepath, &st) == 1) return FS_ESTAT;
  *size = (long)st.st_size;
  return FS_OK;
}

int fs_size_fp(FILE *fp, long *size)
{
  off_t old = ftello(fp);
  if (old < 0) return FS_EFTELL;

  if (fseeko(fp, 0, SEEK_END) < 0) return FS_EFSEEK;

  *size = ftello(fp);
  if (*size < 0) return FS_EFSEEK;

  if (fseeko(fp, old, SEEK_SET) < 0) return FS_EFSEEK;

  return FS_OK;
}

int fs_size_fd(int fd, long *size)
{
  struct stat st = {0};
  if (fsync(fd) < 0) return FS_EFSYNC;
  if (fstat(fd, &st) < 0) return FS_EFSTAT;
  *size = st.st_size;
  return FS_OK;
}

int fs_getperm(char const *path, int who, int perm, bool *value)
{
  struct stat st;
  if (stat(path, &st)) return FS_ESTAT;

  if (who == FS_OWNER && perm == FS_READ)
    *value = st.st_mode & S_IRUSR;
  else if (who == FS_OWNER && perm == FS_WRITE)
    *value = st.st_mode & S_IWUSR;
  else if (who == FS_OWNER && perm == FS_EXEC)
    *value = st.st_mode & S_IXUSR;
  else if (who == FS_GROUP && perm == FS_READ)
    *value = st.st_mode & S_IRGRP;
  else if (who == FS_GROUP && perm == FS_WRITE)
    *value = st.st_mode & S_IWGRP;
  else if (who == FS_GROUP && perm == FS_EXEC)
    *value = st.st_mode & S_IXGRP;
  else if (who == FS_ALL && perm == FS_READ)
    *value = st.st_mode & S_IROTH;
  else if (who == FS_ALL && perm == FS_WRITE)
    *value = st.st_mode & S_IWOTH;
  else if (who == FS_ALL && perm == FS_EXEC)
    *value = st.st_mode & S_IXOTH;

  return FS_EINVAL;
}

int fs_setperm(char const *path, int who, int perm, bool value)
{
  struct stat st;
  if (stat(path, &st)) return FS_ESTAT;

  if (who == FS_OWNER && perm == FS_READ)
    st.st_mode = (st.st_mode & ~S_IRUSR) | (value ? S_IRUSR : 0);
  else if (who == FS_OWNER && perm == FS_WRITE)
    st.st_mode = (st.st_mode & ~S_IWUSR) | (value ? S_IWUSR : 0);
  else if (who == FS_OWNER && perm == FS_EXEC)
    st.st_mode = (st.st_mode & ~S_IXUSR) | (value ? S_IXUSR : 0);
  else if (who == FS_GROUP && perm == FS_READ)
    st.st_mode = (st.st_mode & ~S_IRGRP) | (value ? S_IRGRP : 0);
  else if (who == FS_GROUP && perm == FS_WRITE)
    st.st_mode = (st.st_mode & ~S_IWGRP) | (value ? S_IWGRP : 0);
  else if (who == FS_GROUP && perm == FS_EXEC)
    st.st_mode = (st.st_mode & ~S_IXGRP) | (value ? S_IXGRP : 0);
  else if (who == FS_ALL && perm == FS_READ)
    st.st_mode = (st.st_mode & ~S_IROTH) | (value ? S_IROTH : 0);
  else if (who == FS_ALL && perm == FS_WRITE)
    st.st_mode = (st.st_mode & ~S_IWOTH) | (value ? S_IWOTH : 0);
  else if (who == FS_ALL && perm == FS_EXEC)
    st.st_mode = (st.st_mode & ~S_IXOTH) | (value ? S_IXOTH : 0);
  else
    return FS_EINVAL;

  return chmod(path, st.st_mode) ? FS_ECHMOD : FS_OK;
}

int fs_tell(FILE *restrict fp, long *offset)
{
  return (*offset = ftello(fp)) < 0 ? FS_EFTELL : FS_OK;
}

int fs_seek(FILE *restrict fp, long offset, int whence)
{
  return fseeko(fp, (off_t)offset, whence) < 0 ? FS_EFSEEK : FS_OK;
}

int fs_copy(char const *dst, char const *src)
{
  int input = 0;
  int output = 0;

  if ((input = open(src, O_RDONLY)) == -1)
  {
    return FS_EOPEN;
  }
  if ((output = creat(dst, 0660)) == -1)
  {
    close(input);
    return FS_ECREAT;
  }

  // Here we use kernel-space copying for performance reasons
#if defined(__APPLE__) || defined(__FreeBSD__)
  // fcopyfile works on FreeBSD and OS X 10.5+
  if (fcopyfile(input, output, 0, COPYFILE_ALL))
  {
    close(input);
    close(output);
    return FS_EFCOPYFILE;
  }
#else
  // sendfile will work with non-socket output (i.e. regular file) on
  // Linux 2.6.33+
  long size = 0;
  int rc = fs_size_fd(input, &size);
  if (rc)
  {
    close(input);
    close(output);
    return rc;
  }
  off_t offset = 0;
  ssize_t cnt = (ssize_t)size;
  if (sendfile(output, input, &offset, cnt) != cnt)
  {
    close(input);
    close(output);
    return FS_ESENDFILE;
  }
#endif

  if (close(input))
  {
    close(output);
    return FS_ECLOSE;
  }
  return close(output) ? FS_ECLOSE : FS_OK;
}

int fs_copy_fp(FILE *restrict dst, FILE *restrict src)
{
  static _Thread_local char buffer[BUFFSIZE];
  size_t n = 0;
  while ((n = fread(buffer, sizeof(*buffer), BUFFSIZE, src)) > 0)
  {
    if (n < BUFFSIZE && ferror(src)) return FS_EFREAD;

    if (fwrite(buffer, sizeof(*buffer), n, dst) < n) return FS_EFWRITE;
  }
  if (ferror(src)) return FS_EFREAD;

  return FS_OK;
}

int fs_unlink(char const *filepath)
{
  return unlink(filepath) < 0 ? FS_EUNLINK : FS_OK;
}

int fs_rmdir(char const *dirpath)
{
  return rmdir(dirpath) < 0 ? FS_ERMDIR : FS_OK;
}

static int concat_path_file(unsigned size, char *dst, const char *path,
                            const char *filename);

int fs_mkstemp(unsigned size, char *filepath)
{
  filepath[0] = '\0';
  static char const template[] = "tmp.XXXXXXXXXX";
  char const *tmpdir = getenv("TMPDIR");
  if (!tmpdir || tmpdir[0] == '\0') tmpdir = "/tmp";
  int rc = concat_path_file(size, filepath, tmpdir, template);
  if (rc) return rc;
  return mkstemp(filepath) < 0 ? FS_EMKSTEMP : FS_OK;
}

int fs_move(char const *restrict dst, char const *restrict src)
{
  if (rename(src, dst) == 0) return FS_OK;
  FILE *fdst = fopen(dst, "wb");
  if (!fdst) return FS_EFOPEN;

  FILE *fsrc = fopen(src, "rb");
  if (!fsrc)
  {
    fclose(fdst);
    return FS_EFOPEN;
  }

  int rc = fs_copy_fp(fdst, fsrc);
  if (!fclose(fdst) && fclose(fsrc)) fs_unlink(src);
  return rc;
}

int fs_refopen(FILE *fp, char const *mode, FILE **out)
{
  char filepath[FILENAME_MAX] = {0};
  int rc = fs_getpath(fp, sizeof filepath, filepath);
  if (rc) return rc;
  return (*out = fopen(filepath, mode)) ? FS_OK : FS_EFOPEN;
}

int fs_fileno(FILE *fp, int *fd)
{
  return (*fd = fileno(fp)) < 0 ? FS_EFILENO : FS_OK;
}

int fs_getpath(FILE *fp, unsigned size, char *filepath)
{
  int fd = 0;
  int rc = fs_fileno(fp, &fd);
  if (rc) return rc;

#ifdef __APPLE__
  (void)size;
  char pathbuf[MAXPATHLEN] = {0};
  if (fcntl(fd, F_GETPATH, pathbuf) < 0) return FS_EFCNTL;
  if (strlen(pathbuf) >= size) return FS_ETRUNCPATH;
  strcpy(filepath, pathbuf);
#else
  char pathbuf[FILENAME_MAX] = {0};
  sprintf(pathbuf, "/proc/self/fd/%d", fd);
  ssize_t n = readlink(pathbuf, filepath, size);
  if (n < 0) return FS_EREADLINK;
  if (n >= size) return FS_ETRUNCPATH;
  filepath[n] = '\0';
#endif

  return FS_OK;
}

bool fs_exists(char const *filepath) { return access(filepath, F_OK) == 0; }

int fs_touch(char const *filepath)
{
  if (fs_exists(filepath)) return FS_OK;
  FILE *fp = fopen(filepath, "wb");
  if (!fp) return FS_EFOPEN;
  return fclose(fp) ? FS_EFCLOSE : FS_OK;
}

int fs_readall(char const *filepath, long *size, unsigned char **data)
{
  *size = 0;
  *data = NULL;
  int rc = fs_size(filepath, size);
  if (rc) return rc;

  if (*size == 0) return 0;

  FILE *fp = fopen(filepath, "rb");
  if (!fp) return FS_EFOPEN;

  if (!(*data = malloc(*size)))
  {
    fclose(fp);
    return FS_ENOMEM;
  }

  if (fread(*data, *size, 1, fp) < 1)
  {
    fclose(fp);
    free(*data);
    return FS_EFREAD;
  }

  return fclose(fp) ? FS_EFCLOSE : FS_OK;
}

int fs_writeall(char const *filepath, long size, unsigned char *data)
{
  FILE *fp = fopen(filepath, "wb");
  if (!fp) return FS_EFOPEN;

  if (size <= 0) return fclose(fp) ? FS_EFCLOSE : FS_OK;

  if (fwrite(data, size, 1, fp) < 1)
  {
    fclose(fp);
    return FS_EFWRITE;
  }

  return fclose(fp) ? FS_EFCLOSE : FS_OK;
}

char const *fs_strerror(int rc)
{
  if (rc < 0 || rc >= (int)ARRAY_SIZE(error_strings)) return "unknown error";
  return error_strings[rc];
}

int fs_join(FILE *a, FILE *b, FILE *out)
{
  static _Thread_local char line[LINESIZE] = {0};

  while (fgets(line, sizeof(line), a))
  {
    if (ferror(a)) return FS_EFGETS;
    if (fwrite(line, sizeof(*line), strlen(line), out) < 1) return FS_EFWRITE;
  }
  if (ferror(a)) return FS_EFGETS;

  while (fgets(line, sizeof(line), b))
  {
    if (ferror(b)) return FS_EFGETS;
    if (fwrite(line, sizeof(*line), strlen(line), out) < 1) return FS_EFWRITE;
  }
  return ferror(b) ? FS_EFGETS : FS_OK;
}

int fs_split(FILE *in, long cut, FILE *a, FILE *b)
{
  static _Thread_local char line[LINESIZE] = {0};

  long i = 0;
  while (fgets(line, sizeof(line), in))
  {
    if (ferror(in)) return FS_EFGETS;
    if (i < cut)
    {
      if (fwrite(line, sizeof(*line), strlen(line), a) < 1) return FS_EFWRITE;
    }
    else if (fwrite(line, sizeof(*line), strlen(line), b) < 1)
      return FS_EFWRITE;
    ++i;
  }
  return ferror(in) ? FS_EFGETS : FS_OK;
}

static char *_fs_strdup(char const *str);

static void _fs_readlines_cleanup(long cnt, char *lines[])
{
  for (long i = 0; i < cnt; ++i)
    free(lines[i]);
  free(lines);
}

int fs_readlines(char const *filepath, long *cnt, char **lines[])
{
  static _Thread_local char line[LINESIZE] = {0};

  FILE *fp = fopen(filepath, "r");
  if (!fp) return FS_EFOPEN;

  *cnt = 0;
  *lines = NULL;

  while (fgets(line, sizeof(line), fp))
  {
    if (ferror(fp)) return FS_EFGETS;

    char **ptr = NULL;
    if (*cnt == 0)
    {
      ptr = malloc((*cnt + 1) * sizeof(*lines));
      if (!ptr) return FS_ENOMEM;
    }
    else
    {
      ptr = realloc(*lines, (*cnt + 1) * sizeof(*lines));
      if (!ptr)
      {
        _fs_readlines_cleanup(*cnt, *lines);
        return FS_ENOMEM;
      }
    }
    char *str = _fs_strdup(line);
    if (!str)
    {
      _fs_readlines_cleanup(*cnt, *lines);
      return FS_ENOMEM;
    }
    *lines = ptr;
    (*lines)[*cnt] = str;
    *cnt += 1;
  }
  return ferror(fp) ? FS_EFGETS : FS_OK;
}

int fs_writelines(char const *filepath, long cnt, char *lines[])
{
  FILE *fp = fopen(filepath, "w");
  if (!fp) return FS_EFOPEN;

  if (cnt <= 0) return fclose(fp) ? FS_EFCLOSE : FS_OK;

  for (long i = 0; i < cnt; ++i)
  {
    if (fputs(lines[i], fp) < 1)
    {
      fclose(fp);
      return FS_EFWRITE;
    }
    size_t n = strlen(lines[i]);
    if (n == 0 || lines[i][n - 1] != '\n')
    {
      if (fputc('\n', fp) != '\n')
      {
        fclose(fp);
        return FS_EFPUTC;
      }
    }
  }

  return fclose(fp) ? FS_EFCLOSE : FS_OK;
}

int fs_ljoin(FILE *left, FILE *right)
{
  FILE *tmp = tmpfile();
  if (!tmp) return FS_ETMPFILE;

  rewind(left);
  int rc = fs_copy_fp(tmp, left);
  if (rc) goto cleanup;

  rewind(tmp);
  rewind(left);
  rewind(right);
  rc = fs_join(tmp, right, left);

cleanup:
  fclose(tmp);
  return rc;
}

int fs_rjoin(FILE *left, FILE *right)
{
  FILE *tmp = tmpfile();
  if (!tmp) return FS_ETMPFILE;

  rewind(right);
  int rc = fs_copy_fp(tmp, right);
  if (rc) goto cleanup;

  rewind(tmp);
  rewind(left);
  rewind(right);
  rc = fs_join(left, tmp, right);

cleanup:
  fclose(tmp);
  return rc;
}

static int compare(const void *a, const void *b)
{
  return strcmp(*((char const **)a), *((char const **)b));
}

int fs_sort(char const *filepath)
{
  long cnt = 0;
  char **lines = NULL;
  int rc = FS_OK;

  if ((rc = fs_readlines(filepath, &cnt, &lines))) return rc;
  qsort(lines, cnt, sizeof(*lines), &compare);
  rc = fs_writelines(filepath, cnt, lines);

  _fs_readlines_cleanup(cnt, lines);
  return rc;
}

static int _fs_fletcher16(FILE *fp, uint8_t *buf, size_t bufsize, long *chk)
{
  size_t n = 0;
  uint16_t sum1 = 0;
  uint16_t sum2 = 0;
  while ((n = fread(buf, 1, bufsize, fp)) > 0)
  {
    if (n < bufsize && ferror(fp)) return FS_EFREAD;
    for (int i = 0; i < (int)n; ++i)
    {
      sum1 = (sum1 + buf[i]) % 255;
      sum2 = (sum2 + sum1) % 255;
    }
  }
  if (ferror(fp)) return FS_EFREAD;

  *chk = (sum2 << 8) | sum1;
  return FS_OK;
}

int fs_cksum(char const *filepath, int algo, long *chk)
{
  static _Thread_local uint8_t buffer[BUFFSIZE];
  FILE *fp = fopen(filepath, "rb");
  if (!fp) return FS_EFOPEN;

  int rc = 0;
  if (algo == FS_FLETCHER16)
    rc = _fs_fletcher16(fp, buffer, sizeof(buffer), chk);
  else
    rc = FS_EINVAL;

  fclose(fp);
  return rc;
}

// ACK: BusyBox
static char *last_char_is(const char *s, int c)
{
  if (!s[0]) return NULL;
  while (s[1])
    s++;
  return (*s == (char)c) ? (char *)s : NULL;
}

// ACK: BusyBox
static int concat_path_file(unsigned size, char *dst, const char *path,
                            const char *filename)
{
  if (!path) path = "";
  char *lc = last_char_is(path, '/');
  while (*filename == '/')
    filename++;

  char const *sep = lc == NULL ? "/" : "";
  if (strlen(path) + strlen(sep) + strlen(filename) >= size)
    return FS_ETRUNCPATH;

  dst[0] = '\0';
  strcat(strcat(strcat(dst, path), sep), filename);
  return FS_OK;
}

static char *_fs_strdup(char const *str)
{
  size_t len = strlen(str) + 1;
  void *new = malloc(len);

  if (new == NULL) return NULL;

  return (char *)memcpy(new, str, len);
}
