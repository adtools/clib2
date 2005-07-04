/*
 * $Id: lib_base.c,v 1.4 2005-07-04 11:14:27 obarthel Exp $
 *
 * :ts=4
 *
 * Amiga shared library skeleton example
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
 */

#include <exec/execbase.h>
#include <exec/resident.h>
#include <exec/libraries.h>
#include <exec/memory.h>

#include <dos.h>

/****************************************************************************/

#define __NOLIBBASE__
#define __NOGLOBALIFACE__
#define __USE_INLINE__

/****************************************************************************/

#include <proto/exec.h>

#if defined(__amigaos4__)
#include <proto/skeleton.h>
#endif /* __amigaos4__ */

/****************************************************************************/

#include "lib_base.h"
#include "macros.h"

/****************************************************************************/

#include "skeleton.library_rev.h"

/****************************************************************************/

LONG _start(VOID);

/****************************************************************************/

STATIC struct SkeletonBase *ASM lib_init(REG(d0,struct SkeletonBase *sb),REG(a0,BPTR segment_list),REG(a6,APTR whatever));
STATIC struct SkeletonBase *ASM lib_open(REG(a6,APTR base));
STATIC BPTR ASM lib_expunge(REG(a6,APTR base));
STATIC BPTR ASM lib_close(REG(a6,APTR base));

/****************************************************************************/

#if defined(__amigaos4__)

STATIC ULONG lib_default_obtain(struct Interface *self);
STATIC ULONG lib_default_release(struct Interface *self);

#else

STATIC LONG lib_reserved(VOID);

#endif /* __amigaos4__ */

/****************************************************************************/

/* This is the first executable location of the library. It must return
   an error to the unsuspecting caller who tries to run it as a program. */
LONG
_start(VOID)
{
	return(-1);
}

/****************************************************************************/

/* This routine is called after the library has been loaded into
   memory and its base data structure has been created. Here we
   must initialize those parts of the base data structure which
   have not been set up yet. As of Kickstart 2.0 the setup code
   in ROM will have initialized the entire Library structure with
   the exception of the lib_Revision field, i.e. everything that
   could be gained from the Resident structure is now in the
   Library structure. Earlier operating system releases would leave
   the entire Library structure uninitialized.

   The library initialization is single threaded, i.e. the operating
   system processes library initialization in a queue. The longer it
   takes for the following routine to set up the base data structure,
   the longer will other Tasks and Processes have to wait to get their
   disk resident libraries opened. This means, the initialization code
   has to be as brief as possible. This particular example code opens
   ROM resident libraries here, which is permitted. You should not
   open disk resident libraries and devices or do anything else that
   could take a long time to complete (in this context "long" is anything
   that takes up a second or longer).

   This routine must return the Library base pointer if it succeeds.
   Failure is indicated by returning NULL instead. */
STATIC struct SkeletonBase * ASM
lib_init(
	REG(d0,struct SkeletonBase *	sb),
	REG(a0,BPTR						segment_list),
	REG(a6,APTR						whatever))
{
	struct SkeletonBase * result = NULL;
	struct Library * SysBase;
	#if defined(__amigaos4__)
	struct ExecIFace * IExec;
	#endif /* __amigaos4__ */

	#if defined(__amigaos4__)
	{
		IExec = (struct ExecIFace *)whatever;

		SysBase = (struct Library *)IExec->Data.LibBase;
	}
	#else
	{
		SysBase = (struct Library *)whatever;
	}
	#endif /* __amigaos4__ */

	/* This library implementation requires Kickstart 2.04 or
	   better to work. */
	if(SysBase->lib_Version < 37)
	{
		sb->sb_UserData = NULL;
		goto out;
	}

	sb->sb_SysBase = SysBase;

	#if defined(__amigaos4__)
	{
		sb->sb_IExec = IExec;
	}
	#endif /* __amigaos4__ */

	InitSemaphore(&sb->sb_Semaphore);

	/* The segment pointer must be stored for later, when this library is
	   unloaded from memory again. */
	sb->sb_SegmentList = segment_list;

	sb->sb_Library.lib_Revision = REVISION;

	/* Allocate the implementation-specific user data structure. */
	sb->sb_UserData = AllocMem(sizeof(*sb->sb_UserData),MEMF_ANY|MEMF_PUBLIC);
	if(sb->sb_UserData == NULL)
		goto out;

	/* Now try the user-supplied library initialization function. */
	if(UserLibInit(SysBase,sb,sb->sb_UserData) == FALSE)
		goto out;

	result = sb;

 out:

	/* Free the library data if the initialization failed. */
	if(result == NULL)
	{
		/* First take care of the user data. */
		if(sb->sb_UserData != NULL)
			FreeMem(sb->sb_UserData,sizeof(*sb->sb_UserData));

		/* Finally, free the library data. This is necessary for a library
		   of type RTF_AUTOINIT (see the "struct Resident" rt_Flags
		   initialization value for the LibTag declaration at the end of
		   this file). */
		#if defined(__amigaos4__)
		{
			DeleteLibrary((struct Library *)sb);
		}
		#else
		{
			FreeMem(((BYTE *)sb) - sb->sb_Library.lib_NegSize,
				(ULONG)(sb->sb_Library.lib_NegSize + sb->sb_Library.lib_PosSize));
		}
		#endif /* __amigaos4__ */
	}

	return(result);
}

