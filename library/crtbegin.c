/* 
 * $Id: crtbegin.c,v 1.6 2005-03-10 09:55:03 obarthel Exp $
 *
 * :ts=4
 *
 * Handles global constructors and destructors for the OS4 GCC build.
 */

#if defined(__amigaos4__)

/****************************************************************************/

#include <exec/types.h>

/****************************************************************************/

#include <stdlib.h>
#include <setjmp.h>

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

void
_init(void)
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

void
_fini(void)
{
	extern jmp_buf __exit_jmp_buf;

	int num_dtors,j;
	static int i;

	/* If one of the destructors drops into
	   exit(), processing will continue with
	   the next following destructor. */
	(void)setjmp(__exit_jmp_buf);

	for(i = j, num_dtors = 0 ; __DTOR_LIST__[j] != NULL ; j++)
		num_dtors++;

	while(i++ < num_dtors)
		__DTOR_LIST__[i]();
}

/****************************************************************************/

#endif /*__amigaos4__ */
