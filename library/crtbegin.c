/* 
 * $Id: crtbegin.c,v 1.5 2005-03-09 21:07:25 obarthel Exp $
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
static void (*__CTOR_LIST__[1]) (void) __attribute__(( used, section(".ctors"), aligned(sizeof(void (*)(void))) ));
static void (*__DTOR_LIST__[1]) (void) __attribute__(( used, section(".dtors"), aligned(sizeof(void (*)(void))) ));

/****************************************************************************/

STATIC VOID
_do_ctors(void)
{
	int num_ctors;
	int i;

	num_ctors = 0;

	for(i = 1 ; __CTOR_LIST__[i] != NULL ; i++)
		num_ctors++;

	for(i = 0 ; i < num_ctors ; i++)
		__CTOR_LIST__[num_ctors - i]();
}

/****************************************************************************/

STATIC VOID
_do_dtors(void)
{
	int num_dtors;
	int i;

	num_dtors = 0;

	for(i = 1 ; __DTOR_LIST__[i] != NULL ; i++)
		num_dtors++;

	for(i = 0 ; i < num_dtors ; i++)
		__DTOR_LIST__[i+1]();
}

/****************************************************************************/

#else

/****************************************************************************/

typedef void (*func_ptr)(void);

/****************************************************************************/

STATIC VOID
_do_ctors(void)
{
	extern func_ptr __CTOR_LIST__[];
	ULONG nptrs = (ULONG)__CTOR_LIST__[0];
	ULONG i;

	for(i = nptrs ; i > 0 ; i--)
		__CTOR_LIST__[i]();
}

/****************************************************************************/

STATIC VOID
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