/****************************************************************************/

/* This routine is called every time a Task or Process invokes
   OpenLibrary() on this library. Task switching will be disabled
   when control passes through this routine. Every action in this
   routine happens on the context of the calling Task, so unlike
   the library initialization routine one can open disk resident
   libraries, devices, etc. and generally Wait() for something
   to happen. The catch is that every busy operation that takes
   a while to execute will "freeze" the machine until control
   returns from this routine (Task switching is disabled). The Task
   switching is disabled in order to avoid having two Tasks
   execute this code at the same time. In a way, this makes the
   library routine a critical region in the textbook sense.

   This routine must return the Library base pointer if it succeeds.
   Failure is indicated by returning NULL instead. */
STATIC struct SkeletonBase * ASM
lib_open(REG(a6,APTR base))
{
	USE_LIBBASE(base);
	USE_EXEC(sb);

	struct SkeletonBase * result = NULL;

	/* Mark the library as having another customer and
	   clear the delayed expunge flag. This flag is
	   referenced at the time the last customer closes
	   the library. If set, the library will be both
	   closed and removed from memory. We increment the
	   counter here since we will attempt to gain
	   access to a semaphore, which may have the effect
	   of causing the caller to wait. During the time
	   the caller is waiting somebody might trigger
	   the library expunge function which, if the
	   usage counter is zero, would unload the library. */
	sb->sb_Library.lib_OpenCnt++;
	sb->sb_Library.lib_Flags &= ~LIBF_DELEXP;

	/* Now comes the really critical region of the code
	   Only one Task at a time may enter here. */
	ObtainSemaphore(&sb->sb_Semaphore);

	/* Invoke the user-supplied library open function. */
	if(UserLibOpen(sb->sb_UserData) == FALSE)
		goto out;

	/* We have another customer. */
	sb->sb_Library.lib_OpenCnt++;

	result = sb;

 out:

	ReleaseSemaphore(&sb->sb_Semaphore);

	sb->sb_Library.lib_OpenCnt--;

	return(result);
}

/****************************************************************************/

/* This routine will be called when the library needs to be
   removed from memory. If there are no Tasks or Processes
   which have the library open, the routine must remove itself
   from the public list of libraries, free the data that had
   been allocated for it and return the segment list pointer
   it had been given at initialization time so all its code
   can be unloaded. This routine is the last to be called in
   the life cycle of the library. It is called while Task
   switching is disabled. */
STATIC BPTR ASM
lib_expunge(REG(a6,APTR base))
{
	USE_LIBBASE(base);
	USE_EXEC(sb);

	BPTR result = ZERO;

	/* If there are still customers which have the
	   library open, mark the library for delayed
	   expunge. This means, the library will be removed
	   as soon as the last customer closes it. */
	if(sb->sb_Library.lib_OpenCnt > 0)
	{
		sb->sb_Library.lib_Flags |= LIBF_DELEXP;
	}
	else
	{
		/* Invoke the user-supplied expunge function. It must
		   not break a Forbid(), i.e. it must not call Wait()
		   directly or indirectly. */
		UserLibExpunge(sb->sb_UserData);

		/* Free the user data itself. */
		FreeMem(sb->sb_UserData,sizeof(*sb->sb_UserData));

		/* This is the segment list pointer we have to return. */
		result = sb->sb_SegmentList;

		/* Remove the library from the public list. */
		Remove((struct Node *)sb);

		/* Finally, free the data allocated for the
		   base data structure. */
		#if defined(__amigaos4__)
		{
			DeleteLibrary((struct Library *)sb);
		}
		#else
		{
			FreeMem(((BYTE *)sb) - sb->sb_Library.lib_NegSize,
				(ULONG)(sb->sb_Library.lib_NegSize + sb->sb_Library.lib_PosSize));
		}
		#endif /* __amigaos4__ */
	}

	return(result);
}

/****************************************************************************/

/* This routine is called every time CloseLibrary() is used
   on the Library base data structure. It is called while
   Task switching is disabled and has to return the result
   code of the library expunge routine when the time is right. */
STATIC BPTR ASM
lib_close(REG(a6,APTR base))
{
	USE_LIBBASE(base);
	USE_EXEC(sb);

	BPTR result = ZERO;

	/* Now comes the really critical region of the code
	   Only one Task at a time may enter here. */
	ObtainSemaphore(&sb->sb_Semaphore);

	/* Invoke the user-supplied library close function. */
	UserLibClose(sb->sb_UserData);

	ReleaseSemaphore(&sb->sb_Semaphore);

	/* One less customer. */
	sb->sb_Library.lib_OpenCnt--;

	/* Trigger the library expunge function, if desired. */
	if(sb->sb_Library.lib_OpenCnt == 0 && (sb->sb_Library.lib_Flags & LIBF_DELEXP) != 0)
		result = lib_expunge(base);

	return(result);
}

/****************************************************************************/

