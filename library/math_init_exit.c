/*
 * $Id: math_init_exit.c,v 1.13 2005-03-12 14:10:09 obarthel Exp $
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

#ifndef _MATH_HEADERS_H
#include "math_headers.h"
#endif /* _MATH_HEADERS_H */

/****************************************************************************/

#include "stdlib_protos.h"

/****************************************************************************/

#include <exec/execbase.h>

/****************************************************************************/

#include <proto/exec.h>

/****************************************************************************/

#include <string.h>

/****************************************************************************/

#if defined(FLOATING_POINT_SUPPORT)

/****************************************************************************/

#if defined(IEEE_FLOATING_POINT_SUPPORT)

struct Library * MathIeeeSingBasBase;
struct Library * MathIeeeDoubBasBase;
struct Library * MathIeeeDoubTransBase;

#endif /* IEEE_FLOATING_POINT_SUPPORT */

/****************************************************************************/

double __huge_val;

/****************************************************************************/

#if defined(IEEE_FLOATING_POINT_SUPPORT)

/****************************************************************************/

MATH_DESTRUCTOR(math_exit)
{
	ENTER();

	if(MathIeeeSingBasBase != NULL)
	{
		CloseLibrary(MathIeeeSingBasBase);
		MathIeeeSingBasBase = NULL;
	}

	if(MathIeeeDoubBasBase != NULL)
	{
		CloseLibrary(MathIeeeDoubBasBase);
		MathIeeeDoubBasBase = NULL;
	}

	if(MathIeeeDoubTransBase != NULL)
	{
		CloseLibrary(MathIeeeDoubTransBase);
		MathIeeeDoubTransBase = NULL;
	}

	LEAVE();
}

/****************************************************************************/

#endif /* IEEE_FLOATING_POINT_SUPPORT */

/****************************************************************************/

MATH_CONSTRUCTOR(math_init)
{
	BOOL success = FALSE;

	ENTER();

	#if defined(IEEE_FLOATING_POINT_SUPPORT)
	{
		char * failed_library = NULL;

		PROFILE_OFF();

		/* Try to get the soft floating point libraries open. */
		MathIeeeSingBasBase		= OpenLibrary("mathieeesingbas.library",33);
		MathIeeeDoubBasBase		= OpenLibrary("mathieeedoubbas.library",33);
		MathIeeeDoubTransBase	= OpenLibrary("mathieeedoubtrans.library",33);

		PROFILE_ON();

		if(MathIeeeSingBasBase == NULL)
			failed_library = "mathieeesingbas.library";
		else if (MathIeeeDoubBasBase == NULL)
			failed_library = "mathieeedoubbas.library";
		else if (MathIeeeDoubTransBase == NULL)
			failed_library = "mathieeedoubtrans.library";

		if(failed_library != NULL)
		{
			char message[60];

			strlcpy(message,failed_library,sizeof(message));
			strlcat(message," could not be opened.",sizeof(message));

			__show_error(message);

			goto out;
		}
	}
	#endif /* IEEE_FLOATING_POINT_SUPPORT */

	/* Now fill in the HUGE_VAL constant, which is set to
	   the largest representable floating point value. */
	if(sizeof(__huge_val) == 4) /* single precision */
	{
		union ieee_single * x = (union ieee_single *)&__huge_val;

		/* Exponent = +126, Mantissa = 8,388,607 */
		x->raw[0] = 0x7f7fffff;
	}
	else if (sizeof(__huge_val) == 8) /* double precision */
	{
		union ieee_double * x = (union ieee_double *)&__huge_val;

		/* Exponent = +1022, Mantissa = 4,503,599,627,370,495 */
		x->raw[0] = 0x7fefffff;
		x->raw[1] = 0xffffffff;
	}
#if defined(USE_LONG_DOUBLE)
	else if (sizeof(__huge_val) == 12) /* extended precision */
	{
		union ieee_long_double * x = (union ieee_long_double *)&__huge_val;

		/* Exponent = +32766, Mantissa = 18,446,744,073,709,551,615 */
		x->raw[0] = 0x7ffe0000;
		x->raw[1] = 0xffffffff;
		x->raw[2] = 0xffffffff;
	}
#endif /* USE_LONG_DOUBLE */

	success = TRUE;

 out:

	SHOWVALUE(success);
	LEAVE();

	if(success)
		CONSTRUCTOR_SUCCEED();
	else
		CONSTRUCTOR_FAIL();
}

/****************************************************************************/

#endif /* FLOATING_POINT_SUPPORT */
