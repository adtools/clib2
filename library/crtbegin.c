/* 
 * $Id: crtbegin.c,v 1.12 2010-08-21 09:57:50 obarthel Exp $
 *
 * :ts=4
 *
 * Handles global constructors and destructors for the OS4 GCC build.
 */

#if defined(__amigaos4__)

/****************************************************************************/

#ifndef _STDLIB_HEADERS_H
#include "stdlib_headers.h"
#endif /* _STDLIB_HEADERS_H */

/****************************************************************************/

#include <libraries/elf.h>

#include <proto/elf.h>
#include <proto/dos.h>

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

void _init(void);
void _fini(void);

/****************************************************************************/

/* These are used to initialize the shared objects linked to this binary,
   and for the dlopen(), dlclose() and dlsym() functions. */
struct Library *	__ElfBase;
struct ElfIFace *	__IElf;

Elf32_Handle		__elf_handle;

/****************************************************************************/

static VOID close_elf_library(void)
{
	if(__IElf != NULL)
	{
		DropInterface((struct Interface *)__IElf);
		__IElf = NULL;
	}
		
	if(__ElfBase != NULL)
	{
		CloseLibrary(__ElfBase);
		__ElfBase = NULL;
	}
}

/****************************************************************************/

static BOOL open_elf_library(void)
{
	BOOL success = FALSE;

	/* We need elf.library V52.2 or higher. */
	__ElfBase = OpenLibrary("elf.library",0);
	if(__ElfBase == NULL || (__ElfBase->lib_Version < 52) || (__ElfBase->lib_Version == 52 && __ElfBase->lib_Revision < 2))
		goto out;

	__IElf = (struct ElfIFace *)GetInterface(__ElfBase,"main",1,NULL);
	if(__IElf == NULL)
		goto out;

	success = TRUE;

 out:

	return(success);
}

/****************************************************************************/

static void shared_obj_exit(void)
{
	/* If we got what we wanted, trigger the destructors,
	   etc. in the shared objects linked to this binary. */
	if(__elf_handle != NULL)
		InitSHLibs(__elf_handle,FALSE);

	close_elf_library();
}

/****************************************************************************/

static void shared_obj_init(void)
{
	struct ElfIFace * IElf;
	BOOL success = FALSE;
	BPTR segment_list;

	if(!open_elf_library())
		goto out;

	/* Try to find the Elf handle associated with this
	   program's segment list. */
	segment_list = GetProcSegList(NULL,GPSLF_CLI | GPSLF_SEG);
	if(segment_list == ZERO)
		goto out;

	if(GetSegListInfoTags(segment_list,
		GSLI_ElfHandle,&__elf_handle,
	TAG_DONE) != 1)
	{
		goto out;
	}

	if(__elf_handle == NULL)
		goto out;

	IElf = __IElf;

	/* Trigger the constructors, etc. in the shared objects
	   linked to this binary. */
	InitSHLibs(__elf_handle,TRUE);

	success = TRUE;

 out:

	if(!success)
		close_elf_library();
}

/****************************************************************************/

void
_init(void)
{
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
