/*
 * $Id: amiga_beginio.c,v 1.3 2005-01-02 09:07:06 obarthel Exp $
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

#include <exec/devices.h>
#include <exec/io.h>
#include <clib/alib_protos.h>

/****************************************************************************/

#include "debug.h"

/****************************************************************************/

#ifndef __PPC__

/****************************************************************************/

#if defined(__SASC)

VOID __begin_io(struct IORequest * ior);
#pragma libcall DeviceBase __begin_io 01e 901

#endif /* __SASC */

/****************************************************************************/

#if defined(__GNUC__)

#define __begin_io(ior) ({ \
  struct IORequest * _BeginIO_ior = (ior); \
  { \
  register struct Library * const __BeginIO__bn __asm("a6") = (struct Library *) (DeviceBase);\
  register struct IORequest * __BeginIO_ior __asm("a1") = (_BeginIO_ior); \
  __asm volatile ("jsr a6@(-30:W)" \
  : \
  : "r"(__BeginIO__bn), "r"(__BeginIO_ior)  \
  : "d0", "d1", "a0", "a1", "fp0", "fp1", "cc", "memory"); \
  } \
})

#endif /* __GNUC__ */

/****************************************************************************/

VOID
BeginIO(struct IORequest *ior)
{
	struct Device * DeviceBase;

	ENTER();

	assert( ior != NULL && ior->io_Device != NULL );

	DeviceBase = ior->io_Device;
	__begin_io(ior);

	LEAVE();
}

/****************************************************************************/

#else

/****************************************************************************/

#include <proto/exec.h>

/****************************************************************************/

#if defined(BeginIO)
#undef BeginIO
#endif /* BeginIO */

/****************************************************************************/

VOID
BeginIO(struct IORequest *ior)
{
	ENTER();

	assert( ior != NULL && ior->io_Device != NULL );

	IExec->BeginIO(ior);

	LEAVE();
}

/****************************************************************************/

#endif /* __PPC__ */
