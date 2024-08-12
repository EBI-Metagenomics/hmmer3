#ifndef ZERO_CLIP_H
#define ZERO_CLIP_H

static inline unsigned zero_clip(int x)
{
  return x > 0 ? (unsigned)x : 0;
}

#endif