#if defined(__amigaos4__)

/****************************************************************************/

STATIC ULONG
lib_default_obtain(struct Interface * self)
{
	return(self->Data.RefCount++);
}

STATIC ULONG
lib_default_release(struct Interface * self)
{
	return(self->Data.RefCount--);
}

/****************************************************************************/

STATIC CONST APTR manager_vectors[] =
{
	lib_default_obtain,
	lib_default_release,
	NULL,
	NULL,
	lib_open,
	lib_close,
	lib_expunge,
	NULL,

	(APTR)-1,
};

STATIC CONST struct TagItem manager_tags[] =
{
	{ MIT_Name,			(ULONG)"__library" },
	{ MIT_VectorTable,	(ULONG)manager_vectors },
	{ MIT_Version,		1 },
	{ TAG_DONE,			0 }
};

/****************************************************************************/

STATIC CONST APTR main_vectors[]=
{
	lib_default_obtain,
	lib_default_release,
	NULL,
	NULL,

	(APTR)-1
};

STATIC CONST struct TagItem main_tags[] =
{
	{ MIT_Name,			(ULONG)"main" },
	{ MIT_VectorTable,	(ULONG)main_vectors },
	{ MIT_Version,		1 },
	{ TAG_DONE,			0 }
};

/****************************************************************************/

STATIC CONST APTR lib_interfaces[] =
{
	(CONST APTR)manager_tags,
	(CONST APTR)main_tags,

	NULL
};

/****************************************************************************/

/* This is defined in the file "skeleton_68k.c". */
extern CONST APTR VecTable68K[];

/****************************************************************************/

CONST struct TagItem lib_create_tags[] =
{
	{ CLT_DataSize,		sizeof(struct SkeletonBase) },
	{ CLT_Vector68K,	(ULONG)VecTable68K },
	{ CLT_Interfaces,	(ULONG)lib_interfaces },
	{ CLT_InitFunc,		(ULONG)lib_init },
	{ TAG_DONE,			0 }
};

/****************************************************************************/

CONST struct TagItem local_lib_create_tags[] =
{
	{ CLT_DataSize,		sizeof(struct SkeletonBase) },
	{ CLT_Vector68K,	(ULONG)VecTable68K },
	{ CLT_Interfaces,	(ULONG)lib_interfaces },
	{ TAG_DONE,			0 }
};

/****************************************************************************/

/* This definition and how it is used makes sure that the library
   version information will be visible even with an old 68k "Version"
   command and not just with the OS4 "Version" command. */
STATIC CONST UBYTE version_string[] = "$VER: " VSTRING;

/****************************************************************************/

CONST struct Resident LibTag =
{
	RTC_MATCHWORD,
	(struct Resident *)&LibTag,
	(struct Resident *)&LibTag+1,
	RTF_AUTOINIT|RTF_NATIVE,
	VERSION,
	NT_LIBRARY,
	0,
	"skeleton.library",
	(STRPTR)&version_string[6],
	(struct TagItem *)lib_create_tags
};

/****************************************************************************/

#else

/****************************************************************************/

/* This is the reserved library entry point. It always
   has to return 0. */
STATIC LONG
lib_reserved(VOID)
{
	return(0);
}

/****************************************************************************/

/* The following data structures and data are responsible for
   setting up the Library base data structure and the library
   function vector. */
struct LibraryInitTable
{
	ULONG	lit_BaseSize;		/* Size of the base data structure. */
	APTR *	lit_VectorTable;	/* Points to the function vector. */
	APTR	lit_InitTable;		/* Library base data structure setup table. */
	APTR	lit_InitRoutine;	/* The address of the routine to do the setup. */
};

/****************************************************************************/

/* This is the table of functions that make up the library. The first
   four are mandatory, everything following it are user callable
   routines. The table is terminated by the value -1. */
STATIC CONST APTR lib_vectors[] =
{
	lib_open,
	lib_close,
	lib_expunge,
	lib_reserved,

	(APTR) -1
};

/****************************************************************************/

/* This finally sets up the library base data structure and the
   function vector. */
STATIC CONST struct LibraryInitTable lib_init_table =
{
	sizeof(struct SkeletonBase),
	(APTR *)lib_vectors,
	NULL,
	lib_init
};

/****************************************************************************/

/* The library loader looks for this marker in the memory
   the library code and data will occupy. It is responsible
   setting up the Library base data structure. */
struct Resident LibTag =
{
	RTC_MATCHWORD,					/* Marker value. */
	(struct Resident *)&LibTag,		/* This points back to itself. */
	(struct Resident *)&LibTag+1,	/* This points behind this marker. */
	RTF_AUTOINIT,					/* The Library should be set up according to the given table. */
	VERSION,						/* The version of this Library. */
	NT_LIBRARY,						/* This defines this module as a Library. */
	0,								/* Initialization priority of this Library; unused. */
	"skeleton.library",				/* Points to the name of the Library. */
	VSTRING,						/* The identification string of this Library. */
	(APTR)&lib_init_table			/* This table is for initializing the Library. */
};

/****************************************************************************/

#endif /* __amigaos4__ */
