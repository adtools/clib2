/* 
 * $Id: crtbegin.c,v 1.10 2005-03-18 12:38:22 obarthel Exp $
 *
 * :ts=4
 *
 * Handles global constructors and destructors for the OS4 GCC build.
 */

#if defined(__amigaos4__)

/****************************************************************************/

#ifndef EXEC_TYPES_H
#include <exec/types.h>
#endif /* EXEC_TYPES_H */

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
	int num_ctors,i;
	int j;

	for(i = 1, num_ctors = 0 ; __CTOR_LIST__[i] != NULL ; i++)
		num_ctors++;

	for(j = 0 ; j < num_ctors ; j++)
		__CTOR_LIST__[num_ctors - j]();
}

/****************************************************************************/

void
_fini(void)
{
	int num_dtors,i;
	static int j;

	for(i = 1, num_dtors = 0 ; __DTOR_LIST__[i] != NULL ; i++)
		num_dtors++;

	while(j++ < num_dtors)
		__DTOR_LIST__[j]();
}

/****************************************************************************/

#endif /*__amigaos4__ */
