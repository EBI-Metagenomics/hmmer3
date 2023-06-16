#ifndef H3RESULT_ZERO_CLIP_H
#define H3RESULT_ZERO_CLIP_H

static inline unsigned h3result_zero_clip(int x)
{
  return x > 0 ? (unsigned)x : 0;
}

#endif
