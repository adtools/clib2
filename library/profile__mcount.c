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
