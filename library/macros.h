/*
 * $Id: macros.h,v 1.7 2005-01-29 18:05:14 obarthel Exp $
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

#ifndef ZERO
#define ZERO ((BPTR)NULL)
#endif /* ZERO */

/****************************************************************************/

/* This is the difference (in seconds) between the Unix epoch (which began
   on January 1st, 1970) and the AmigaOS epoch (which began eight years
   later on January 1st 1978). */
#define UNIX_TIME_OFFSET 252460800

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
 * Uses ANSI token catenation to form a name for the char array
 * based on the variable name, then creates an appropriately
 * typed pointer to point to the first longword boundary in the
 * char array allocated.
 */
#define D_S(type, name) \
	char a_##name[sizeof(type)+3]; \
	type *name = (type *)((LONG)(a_##name+3) & ~3)

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
#endif
#endif /* WEAK */

/****************************************************************************/

#ifndef UNUSED
#ifdef __GNUC__
#define UNUSED __attribute__((unused))
#define NOCOMMON __attribute__((nocommon))
#else
#define UNUSED /* UNUSED */
#define NOCOMMON /* NOCOMMON */
#endif
#endif /* UNUSED */

/****************************************************************************/

#ifdef __SASC
#define CLIB_CONSTRUCTOR(name)		int __stdargs _STI_500_##name(void)
#define CLIB_DESTRUCTOR(name)		void __stdargs _STD_500_##name(void)
#define PROFILE_CONSTRUCTOR(name)	int __stdargs _STI_150_##name(void)
#define PROFILE_DESTRUCTOR(name)	void __stdargs _STD_150_##name(void)
#define CONSTRUCTOR_SUCCEED()		return(0)
#define CONSTRUCTOR_FAIL()			return(1)
#endif /* __SASC */

#ifdef __GNUC__
#define CLIB_CONSTRUCTOR(name)		static void __attribute__((constructor)) name##_ctor(void)
#define CLIB_DESTRUCTOR(name)		static void __attribute__((destructor)) name##_dtor(void)
#define PROFILE_CONSTRUCTOR(name)	static void __attribute__((constructor)) name##_ctor(void)
#define PROFILE_DESTRUCTOR(name)	static void __attribute__((destructor)) name##_dtor(void)
#define CONSTRUCTOR_SUCCEED()		return
#define CONSTRUCTOR_FAIL()			exit(RETURN_FAIL)	/* ZZZ not a nice thing to do; fix the constructor invocation code! */
#endif /* __GNUC__ */

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

#ifdef __GNUC__
#define PROFILE_OFF()	((void)0)
#define PROFILE_ON()	((void)0)
#endif /* __GNUC__ */

/****************************************************************************/

#endif /* _MACROS_H */
