/*
 * $Id: macros.h,v 1.18 2005-03-12 14:10:09 obarthel Exp $
 *
 * :ts=4
 *
 * Portable ISO 'C' (1994) runtime library for the Amiga computer
 * Copyright (c) 2002-2005 by Olaf Barthel <olsen@sourcery.han.de>
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

#ifndef _MACROS_H
#define _MACROS_H

/****************************************************************************/

#ifndef DOS_DOS_H
#include <dos/dos.h>
#endif /* DOS_DOS_H */

/****************************************************************************/

#define CANNOT !
#define DO_NOTHING ((void)0)
#define NO !
#define NOT !
#define DO_NOT !
#define OK (0)
#define SAME (0)
#define SEEK_ERROR (-1)
#define ERROR (-1)

/****************************************************************************/

#define NUM_ENTRIES(t) (sizeof(t) / sizeof(t[0]))

/****************************************************************************/

#define FLAG_IS_SET(v,f)	(((v) & (f)) == (f))
#define FLAG_IS_CLEAR(v,f)	(((v) & (f)) ==  0 )

/****************************************************************************/

#define SET_FLAG(v,f)		((void)((v) |=  (f)))
#define CLEAR_FLAG(v,f)		((void)((v) &= ~(f)))

/****************************************************************************/

/* Macro to get longword-aligned stack space for a structure
   Uses ANSI token catenation to form a name for the char array
   based on the variable name, then creates an appropriately
   typed pointer to point to the first longword boundary in the
   char array allocated. */
