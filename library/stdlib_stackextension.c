/*
 * $Id: stdlib_stackextension.c,v 1.2 2004-09-29 14:17:44 obarthel Exp $
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

#ifndef _STDLIB_HEADERS_H
#include "stdlib_headers.h"
#endif /* _STDLIB_HEADERS_H */

/****************************************************************************/

#if (defined(__GNUC__) && !defined(__PPC__))

/****************************************************************************/

#if defined(STACK_EXTENSION)

/****************************************************************************/

/* internal structure used by the stackextend code */
struct stackframe
{
	struct stackframe *	next;	/* NULL if there is no next one */
	void *				savesp;	/* stored sp for next underlying stackframe */
	void *				upper;	/* end of this stackframe+1 */
};

/****************************************************************************/

/* Local stack extension variables */
static void *				__stk_sp_lower;			/* original entries of task structure */
static void *				__stk_sp_upper;			/* to restore them at exit */
static void *				__stk_initial_sp_lower;	/* original stackborders */
static void *				__stk_initial_sp_upper;
static struct stackframe *	__stk_used;				/* used stackframes */
static struct stackframe *	__stk_spare;			/* spare stackframes */

/****************************************************************************/

#if defined(SMALL_DATA)
#define A4(x) "a4@(" #x ":W)"
#elif defined(SMALL_DATA32)
#define A4(x) "a4@(" #x ":L)"
#else
#define A4(x) #x
#endif /* SMALL_DATA */

/****************************************************************************/

