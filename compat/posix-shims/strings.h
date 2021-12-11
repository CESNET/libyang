#include <compat.h>

#ifndef HAVE_STRCASECMP
#ifdef _MSC_VER
#define strcasecmp _stricmp
#else
#error No strcasecmp() implementation for this platform is available.
#endif
#endif
