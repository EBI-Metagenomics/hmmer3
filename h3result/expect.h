#ifndef H3RESULT_EXPECT_H
#define H3RESULT_EXPECT_H

#include <stdbool.h>
#include <stdint.h>

struct lip_file;

bool h3result_expect_key(struct lip_file *f, char const *key);
bool h3result_expect_array_size(struct lip_file *f, unsigned size);
bool h3result_expect_map_size(struct lip_file *f, unsigned size);
int h3result_read_string(struct lip_file *f, char **str);

#endif
