/*
 * $Id: math_lrintf.c,v 1.3 2006-01-08 12:04:23 obarthel Exp $
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

/* Adding a float, x, to 2^23 will cause the result to be rounded based on
   the fractional part of x, according to the implementation's current rounding
   mode.  2^23 is the smallest float that can be represented using all 23 significant
   digits. */
static const float TWO23[2]={
  8.3886080000e+06, /* 0x4b000000 */
 -8.3886080000e+06, /* 0xcb000000 */
};

/****************************************************************************/

long int
lrintf(float x)
{
  LONG j0,sx;
  ULONG i0;
  float t;
  volatile float w;
  long int result;

  GET_FLOAT_WORD(i0,x);

  /* Extract sign bit. */
  sx = (i0 >> 31);

  /* Extract exponent field. */
  j0 = ((i0 & 0x7f800000) >> 23) - 127;
  
  if (j0 < (int)(sizeof (long int) * 8) - 1)
    {
      if (j0 < -1)
        return 0;
      else if (j0 >= 23)
        result = (long int) ((i0 & 0x7fffff) | 0x800000) << (j0 - 23);
      else
        {
          w = TWO23[sx] + x;
          t = w - TWO23[sx];
          GET_FLOAT_WORD (i0, t);
          /* Detect the all-zeros representation of plus and
             minus zero, which fails the calculation below. */
          if ((i0 & ~(1 << 31)) == 0)
              return 0;
          j0 = ((i0 >> 23) & 0xff) - 0x7f;
          i0 &= 0x7fffff;
          i0 |= 0x800000;
          result = i0 >> (23 - j0);
        }
    }
  else
    {
      return (long int) x;
    }
  return sx ? -result : result;
}

/****************************************************************************/

#endif /* FLOATING_POINT_SUPPORT */
