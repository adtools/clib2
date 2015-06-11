/*
 * $Id$
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
 */

#include "profile_gmon.h"
#include <exec/exec.h>
#include <proto/exec.h>
#include <stddef.h>


void __mcount(uint32 frompc, uint32 selfpc);

void
__mcount(uint32 frompc, uint32 selfpc)
{
	uint16 *frompcindex;
	struct tostruct *top, *prevtop;
	struct gmonparam *p;

	int32 toindex;

	p = &_gmonparam;

	if (p->state != kGmonProfOn) return;

	p->state = kGmonProfBusy;

	/*
	 * Check if the PC is inside our text segment.
	 * Really should be...
	 */
	frompc -= p->lowpc;
	selfpc -= p->lowpc;
	if (frompc > p->textsize) goto done;

#if (HASHFRACTION & (HASHFRACTION-1)) == 0
	if (p->hashfraction == HASHFRACTION)
	{
		frompcindex = &p->froms[(size_t)(frompc / (HASHFRACTION *
			sizeof(*p->froms)))];
	}
	else
#endif
	{
		frompcindex = &p->froms[(size_t)(frompc / (p->hashfraction *
			sizeof(*p->froms)))];
	}

	toindex = *frompcindex;

	if (toindex == 0)
	{
		/* first time down this arc */
		toindex = ++p->tos[0].link;
		if (toindex >= p->tolimit)
			/* Ouch! Overflow */
			goto overflow;

		*frompcindex = (uint16)toindex;
		top = &p->tos[toindex];
		top->selfpc = selfpc;
		top->count = 1;
		top->link = 0;
		goto done;
	}

	top = &p->tos[toindex];
	if (top->selfpc == selfpc)
	{
		/* arc at front of chain */
		top->count++;
		goto done;
	}

	for (;;)
	{
		if (top->link == 0)
		{
			toindex = ++p->tos[0].link;
			if (toindex >= p->tolimit)
				goto overflow;

			top = &p->tos[toindex];
			top->selfpc = selfpc;
			top->count = 1;
			top->link = *frompcindex;
			*frompcindex = (uint16)toindex;
			goto done;
		}
		prevtop = top;
		top = &p->tos[top->link];
		if (top->selfpc == selfpc)
		{
			top->count++;
			toindex = prevtop->link;
			prevtop->link = top->link;
			top->link = *frompcindex;
			*frompcindex = (uint16)toindex;
			goto done;
		}
	}

done:
	p->state = kGmonProfOn;
	return;

overflow:
	p->state = kGmonProfError;
	return;
}
