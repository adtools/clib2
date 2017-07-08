/*
 * crtbegin.c
 *
 * :ts=4
 *
 * Handles global constructors and destructors for the OS4 GCC build.
 *
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
 * code can find the global constructor/destructor pointers.
 *
 * WARNING:
 * This hack does not work correctly with GCC 5 and higher. The optimizer
 * will see a one element array and act appropriately. The current workaround
 * is to use -fno-aggressive-loop-optimizations when compiling this file.
 */
static void (*__CTOR_LIST__[1]) (void) __attribute__(( used, section(".ctors"), aligned(sizeof(void (*)(void))) ));
static void (*__DTOR_LIST__[1]) (void) __attribute__(( used, section(".dtors"), aligned(sizeof(void (*)(void))) ));

/****************************************************************************/

void _init(void);
void _fini(void);

/****************************************************************************/

void
_init(void)
{
	extern void shared_obj_init(void);
	int num_ctors,i;
	int j;

	/* The shared objects need to be set up before any local
	   constructors are invoked. */
	shared_obj_init();

	for(i = 1, num_ctors = 0 ; __CTOR_LIST__[i] != NULL ; i++)
		num_ctors++;

	for(j = 0 ; j < num_ctors ; j++)
		__CTOR_LIST__[num_ctors - j]();
}

/****************************************************************************/

void
_fini(void)
{
	extern void shared_obj_exit(void);
	int num_dtors,i;
	static int j;

	for(i = 1, num_dtors = 0 ; __DTOR_LIST__[i] != NULL ; i++)
		num_dtors++;

	while(j++ < num_dtors)
		__DTOR_LIST__[j]();

	/* The shared objects need to be cleaned up after all local
	   destructors have been invoked. */
	shared_obj_exit();
}

/****************************************************************************/

#endif /*__amigaos4__ */
