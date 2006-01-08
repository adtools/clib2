/*
 * $Id: macros.h,v 1.2 2006-01-08 12:06:14 obarthel Exp $
 *
 * :ts=4
 *
 * Amiga shared library skeleton example
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
 */

#ifndef _MACROS_H
#define _MACROS_H

/****************************************************************************/

#ifndef EXEC_TYPES_H
#include <exec/types.h>
#endif /* EXEC_TYPES_H */

#ifndef DOS_DOS_H
#include <dos/dos.h>
#endif /* DOS_DOS_H */

/****************************************************************************/

/* We use a few macros to ease the declaration of function parameters
   which are normally part of the AmigaOS4 header files. They are
   automatically included by the <exec/types.h> header file. However,
   if you are going to build this code on the plain 68k AmigaOS
   platform, you will need to define these macros locally. */

/****************************************************************************/

#ifndef AMIGA_COMPILER_H

/****************************************************************************/

#ifdef __SASC

#ifndef ASM
#define ASM __asm
#endif /* ASM */

#ifndef REG
#define REG(reg,arg) register __##reg arg
#endif /* REG */

#define APICALL

#endif /* __SASC */

/****************************************************************************/

#ifdef __GNUC__

#ifndef ASM
#define ASM
#endif /* ASM */

#ifndef REG
#define REG(reg,arg) arg __asm(#reg)
#endif /* REG */

#define APICALL

#define VARARGS68K

#endif /* __GNUC__ */

/****************************************************************************/

#endif /* AMIGA_COMPILER_H */

/****************************************************************************/

#ifndef ZERO
#define ZERO ((BPTR)NULL)
#endif /* ZERO */

/****************************************************************************/

#endif /* _MACROS_H */
