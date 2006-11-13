/*
 * $Id: stdlib.h,v 1.18 2006-11-13 09:51:53 obarthel Exp $
 *
 * :ts=4
 *
 * Portable ISO 'C' (1994) runtime library for the Amiga computer
 * Copyright (c) 2002-2006 by Olaf Barthel <olsen (at) sourcery.han.de>
 * All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions
 * are met:
 *
 *   - Redistributions of source code must retain the above copyright
 *     notice, this list of conditions and the following disclaimer.
 *
 *   - Neither the name of Olaf Barthel nor the names of contributors
 *     may be used to endorse or promote products derived from this
 *     software without specific prior written permission.
 *
 * THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS"
 * AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
 * IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE
 * ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT OWNER OR CONTRIBUTORS BE
 * LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR
 * CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF
 * SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS
 * INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN
 * CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE)
 * ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE
 * POSSIBILITY OF SUCH DAMAGE.
 *
 *****************************************************************************
 *
 * Documentation and source code for this library, and the most recent library
 * build are available from <http://sourceforge.net/projects/clib2>.
 *
 *****************************************************************************
 */

#ifndef _STDLIB_H
#define _STDLIB_H

/****************************************************************************/

#ifndef _STDDEF_H
#include <stddef.h>
#endif /* _STDDEF_H */

/****************************************************************************/

#ifdef __cplusplus
extern "C" {
#endif /* __cplusplus */

/****************************************************************************/

/* Maximum number of bytes in a multibyte character */
#define MB_CUR_MAX 2

/****************************************************************************/

/* The maximum value that can be returned by the rand() function */
#define RAND_MAX 2147483647

/****************************************************************************/

/* Return values to be passed to exit() */
#define EXIT_FAILURE 20
#define EXIT_SUCCESS 0

/****************************************************************************/

/* Data structures used by the div() and ldiv() functions */
typedef struct { int quot; int rem; } div_t;
typedef struct { long quot; long rem; } ldiv_t;

/****************************************************************************/

extern int mblen(const char *s, size_t n);
extern int mbtowc(wchar_t *pwc, const char *s, size_t n);
extern int wctomb(char *s, wchar_t wchar);
extern size_t mbstowcs(wchar_t *pwcs, const char *s, size_t n);
extern size_t wcstombs(char *s, const wchar_t *pwcs, size_t n);

/****************************************************************************/

extern void *malloc(size_t size);
extern void *calloc(size_t num_elements,size_t element_size);
extern void free(void *ptr);
extern void *realloc(void *ptr,size_t size);

#ifdef __MEM_DEBUG
extern void *__malloc(size_t size,const char *file,int line);
extern void *__calloc(size_t num_elements,size_t element_size,const char *file,int line);
extern void __free(void *ptr,const char *file,int line);
extern void *__realloc(void *ptr,size_t size,const char *file,int line);

#define malloc(size) __malloc((size),__FILE__,__LINE__)
#define calloc(num_elements,element_size) __calloc((num_elements),(element_size),__FILE__,__LINE__)
#define free(ptr) __free((ptr),__FILE__,__LINE__)
#define realloc(ptr,size) __realloc((ptr),(size),__FILE__,__LINE__)
#endif /* __MEM_DEBUG */

/****************************************************************************/

extern int abs(int x);
extern long labs(long x);
extern div_t div(int n,int d);
extern ldiv_t ldiv(long n,long d);

/****************************************************************************/

extern int rand(void);
extern void srand(unsigned seed);

/****************************************************************************/

extern int system(const char *command);

/****************************************************************************/

extern void exit(int status);
extern void abort(void);

/****************************************************************************/

extern int atexit(void (*)(void));

/****************************************************************************/

extern char * getenv(const char *name);

/****************************************************************************/

extern void * bsearch(const void *key, const void *base, size_t count, size_t size,
	int (*compare)(const void * key,const void * value));
extern void qsort(void *base,size_t count,size_t size,
	int (*compare)(const void * element1,const void * element2));

/****************************************************************************/

extern double strtod(const char *str, char ** ptr);
extern long strtol(const char *str, char **ptr, int base);
extern unsigned long strtoul(const char *str, char **ptr, int base);

/****************************************************************************/

extern double atof(const char *str);
extern int atoi(const char *str);
extern long atol(const char *str);

/****************************************************************************/

/* The following is not part of the ISO 'C' (1994) standard. */

/****************************************************************************/

extern void _exit(int status);
extern int rand_r(unsigned int * seed);

/****************************************************************************/

/*
 * You can request to use the alloca() variant that actually does allocate
 * memory from the system rather than the current stack frame, which will
 * ease stack requirements but may not release allocate memory immediately.
 * In order to do so, add #define __USE_CLIB2_ALLOCA to your program, or
 * -D__USE_CLIB2_ALLOCA to your build makefile. For GCC you will also have
 * to compile your program with the -fno-builtin switch to override the
 * built-in alloca() function.
 */

#if defined(alloca)
#undef alloca
#endif /* alloca */

#if defined(__GNUC__) && !defined(__USE_CLIB2_ALLOCA)
 #define alloca(size) __builtin_alloca(size)
#else
 extern void * alloca(size_t size);

 #ifdef __MEM_DEBUG
  extern void * __alloca(size_t size,const char *file,int line);

  #define alloca(size) __alloca((size),__FILE__,__LINE__)
 #else
  /* This is necessary because some applications key off the fact that
     alloca is a symbol defined by the preprocessor. */
  #define alloca alloca
 #endif /* __MEM_DEBUG */
#endif /* __GNUC__ */

/****************************************************************************/

extern int setenv(const char *name, const char *value, int overwrite);
extern int putenv(const char *string);
extern void unsetenv(const char *name);
extern char * mktemp(char * name_template);
extern int mkstemp(char *name_template);
extern char * mkdtemp(char *name_template);

/****************************************************************************/

/* The following is not part of the ISO 'C' (1994) standard, but it should
   be part of ISO/IEC 9899:1999, also known as "C99". */

/****************************************************************************/

#if defined(__GNUC__) || (defined(__STDC_VERSION__) && (__STDC_VERSION__ >= 199901L))

/****************************************************************************/

extern long long strtoll(const char *str, char **ptr, int base);
extern unsigned long long strtoull(const char *str, char **ptr, int base);
extern long long atoll(const char *str);

/****************************************************************************/

typedef struct { long long quot; long long rem; } lldiv_t;

/****************************************************************************/

extern long long llabs(long long x);
extern lldiv_t lldiv(long long n,long long d);

/****************************************************************************/

#endif /* __GNUC__ || (__STDC_VERSION__ && __STDC_VERSION__ >= 199901L) */

/****************************************************************************/

extern float strtof(const char *str, char ** ptr);

/****************************************************************************/

extern void _Exit(int status);

/****************************************************************************/

#ifdef __cplusplus
}
#endif /* __cplusplus */

/****************************************************************************/

#endif /* _STDLIB_H */
