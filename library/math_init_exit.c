/*
 * $Id: math_init_exit.c,v 1.1.1.1 2004-07-26 16:30:46 obarthel Exp $
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

#ifndef _MATH_HEADERS_H
#include "math_headers.h"
#endif /* _MATH_HEADERS_H */

/****************************************************************************/

#include "stdlib_protos.h"

/****************************************************************************/

#include <proto/exec.h>

/****************************************************************************/

#include <string.h>

/****************************************************************************/

#if defined(FLOATING_POINT_SUPPORT)

/****************************************************************************/

void
__math_exit(void)
{
	#if defined(IEEE_FLOATING_POINT_SUPPORT)
	{
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
	}
	#endif /* IEEE_FLOATING_POINT_SUPPORT */
}

/****************************************************************************/

int
__math_init(void)
{
	int result = ERROR;

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

			strcpy(message,failed_library);
			strcat(message," could not be opened.");

			__show_error(message);

			goto out;
		}
	}
	#endif /* IEEE_FLOATING_POINT_SUPPORT */

	/* Now fill in the HUGE_VAL constant, which is set to
	 * +Infinity.
	 */
	if(sizeof(__huge_val) == 4) /* single precision */
	{
		static const unsigned long infinity[] =
		{
			0x7F800000
		};

		assert( sizeof(infinity) == sizeof(__huge_val) );

		memmove((void *)&__huge_val,infinity,sizeof(infinity));
	}
	else if (sizeof(__huge_val) == 8) /* double precision */
	{
		static const unsigned long infinity[] =
		{
			0x7FF00000,0x00000000
		};

		assert( sizeof(infinity) == sizeof(__huge_val) );

		memmove((void *)&__huge_val,infinity,sizeof(infinity));
	}
	else if (sizeof(__huge_val) == 12) /* extended precision */
	{
		static const unsigned long infinity[] =
		{
			0x7FFF0000,0x00000000,0x00000000
		};

		assert( sizeof(infinity) == sizeof(__huge_val) );

		memmove((void *)&__huge_val,infinity,sizeof(infinity));
	}

	result = OK;

 out:

	return(result);
}

/****************************************************************************/

#endif /* FLOATING_POINT_SUPPORT */
