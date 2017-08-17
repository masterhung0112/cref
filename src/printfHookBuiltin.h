#ifndef _PRINTFHOOKBUILTIN_H
#define _PRINTFHOOKBUILTIN_H 1

#include <stdio.h>	/* FILE */
#include <stdarg.h> /* va_list, va_arg */

#ifdef __cplusplus
extern "C" {
#endif

int builtin_printf(const char *format, ...);
int builtin_fprintf(FILE *stream, const char *format, ...);
int builtin_sprintf(char *str, const char *format, ...);
int builtin_snprintf(char *str, size_t size, const char *format, ...);
int builtin_asprintf(char **str, const char *format, ...);

int builtin_vprintf(const char *format, va_list ap);
int builtin_vfprintf(FILE *stream, const char *format, va_list ap);
int builtin_vsprintf(char *str, const char *format, va_list ap);
int builtin_vsnprintf(char *str, size_t size, const char *format, va_list ap);
int builtin_vasprintf(char **str, const char *format, va_list ap);

#ifdef printf
#undef printf
#endif
#ifdef fprintf
#undef fprintf
#endif
#ifdef sprintf
#undef sprintf
#endif
#ifdef snprintf
#undef snprintf
#endif
#ifdef asprintf
#undef asprintf
#endif
#ifdef vprintf
#undef vprintf
#endif
#ifdef vfprintf
#undef vfprintf
#endif
#ifdef vsprintf
#undef vsprintf
#endif
#ifdef vsnprintf
#undef vsnprintf
#endif
#ifdef vasprintf
#undef vasprintf
#endif

#define printf builtin_printf
#define fprintf builtin_fprintf
#define sprintf builtin_sprintf
#define snprintf builtin_snprintf
#define asprintf builtin_asprintf

#define vprintf builtin_vprintf
#define vfprintf builtin_vfprintf
#define vsprintf builtin_vsprintf
#define vsnprintf builtin_vsnprintf
#define vasprintf builtin_vasprintf

#ifdef __cplusplus
}
#endif

#endif /* _PRINTFHOOKBUILTIN_H */
