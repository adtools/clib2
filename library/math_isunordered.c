/*
 * $Id: math_isunordered.c,v 1.1 2005-05-12 13:21:43 obarthel Exp $
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
__isunordered_float(float number_x,float number_y)
{
	union ieee_single x;
	union ieee_single y;
	int result;

	x.value = number_x;
	y.value = number_y;

	/* Exponent = 255 and fraction != 0.0 -> not a number */
	if((x.raw[0] & 0x7f800000) == 0x7f800000 && (x.raw[0] & 0x007fffff) != 0)
		result = 1;
	else if((y.raw[0] & 0x7f800000) == 0x7f800000 && (y.raw[0] & 0x007fffff) != 0)
		result = 1;
	else
		result = 0;

	return(result);
}

/****************************************************************************/

int
__isunordered_float_double(float number_x,double number_y)
{
	union ieee_single x;
	union ieee_double y;
	int result;

	x.value = number_x;
	y.value = number_y;

	if((x.raw[0] & 0x7f800000) == 0x7f800000 && (x.raw[0] & 0x007fffff) != 0)
		result = 1; /* Exponent = 255 and fraction != 0.0 -> not a number */
	else if (((y.raw[0] & 0x7ff00000) == 0x7ff00000) && ((y.raw[0] & 0x000fffff) != 0 || (y.raw[1] != 0)))
		result = 1; /* Exponent = 2047 and fraction != 0.0 -> not a number */
	else
		result = 0;

	return(result);
}

/****************************************************************************/

int
__isunordered_double(double number_x,double number_y)
{
	union ieee_double x;
	union ieee_double y;
	int result;

	x.value = number_x;
	y.value = number_y;

	/* Exponent = 2047 and fraction != 0.0 -> not a number */
	if(((x.raw[0] & 0x7ff00000) == 0x7ff00000) && ((x.raw[0] & 0x000fffff) != 0 || (x.raw[1] != 0)))
		result = 1;
	else if (((y.raw[0] & 0x7ff00000) == 0x7ff00000) && ((y.raw[0] & 0x000fffff) != 0 || (y.raw[1] != 0)))
		result = 1;
	else
		result = 0;

	return(result);
}

/****************************************************************************/

#endif /* FLOATING_POINT_SUPPORT */
