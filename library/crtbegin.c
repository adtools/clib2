/* 
 * $Id: crtbegin.c,v 1.2 2004-09-29 14:31:56 obarthel Exp $
 *
 * :ts=4
 *
 * Handles global constructors and destructors.
 */

#include <exec/types.h>

/****************************************************************************/

#include <stdlib.h>
#include <setjmp.h>

/****************************************************************************/

#if defined(__amigaos4__)

/****************************************************************************/

/*
 * Dummy constructor and destructor array. The linker script will put these at the
 * very beginning of section ".ctors" and ".dtors". crtend.o contains a similar entry
 * with a NULL pointer entry and is put at the end of the sections. This way, the init
 * code can find the global constructor/destructor pointers
 */
static void (*__CTOR_LIST__[1]) (void) __attribute__((section(".ctors"))) = { (void *)-1 };
static void (*__DTOR_LIST__[1]) (void) __attribute__((section(".dtors"))) = { (void *)-1 };

/****************************************************************************/

static void
_do_ctors(void)
{
	void (**pFuncPtr)(void);

	/* Skip the first entry in the list (it's -1 anyway) */
	pFuncPtr = __CTOR_LIST__ + 1;

	/* Call all constructors in forward order */
	while (*pFuncPtr != NULL)
		(**pFuncPtr++)();
}

/****************************************************************************/

static void
_do_dtors(void)
{
	unsigned long i = (unsigned long)__DTOR_LIST__[0];
	void (**pFuncPtr)(void);

	if (i == ~0UL)
	{
		/* Find the end of the destructors list */
		i = 1;

		while (__DTOR_LIST__[i] != NULL)
			i++;

		/* We're at the NULL entry now. Go back by one */
		i--;
	}

	/* Call all destructors in reverse order */
	pFuncPtr = __DTOR_LIST__ + i;
	while (i-- > 0)
		(**pFuncPtr--)();
}

/****************************************************************************/

#else

/****************************************************************************/

typedef void (*func_ptr)(void);

/****************************************************************************/

static void
_do_ctors(void)
{
	extern func_ptr __CTOR_LIST__[];
	ULONG nptrs = (ULONG)__CTOR_LIST__[0];
	ULONG i;

	for(i = nptrs ; i > 0 ; i--)
		__CTOR_LIST__[i]();
}

/****************************************************************************/

static void
_do_dtors(void)
{
	extern func_ptr __DTOR_LIST__[];
	extern jmp_buf __exit_jmp_buf;
	ULONG nptrs = (ULONG)__DTOR_LIST__[0];
	static ULONG i;

	/* If one of the destructors drops into
	 * exit(), processing will continue with
	 * the next following destructor.
	 */
	(void)setjmp(__exit_jmp_buf);

	while(i++ < nptrs)
		__DTOR_LIST__[i]();
}

/****************************************************************************/

#endif /*__amigaos4__ */

/****************************************************************************/

/* FIXME: Do we need to put these in .init/.fini sections? */

//void _init(void) __attribute__((section(".init")));
//void _fini(void) __attribute__((section(".fini")));

/****************************************************************************/

void
_init(void)
{
	_do_ctors();
}

/****************************************************************************/

void
_fini(void)
{
	_do_dtors();
}
