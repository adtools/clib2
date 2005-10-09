/*
 * $Id: math_trunc.c,v 1.2 2005-10-09 10:38:55 obarthel Exp $
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

double
trunc(double x)
{
  int signbit;
  /* Most significant word, least significant word. */
  int msw;
  unsigned int lsw;
  int exponent_less_1023;

  EXTRACT_WORDS(msw, lsw, x);

  /* Extract sign bit. */
  signbit = msw & 0x80000000;

  /* Extract exponent field. */
  exponent_less_1023 = ((msw & 0x7ff00000) >> 20) - 1023;

  if (exponent_less_1023 < 20)
    {
      /* All significant digits are in msw. */
      if (exponent_less_1023 < 0)
        {
          /* -1 < x < 1, so result is +0 or -0. */
          INSERT_WORDS(x, signbit, 0);
        }
      else
        {
          /* All relevant fraction bits are in msw, so lsw of the result is 0. */
          INSERT_WORDS(x, signbit | (msw & ~(0x000fffff >> exponent_less_1023)), 0);
        }
    }
  else if (exponent_less_1023 > 51)
    {
      if (exponent_less_1023 == 1024)
        {
          /* x is infinite, or not a number, so trigger an exception. */
          return x + x;
        }
      /* All bits in the fraction fields of the msw and lsw are needed in the result. */
    }
  else
    {
      /* All fraction bits in msw are relevant.  Truncate irrelevant
         bits from lsw. */
      INSERT_WORDS(x, msw, lsw & ~(0xffffffffu >> (exponent_less_1023 - 20)));
    }
  return x;
}

/****************************************************************************/

#endif /* FLOATING_POINT_SUPPORT */
