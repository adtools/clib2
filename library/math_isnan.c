/*
 * $Id: math_isnan.c,v 1.1 2004-08-07 09:15:32 obarthel Exp $
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

#ifndef _STDIO_HEADERS_H
#include "stdio_headers.h"
#endif /* _STDIO_HEADERS_H */

/****************************************************************************/

#if defined (FLOATING_POINT_SUPPORT)

/****************************************************************************/

/* The following is not part of the ISO 'C' (1994) standard. */

/****************************************************************************/

union ieee_long_double
{
	long double		value;
	unsigned long	raw[3];
};

union ieee_double
{
	double			value;
	unsigned long	raw[2];
};

union ieee_single
{
	float			value;
	unsigned long	raw[1];
};

/****************************************************************************/

int
isnan(double number)
{
	int result;

	ENTER();

	/* This assumes that a) 'number' is stored in big endian format
	   and b) it is stored in IEEE 754 format. */
	if (sizeof(number) == 4) /* single precision */
	{
		union ieee_single x;

		x.value = number;

		/* Exponent = 255 and fraction != 0.0 */
		result = ((x.raw[0] & 0x7F800000) == 0x7F800000 && (x.raw[0] & 0x007FFFFF) != 0);
	}
	else if (sizeof(number) == 8) /* double precision */
	{
		union ieee_double x;

		x.value = number;

		/* Exponent = 2047 and fraction != 0.0 */
		result = (((x.raw[0] & 0x7FF00000) == 0x7FF00000) && ((x.raw[0] & 0x000FFFFF) != 0 || (x.raw[1] != 0)));
	}
	else if (sizeof(number) == 12) /* extended precision */
	{
		union ieee_long_double x;

		x.value = number;

		/* Exponent = 32767 and fraction != 0.0 */
		result = (((x.raw[0] & 0x7FFF0000) == 0x7FFF0000) && ((x.raw[1] & 0x7FFFFFFF) != 0 || x.raw[2] != 0));
	}
	else
	{
		/* Can't happen */
		result = 0;
	}

	RETURN(result);
	return(result);
}

/****************************************************************************/

#endif /* FLOATING_POINT_SUPPORT */
