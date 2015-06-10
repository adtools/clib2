/*
 * $Id: math_ceil.c,v 1.6 2006-01-08 12:04:23 obarthel Exp $
 *
 * :ts=4
 *
 * Portable ISO 'C' (1994) runtime library for the Amiga computer
 * Copyright (c) 2002-2015 by Olaf Barthel <obarthel (at) gmx.net>
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
 *
 *
 * PowerPC math library based in part on work by Sun Microsystems
 * Copyright (C) 1993 by Sun Microsystems, Inc. All rights reserved.
 *
 * Developed at SunPro, a Sun Microsystems, Inc. business.
 * Permission to use, copy, modify, and distribute this
 * software is freely granted, provided that this notice
 * is preserved.
 */

#ifndef _MATH_HEADERS_H
#include "math_headers.h"
#endif /* _MATH_HEADERS_H */

/****************************************************************************/

#if defined(FLOATING_POINT_SUPPORT)

/****************************************************************************/

#if defined(IEEE_FLOATING_POINT_SUPPORT)

/****************************************************************************/

#if defined(__GNUC__)

/****************************************************************************/

#if defined(SMALL_DATA)
#define A4(x) "a4@(" #x ":W)"
#elif defined(SMALL_DATA32)
#define A4(x) "a4@(" #x ":L)"
#else
#define A4(x) #x
#endif /* SMALL_DATA */

/****************************************************************************/

extern double __ceil(double x);

/****************************************************************************/

asm("

	.text
	.even

	.globl	_MathIeeeDoubBasBase
	.globl	___ceil

___ceil:

	movel	a6,sp@-
	movel	"A4(_MathIeeeDoubBasBase)",a6
	moveml	sp@(8),d0/d1
	jsr		a6@(-96:W)
	movel	sp@+,a6
	rts

");

/****************************************************************************/

#else

/****************************************************************************/

INLINE STATIC const double
__ceil(double x)
{
	double result;

	result = IEEEDPCeil(x);

	return(result);
}

/****************************************************************************/

#endif /* __GNUC__ */

/****************************************************************************/

#endif /* IEEE_FLOATING_POINT_SUPPORT */

/****************************************************************************/

#if defined(M68881_FLOATING_POINT_SUPPORT)

INLINE STATIC const double
__ceil(double x)
{
	int rounding_mode, round_up;
	double result;

	__asm __volatile ("fmove%.l fpcr,%0"
	                  : "=dm" (rounding_mode)
	                  : /* no inputs */ );

	round_up = rounding_mode | 0x30;

	__asm __volatile ("fmove%.l %0,fpcr"
	                  : /* no outputs */
	                  : "dmi" (round_up));
	__asm __volatile ("fint%.x %1,%0"
	                  : "=f" (result)
	                  : "f" (x));
	__asm __volatile ("fmove%.l %0,fpcr"
	                  : /* no outputs */
	                  : "dmi" (rounding_mode));

	return(result);
}

#endif /* M68881_FLOATING_POINT_SUPPORT */

/****************************************************************************/

#if defined(PPC_FLOATING_POINT_SUPPORT)

static const double huge = 1.0e300;

INLINE STATIC double
__ceil(double x)
{
	int i0,i1,j0;
	unsigned int i,j;

	EXTRACT_WORDS(i0,i1,x);

	j0 = ((i0>>20)&0x7ff)-0x3ff;

	if(j0<20) 
	{
	    if(j0<0) 
		{
			if(huge+x>0.0) 
			{
				if(i0<0) 
				{
					i0=0x80000000;
					i1=0;
				} 
				else if((i0|i1)!=0) 
				{ 
					i0=0x3ff00000;
					i1=0;
				}
			}
	    } 
		else 
		{
			i = (0x000fffff)>>j0;
			if(((i0&i)|i1)==0) 
				return x;
			if(huge+x>0.0) 
			{	
				if(i0>0) 
					i0 += (0x00100000)>>j0;
				i0 &= (~i); i1=0;
			}
	    }
	} 
	else if (j0>51) 
	{
	    if(j0==0x400) 
			return x+x;
	    else 
			return x;
	}
	else 
	{
	    i = ((unsigned int)(0xffffffff))>>(j0-20);

	    if((i1&i)==0) 
			return x;

	    if(huge+x>0.0)
		{
			
			if(i0>0)
			{
				if(j0==20)
					i0+=1; 
				else 
				{
					j = i1 + (1<<(52-j0));
					if(j<i1) 
						i0+=1;	
					i1 = j;
				}
			}
			i1 &= (~i);
	    }
	}

	INSERT_WORDS(x,i0,i1);

	return x;
}
	
#endif /* PPC_FLOATING_POINT_SUPPORT */

/****************************************************************************/

double
ceil(double x)
{
	double result;

	result = __ceil(x);

	return(result);
}

/****************************************************************************/

#endif /* FLOATING_POINT_SUPPORT */