#define D_S(type, name) \
	char a_##name[sizeof(type)+3]; \
	type *name = (type *)((LONG)(a_##name+3) & ~3)

/****************************************************************************/

/* BCPL style "NULL"; this should be in <dos/dos.h>. */
#ifndef ZERO
#define ZERO ((BPTR)NULL)
#endif /* ZERO */

/****************************************************************************/

/* Constructor and destructor functions, as used by the library for data
   initialization and cleanup. These particular functions are invoked by
   the startup code before and after the main() function is/was called.
   How this works is very compiler specific. We support three flavours
   below. */

/****************************************************************************/

#ifdef __SASC

#define CONSTRUCTOR(name,pri) \
	int __stdargs _STI_##pri##_##name(void); \
	int __stdargs _STI_##pri##_##name(void)

#define DESTRUCTOR(name,pri) \
	void __stdargs _STD_##pri##_##name(void); \
	void __stdargs _STD_##pri##_##name(void)

#define CONSTRUCTOR_SUCCEED() \
	return(0)

#define CONSTRUCTOR_FAIL() \
	return(1)

#endif /* __SASC */

/****************************************************************************/

#ifdef __GNUC__

#if defined(__amigaos4__)

#define CONSTRUCTOR(name,pri) \
	STATIC VOID __attribute__((used)) name##_ctor(VOID); \
	STATIC VOID (*__##name##_ctor)(VOID) __attribute__((used,section(".ctors._" #pri))) = name##_ctor; \
	STATIC VOID name##_ctor(VOID)

#define DESTRUCTOR(name,pri) \
	STATIC VOID __attribute__((used)) name##_dtor(VOID); \
	STATIC VOID (*__##name##_dtor)(VOID) __attribute__((used,section(".dtors._" #pri))) = name##_dtor; \
	STATIC VOID name##_dtor(VOID)

#else

#define CONSTRUCTOR(name,pri) \
	asm(".stabs \"___INIT_LIST__\",22,0,0,___ctor_" #name); \
	asm(".stabs \"___INIT_LIST__\",20,0,0," #pri); \
	VOID __ctor_##name##(VOID); \
	VOID __ctor_##name##(VOID)

#define DESTRUCTOR(name,pri) \
	asm(".stabs \"___EXIT_LIST__\",22,0,0,___dtor_" #name); \
	asm(".stabs \"___EXIT_LIST__\",20,0,0," #pri); \
	VOID __dtor_##name##(VOID); \
	VOID __dtor_##name##(VOID)

#endif /* __amigaos4__ */

#define CONSTRUCTOR_SUCCEED() \
	return

#define CONSTRUCTOR_FAIL() \
	exit(RETURN_FAIL)

#endif /* __GNUC__ */

/****************************************************************************/

/* These macros are for declaring functions to serve as constructors or
   destructors. In which order these should be invoked is defined by the
   priority, which is a number in the range 0-999. User-supplied
   constructor/destructor functions should have priority 0. That way,
   the user-supplied constructors will be invoked after the library
   constructors and the user-supplied destructors before the library
   destructors. */

#define STDLIB_CONSTRUCTOR(name)	CONSTRUCTOR(name,	1)
#define STDLIB_DESTRUCTOR(name)		DESTRUCTOR(name,	1)

#define STK_CONSTRUCTOR(name)		CONSTRUCTOR(name,	2)
#define STK_DESTRUCTOR(name)		DESTRUCTOR(name,	2)

#define STDIO_CONSTRUCTOR(name)		CONSTRUCTOR(name,	3)
#define STDIO_DESTRUCTOR(name)		DESTRUCTOR(name,	3)

#define FILE_CONSTRUCTOR(name)		CONSTRUCTOR(name,	4)
#define FILE_DESTRUCTOR(name)		DESTRUCTOR(name,	4)

#define MATH_CONSTRUCTOR(name)		CONSTRUCTOR(name,	5)
#define MATH_DESTRUCTOR(name)		DESTRUCTOR(name,	5)

#define SOCKET_CONSTRUCTOR(name)	CONSTRUCTOR(name,	6)
#define SOCKET_DESTRUCTOR(name)		DESTRUCTOR(name,	6)

#define ARG_CONSTRUCTOR(name)		CONSTRUCTOR(name,	7)
#define ARG_DESTRUCTOR(name)		DESTRUCTOR(name,	7)

#define CLIB_CONSTRUCTOR(name)		CONSTRUCTOR(name,	8)
#define CLIB_DESTRUCTOR(name)		DESTRUCTOR(name,	8)

#define PROFILE_CONSTRUCTOR(name)	CONSTRUCTOR(name,	9)
#define PROFILE_DESTRUCTOR(name)	DESTRUCTOR(name,	9)

/****************************************************************************/

/* Magic macros for code profiling, SAS/C style. Normally, you would find
   these in <sprof.h>, which is SAS/C-specific. */

/****************************************************************************/

#ifdef __SASC
extern void ASM _PROLOG(REG(a0,char *));
extern void ASM _EPILOG(REG(a0,char *));

#if _PROFILE
#define PROFILE_OFF()	_PROLOG(0L)
#define PROFILE_ON()	_EPILOG(0L)
#else
#define PROFILE_OFF()	((void)0)
#define PROFILE_ON()	((void)0)
#endif /* _PROFILE */
#endif /* __SASC */

/****************************************************************************/

#ifdef __GNUC__
#define PROFILE_OFF()	((void)0)
#define PROFILE_ON()	((void)0)
#endif /* __GNUC__ */

/****************************************************************************/

/* Special data and function attributes; for OS4 most, if not all of them
   are in a file called <amiga_compiler.h> which is pulled in by the
   <exec/types.h> header file. */

/****************************************************************************/

#ifndef AMIGA_COMPILER_H

#ifdef __SASC

#ifndef ASM
#define ASM __asm
#endif /* ASM */

#ifndef REG
#define REG(r,p) register __##r p
#endif /* REG */

#ifndef INTERRUPT
#define INTERRUPT __interrupt
#endif /* INTERRUPT */

#ifndef INLINE
#define INLINE __inline
#endif /* INLINE */
#endif /* __SASC */

#ifdef __GNUC__

#ifndef ASM
#define ASM
#endif /* ASM */

#ifndef REG
#define REG(r,p) p __asm(#r)
#endif /* REG */

#ifndef INTERRUPT
#define INTERRUPT __attribute__((__interrupt__))
#endif /* INTERRUPT */

#ifndef INLINE
#define INLINE __inline__
#endif /* INLINE */

#endif /* __GNUC__ */

#endif /* AMIGA_COMPILER_H */

/****************************************************************************/

#ifndef WEAK
#ifdef __GNUC__
#define WEAK __attribute__((weak))
#else
#define WEAK /* WEAK */
#endif /* __GNUC__ */
#endif /* WEAK */

/****************************************************************************/

#ifndef UNUSED
#ifdef __GNUC__
#define UNUSED __attribute__((unused))
#else
#define UNUSED /* UNUSED */
#endif /* __GNUC__ */
#endif /* UNUSED */

/****************************************************************************/

#ifndef NOCOMMON
#ifdef __GNUC__
#define NOCOMMON __attribute__((nocommon))
#else
#define NOCOMMON /* NOCOMMON */
#endif /* __GNUC__ */
#endif /* NOCOMMON */

/****************************************************************************/

#endif /* _MACROS_H */
