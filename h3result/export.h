#ifndef H3RESULT_EXPORT_H
#define H3RESULT_EXPORT_H

/* clang-format off */
#ifdef H3RESULT_STATIC_DEFINE
#  define H3RESULT_API
#else
#  ifdef imm_EXPORTS /* We are building this library */
#    ifdef _WIN32
#      define H3RESULT_API __declspec(dllexport)
#    else
#      define H3RESULT_API __attribute__((visibility("default")))
#    endif
#  else /* We are using this library */
#    ifdef _WIN32
#      define H3RESULT_API __declspec(dllimport)
#    else
#      define H3RESULT_API __attribute__((visibility("default")))
#    endif
#  endif
#endif
/* clang-format on */

#endif
