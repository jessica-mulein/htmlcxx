#ifndef __FIX_WIN_CSTRING_H__
#define __FIX_WIN_CSTRING_H__

#if defined(WIN32) && !defined(__MINGW32__)

/*
 * some functions have strange names on windows
 */

#ifndef strcasecmp
#define strcasecmp _stricmp
#endif

#ifndef strncasecmp
#define strncasecmp _strnicmp
#endif

#ifndef snprintf
#define snprintf(buf, n, format, ...) _snprintf_s(buf, n, format, __VA_ARGS__)
#endif

#endif  // WIN32

#endif // __FIX_WIN_CSTRING_H__
