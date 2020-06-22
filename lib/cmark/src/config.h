#ifndef CMARK_CONFIG_H
#define CMARK_CONFIG_H

#ifdef __cplusplus
extern "C" {
#endif

#include <stdbool.h>

#define HAVE___BUILTIN_EXPECT
#define CMARK_ATTRIBUTE(list) __attribute__ (list)
#define CMARK_INLINE inline

#ifdef __cplusplus
}
#endif

#endif
