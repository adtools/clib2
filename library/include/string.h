/*
 * $Id: string.h,v 1.1.1.1 2004-07-26 16:32:55 obarthel Exp $
 *
 * :ts=4
 *
 * Portable ISO 'C' (1994) runtime library for the Amiga computer
 * Copyright (c) 2002-2004 by Olaf Barthel <olsen@sourcery.han.de>
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
 */

#ifndef _STRING_H
#define _STRING_H

/****************************************************************************/

#ifdef __cplusplus
extern "C" {
#endif /* __cplusplus */

/****************************************************************************/

#ifndef _STDDEF_H
#include <stddef.h>
#endif /* _STDDEF_H */

/****************************************************************************/

extern char *strerror(int error_number);
extern char *strcat(char *dest, const char *src);
extern char *strncat(char *dest, const char *src, size_t n);
extern int strcmp(const char *s1, const char * s2);
extern int strncmp(const char *s1, const char *s2, size_t n);
extern char *strcpy(char *dest, const char *src);
extern char *strncpy(char *dest, const char *src, size_t n);
extern size_t strlen(const char *s);
extern char *strchr(const char *s, int c);
extern char *strrchr(const char *s, int c);
extern size_t strspn(const char *s, const char *set);
extern size_t strcspn(const char *s, const char *set);
extern char *strpbrk(const char *s, const char *set);
extern char *strtok(char *str, const char *set);
extern char *strstr(const char *src, const char *sub);

/****************************************************************************/

extern int strcoll(const char *s1, const char *s2);
extern size_t strxfrm(char *dest, const char *src, size_t len);

/****************************************************************************/

extern void *memchr(const void * ptr, int val, size_t len);
extern int memcmp(const void *ptr1, const void *ptr2, size_t len);
extern void *memcpy(void *dest, const void *src, size_t len);
extern void *memmove(void *dest, const void * src, size_t len);
extern void *memset(void *ptr, int val, size_t len);

/****************************************************************************/

/* The following is not part of the ISO 'C' standard. */

/****************************************************************************/

#ifndef _STRINGS_H
#include <strings.h>
#endif /* _STRINGS_H */

/****************************************************************************/

extern char * index(const char *s, int c);
extern char * rindex(const char *s, int c);

/****************************************************************************/

extern char * strdup(const char *s);

/* This is the version for use with memory debugging; do not call
   it directly! */
extern char * __strdup(const char *s,const char *file,int line);

#ifdef __MEM_DEBUG
#define strdup(s) __strdup((s),__FILE__,__LINE__)
#endif /* __MEM_DEBUG */

/****************************************************************************/

extern void bcopy(const void *from,void *to,size_t len);
extern void bzero(void *m,size_t len);
extern int bcmp(const void *a,const void *b,size_t len);

/****************************************************************************/

#ifdef __cplusplus
}
#endif /* __cplusplus */

/****************************************************************************/

#endif /* _STRING_H */