/* Glue code */
asm("

_LVOStackSwap = -0x2dc
StackSwapStruct_SIZEOF = 12

	.globl	_SysBase
	.globl	___stk_limit

|-----------------------------------------------------------------------------

	.globl	___link_a5_0_f

___link_a5_0_f:

	movel	sp@+,a0						| load the return address into A0
	cmpl	"A4(___stk_limit)",sp
	jcc		l0							| if SP >= __stk_limit then we are well within bounds
	jbsr	l2							| try to extend the stack
l0:	link	a5,#0:W						| restore frame pointer
	jmp		a0@							| go to the return address

|-----------------------------------------------------------------------------

	.globl	___sub_0_sp_f

___sub_0_sp_f:
	movel	sp@+,a0						| load the return address into A0
	cmpl	"A4(___stk_limit)",sp
	jcc		l1							| if SP >= __stk_limit then we are well within bounds
	jbsr	l2							| try to extend the stack
l1:	jmp		a0@							| go to the return address

l2:	moveq	#0,d0
	moveq	#0,d1
	jra		___stkext_f					| try stack extension

|-----------------------------------------------------------------------------

	.globl	___link_a5_d0_f

___link_a5_d0_f:
	movel	sp@+,a0						| load the return address into A0
	movel	sp,d1
	subl	d0,d1						| D0 holds size of stack adjustment to make
	cmpl	"A4(___stk_limit)",d1
	jcc		l3							| if (SP-D0) >= __stk_limit then we are well within bounds
	jbsr	l5							| try stack extension
l3:	link	a5,#0:W						| restore the frame pointer
	subl	d0,sp						| adjust the stack pointer, as required
	jmp		a0@							| go to the return address

|-----------------------------------------------------------------------------

	.globl	___sub_d0_sp_f

___sub_d0_sp_f:
	movel	sp@+,a0						| load the return address into A0
	movel	sp,d1
	subl	d0,d1						| D0 holds size of stack adjustment to make
	cmpl	"A4(___stk_limit)",d1
	jcc		l4							| if (SP-D0) >= __stk_limit then we are well within bounds
	jbsr	l5							| try stack extension
l4:	subl	d0,sp						| adjust the stack pointer, as required
	jmp		a0@							| go to the return address

l5:	moveq	#0,d1
	jra		___stkext_f					| try stack extension

|-----------------------------------------------------------------------------

	.globl	___sub_d0_sp

___sub_d0_sp:
	movel	sp@+,a0						| load the return address into A0
	movel	sp,d1
	subl	d0,d1						| D0 holds size of stack adjustment to make
	cmpl	"A4(___stk_limit)",d1
	jcc		l6							| if (SP-D0) >= __stk_limit then we are well within bounds
	jbsr	___stkext					| try stack extension
l6:	subl	d0,sp						| adjust the stack pointer, as required
	jmp		a0@							| go to the return address

|-----------------------------------------------------------------------------

	.globl	___move_d0_sp

___move_d0_sp:
	jra		___stkrst					| Straight jump into ___stkrst

|-----------------------------------------------------------------------------

	.globl	___unlk_a5_rts

___unlk_a5_rts:
	movel	d0,a0						| Preserve D0
	movel	a5,d0						| A5 is stack pointer to be restored
	jbsr	___stkrst
	movel	a0,d0						| Restore D0
	movel	sp@+,a5						| Restore A5, thus performing 'unlink A5'
	rts

|-----------------------------------------------------------------------------

	.globl	___stkext

___stkext:
	moveml	d0/d1/a0/a1/a6,sp@-
	subw	#StackSwapStruct_SIZEOF,sp
	jbsr	_stkext
	tstl	d0
	jeq		s_noext
	movel	"A4(_SysBase)",a6
	movel	sp,a0
	jsr		a6@(_LVOStackSwap)
s_ret:
	moveml	sp@+,d0/d1/a0/a1/a6
	rts
s_noext:
	addw	#StackSwapStruct_SIZEOF,sp
	jra		s_ret

|-----------------------------------------------------------------------------

	.globl	___stkext_f

___stkext_f:
	moveml	d0/d1/a0/a1/a6,sp@-
	subw	#StackSwapStruct_SIZEOF,sp
	jbsr	_stkext_f
	tstl	d0
	jeq		sf_noext
	movel	"A4(_SysBase)",a6
	movel	sp,a0
	jsr		a6@(_LVOStackSwap)
sf_ret:
	moveml	sp@+,d0/d1/a0/a1/a6
  	rts
sf_noext:
	addw	#StackSwapStruct_SIZEOF,sp
	jra		sf_ret

|-----------------------------------------------------------------------------
  
	.globl	___stkrst_f

___stkrst_f:
	moveml	d0/d1/a0/a1/a6,sp@-
	subw	#StackSwapStruct_SIZEOF,sp
	jbsr	_stkrst_f
	movel	"A4(_SysBase)",a6
	movel	sp,a0
	jsr		a6@(_LVOStackSwap)
	moveml	sp@+,d0/d1/a0/a1/a6
  	rts

|-----------------------------------------------------------------------------
  
	.globl	___stkrst

___stkrst:
	moveml	d0/d1/a0/a1/a6,sp@-
	subw	#StackSwapStruct_SIZEOF,sp
	jbsr	_stkrst						| calculate either target sp or StackSwapStruct
	tstl	d0							| set target sp?
	jeq		swpfrm						| jump if not
	movel	d0,a0						| I have a lot of preserved registers and
										| returnadresses on the stack. It's necessary
										| to copy them to the new location
	moveq	#6,d0						| 1 rts, 5 regs and 1 signal mask to copy (1+5+1)-1=6
	lea		sp@(40:W),a1				| get address of uppermost byte+1 (1+5+1)*4+StackSwapStruct_SIZEOF=40
	cmpl	a0,a1						| compare with target location
	jls		lp1							| jump if source<=target
	lea		a0@(-28:W),a0				| else start at lower bound (1+5+1)*4=28
	lea		a1@(-28:W),a1
	movel	a0,sp						| set sp to reserve the room
lp0:
	movel	a1@+,a0@+					| copy with raising addresses
	dbra	d0,lp0						| as long as d0>=0.
	jra		endlp						| ready
lp1:
	movel	a1@-,a0@-					| copy with falling addresses
	dbra	d0,lp1						| as long as d0>=0
	movel	a0,sp						| finally set sp
	jra		endlp						| ready
swpfrm:
	movel	"A4(_SysBase)",a6
	movel	sp,a0
	jsr		a6@(_LVOStackSwap)
endlp:
	moveml	sp@+,d0/d1/a0/a1/a6			| restore registers
	rts									| and return

");

/****************************************************************************/

int
__stk_init(void)
{
	struct Task *task = FindTask(NULL);

	ENTER();

	__stk_initial_sp_lower = __stk_sp_lower = task->tc_SPLower; /* Lower stack bound */
	__stk_initial_sp_upper = __stk_sp_upper = task->tc_SPUpper; /* Upper stack bound +1 */

	SHOWPOINTER(__stk_sp_lower);
	SHOWPOINTER(__stk_sp_upper);

	D(("stack size = %ld",(ULONG)__stk_sp_upper - (ULONG)__stk_sp_lower));

	RETURN(OK);
	return(OK);
}

/****************************************************************************/

/* Free all spare stackframes */
CLIB_DESTRUCTOR(__stk_exit)
{
	ENTER();

	if(__memory_pool == NULL)
	{
		struct stackframe *sf, *sf_next;

		SHOWMSG("we don't have a memory pool; cleaning up the stack frames manually");

		for(sf = __stk_spare ; sf != NULL ; sf = sf_next)
		{
			sf_next = sf->next;

			FreeMem(sf, (char *)sf->upper - (char *)sf);
		}
	}

	__stk_spare = NULL;

	LEAVE();
}

/****************************************************************************/

/* Move a stackframe with a minimum of requiredstack bytes to the used list
   and fill the StackSwapStruct structure. */
static void
pushframe(ULONG requiredstack, struct StackSwapStruct *sss)
{
	struct stackframe *sf;
	ULONG recommendedstack;

	ENTER();

	requiredstack += __stk_safezone + __stk_argbytes;
	if (requiredstack < __stk_minframe)
		requiredstack = __stk_minframe;

	SHOWVALUE(requiredstack);

	recommendedstack = __stk_maxsize - __stk_size;
	if (recommendedstack < requiredstack)
		recommendedstack = requiredstack;

	SHOWVALUE(recommendedstack);

	for (;;)
	{
		sf = __stk_spare; /* get a stackframe from the spares list */
		if (sf == NULL) /* stack overflown */
		{
			for ( ; recommendedstack >= requiredstack ; recommendedstack /= 2)
			{
				D(("allocating %ld bytes for a stack frame",recommendedstack + sizeof(struct stackframe)));

				if(__memory_pool != NULL)
					sf = AllocPooled(__memory_pool,recommendedstack + sizeof(struct stackframe));
				else
					sf = AllocMem(recommendedstack + sizeof(struct stackframe), MEMF_ANY);

				if (sf != NULL)
					break;

				SHOWMSG("that didn't work");
			}

			if (sf == NULL) /* and we have no way to extend it :-| */
			{
				SHOWMSG("bad luck... stack overflow!");
				__stkovf();
			}

			sf->upper = (char *)(sf + 1) + recommendedstack;
			break;
		}

		__stk_spare = sf->next;
		if ((ULONG)((char *)sf->upper - (char *)(sf + 1)) >= recommendedstack)
			break;

		if(__memory_pool != NULL)
			FreePooled(__memory_pool, sf, (char *)sf->upper - (char *)sf);
		else
			FreeMem(sf, (char *)sf->upper - (char *)sf);
	}

	/* Add stackframe to the used list */
	sf->next = __stk_used;
	__stk_used = sf;
	__stk_limit = (char *)(sf + 1) + __stk_safezone + __stk_argbytes;

	/* prepare StackSwapStruct */
	(void *)sss->stk_Pointer = (void *)sf->upper;
	sss->stk_Lower = sf + 1;
	(ULONG)sss->stk_Upper = (ULONG)sf->upper;

	/* Update stack statistics. */
	__stk_size += (char *)sf->upper - (char *)(sf + 1);
	if (__stk_size > __stk_maxsize)
		__stk_maxsize = __stk_size;

	__stk_extensions++;

	SHOWVALUE(__stk_size);
	SHOWVALUE(__stk_maxsize);
	SHOWVALUE(__stk_extensions);

	LEAVE();
}

/****************************************************************************/

#define	STK_UPPER (__stk_used != NULL ? __stk_used->upper : __stk_initial_sp_upper)
#define	STK_LOWER (__stk_used != NULL ? (void *)(__stk_used + 1) : __stk_initial_sp_lower)

/****************************************************************************/

/* Allocate a new stackframe with d0 bytes minimum. */
int
stkext(struct StackSwapStruct sss, long d0, long d1, long a0, long a1, long a6, long ret1)
{
	void *callsp = &ret1 + 1;
	int cpsize = (char *)callsp - (char *)&d0;
	int result;

	ENTER();

	D(("new stack frame with at least %ld bytes space is required", d0));

	if (callsp >= STK_UPPER || callsp < STK_LOWER)
	{
		SHOWMSG("that didn't turn out right");

		result = 0; /* User intentionally left area of stackextension */
	}
	else
	{
		pushframe((ULONG)d0, &sss);
		*(char **)&sss.stk_Pointer -= cpsize;
		CopyMem(&d0, sss.stk_Pointer, cpsize);

		SHOWMSG("done");

		result = 1;
	}

	RETURN (result);
	return (result);
}

/****************************************************************************/

/* Defined in the glue code above. */
extern void __stkrst_f(void);

/****************************************************************************/

/* Allocate a new stackframe with d0 bytes minimum, copy the callers arguments
   and set his returnaddress (offset d1 from the sp when called) to stk_rst_f */
int
stkext_f(struct StackSwapStruct sss, long d0, long d1, long a0, long a1, long a6, long ret1)
{
	void *argtop, *callsp = &ret1 + 1;
	int cpsize;
	int result;

	ENTER();

	D(("new stack frame with at least %ld bytes space is required", d0));

	if (callsp >= STK_UPPER || callsp < STK_LOWER)
	{
		SHOWMSG("that didn't turn out right");

		result = 0; /* User intentionally left area of stackextension */
	}
	else
	{
		argtop = (char *)callsp + __stk_argbytes; /* Top of area with arguments */
		if (argtop > STK_UPPER)
			argtop = STK_UPPER;

		cpsize = (char *)argtop - (char *)&d0;

		pushframe((ULONG)d0, &sss);
		*(char **)&sss.stk_Pointer -= cpsize;
		CopyMem(&d0, sss.stk_Pointer, cpsize);

		__stk_used->savesp = (char *)callsp + d1; /* store sp */
		*(void **)((char *)sss.stk_Upper - ((char *)argtop - (char *)callsp) + d1) = &__stkrst_f; /* set returnaddress */

		SHOWMSG("done");

		result = 1;
	}

	RETURN (result);
	return (result);
}

/****************************************************************************/

/* Move all used stackframes upto (and including) sf to the spares list
   and fill the StackSwapStruct structure. */
static void
popframes(struct stackframe *sf, struct StackSwapStruct *sss)
{
	struct stackframe *sf2;

	ENTER();

	if (sf->next != NULL)
	{
		sss->stk_Lower = sf->next + 1;
		(ULONG)sss->stk_Upper = (ULONG)sf->next->upper;
		__stk_limit = (char *)(sf->next + 1) + __stk_safezone + __stk_argbytes;
	}
	else
	{
		sss->stk_Lower = __stk_sp_lower;
		(ULONG)sss->stk_Upper = (ULONG)__stk_sp_upper;
		__stk_limit = (char *)__stk_initial_sp_lower + __stk_safezone + __stk_argbytes;
	}

	sf2 = __stk_spare;
	__stk_spare = __stk_used;
	__stk_used = sf->next;
	sf->next = sf2;

	/* Update stack statistics. */
	for (sf2 = __stk_spare ; sf2 != sf->next ; sf2 = sf2->next)
		__stk_size -= (char *)sf2->upper - (char *)(sf2 + 1);

	SHOWVALUE(__stk_size);

	LEAVE();
}

/****************************************************************************/

/* Set stackpointer back to some previous value
   != NULL: on the same stackframe (returns sp)
   == NULL: on another stackframe */
void *
stkrst(struct StackSwapStruct sss, void *d0, long d1, long a0, long a1, long a6, long ret1)
{
	void *callsp = &ret1 + 1;
	int cpsize = (char *)callsp - (char *)&d0;
	struct stackframe *sf1, *sf2;
	void * result = d0;

	ENTER();

	if (d0 >= STK_LOWER && d0 < STK_UPPER)
		goto out;

	sf1 = __stk_used;
	if (sf1 == NULL)
		goto out;

	for (;;)
	{
		sf2 = sf1->next;
		if (sf2 == NULL)
		{
			if (d0 < __stk_initial_sp_lower || d0 >= __stk_initial_sp_upper)
				goto out;

			break;
		}

		if (d0 >= (void *)(sf2 + 1) && d0 < sf2->upper) /* This stackframe fits */
			break;

		sf1 = sf2;
	}

	popframes(sf1, &sss);
	sss.stk_Pointer = (char *)d0 - cpsize;
	CopyMem(&d0, sss.stk_Pointer,cpsize);

	result = NULL;

 out:

	RETURN (result);
	return (result);
}

/****************************************************************************/

/* return to last stackframe */
void
stkrst_f(struct StackSwapStruct sss, long d0, long d1, long a0, long a1, long a6)
{
	void *callsp = &a6 + 1; /* This one has no returnaddress - it's a fallback for rts */
	int cpsize = (char *)callsp - (char *)&d0;

	ENTER();

	sss.stk_Pointer = (char *)__stk_used->savesp - cpsize;
	popframes(__stk_used, &sss);
	CopyMem(&d0, sss.stk_Pointer, cpsize);

	LEAVE();
}

/****************************************************************************/

#endif /* STACK_EXTENSION */

/****************************************************************************/

#endif /* __GNUC__ && !__PPC__ */
