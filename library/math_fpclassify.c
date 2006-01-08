/*
 * $Id: math_fpclassify.c,v 1.4 2006-01-08 12:04:23 obarthel Exp $
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
 */

#ifndef _STDIO_HEADERS_H
#include "stdio_headers.h"
#endif /* _STDIO_HEADERS_H */

/****************************************************************************/

#if defined (FLOATING_POINT_SUPPORT)

/****************************************************************************/

/* The following is not part of the ISO 'C' (1994) standard, but it should
   be part of ISO/IEC 9899:1999, also known as "C99". */

/****************************************************************************/

int
__fpclassify_float(float number)
{
	union ieee_single x;
	int result;

	x.value = number;

	D(("number = 0x%08lx",x.raw[0]));

	if((x.raw[0] & 0x7f800000) == 0x7f800000 && (x.raw[0] & 0x007fffff) != 0)
	{
		SHOWMSG("not a number");

		/* Exponent = 255 and fraction != 0.0 -> not a number */
		result = FP_NAN;
	}
	else if ((x.raw[0] & 0x7fffffff) == 0x7f800000)
	{
		SHOWMSG("infinity");

		/* Exponent = 255 and fraction = 0.0 -> infinity */
		result = FP_INFINITE;
	}
	else if ((x.raw[0] & 0x7fffffff) == 0)
	{
		SHOWMSG("zero");

		/* Both exponent and fraction are zero -> zero */
		result = FP_ZERO;
	}
	else if ((x.raw[0] & 0x7f800000) == 0)
	{
		SHOWMSG("subnormal");

		/* Exponent = 0 -> subnormal (IEEE 754) */
		result = FP_SUBNORMAL;
	}
	else
	{
		SHOWMSG("normal");

		result = FP_NORMAL;
	}

	SHOWVALUE(result);

	return(result);
}

/****************************************************************************/

int
__fpclassify_double(double number)
{
	union ieee_double x;
	int result;

	x.value = number;

	D(("number = 0x%08lx%08lx",x.raw[0],x.raw[1]));

	if(((x.raw[0] & 0x7ff00000) == 0x7ff00000) && ((x.raw[0] & 0x000fffff) != 0 || (x.raw[1] != 0)))
	{
		SHOWMSG("not a number");

		/* Exponent = 2047 and fraction != 0.0 -> not a number */
		result = FP_NAN;
	}
	else if (((x.raw[0] & 0x7fffffff) == 0x7ff00000) && (x.raw[1] == 0))
	{
		SHOWMSG("infinity");

		/* Exponent = 2047 and fraction = 0.0 -> infinity */
		result = FP_INFINITE;
	}
	else if ((((x.raw[0] & 0x7fffffff) == 0) && (x.raw[1] == 0)))
	{
		SHOWMSG("zero");

		/* Both exponent and fraction are zero -> zero */
		result = FP_ZERO;
	}
	else if ((x.raw[0] & 0x7fff0000) == 0)
	{
		SHOWMSG("subnormal");

		/* Exponent = 0 -> subnormal (IEEE 754) */
		result = FP_SUBNORMAL;
	}
	else
	{
		SHOWMSG("normal");

		result = FP_NORMAL;
	}

	SHOWVALUE(result);

	return(result);
}

/****************************************************************************/

#endif /* FLOATING_POINT_SUPPORT */
