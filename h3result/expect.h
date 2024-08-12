#ifndef H3RESULT_EXPECT_H
#define H3RESULT_EXPECT_H

#include <stdbool.h>
#include <stdint.h>

struct lio_reader;

bool h3result_expect_key(struct lio_reader *f, char const *key);
bool h3result_expect_array_size(struct lio_reader *f, unsigned size);
bool h3result_expect_map_size(struct lio_reader *f, unsigned size);
int h3result_read_string(struct lio_reader *f, char **str);

#endif
