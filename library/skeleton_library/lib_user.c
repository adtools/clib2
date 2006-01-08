/*
 * $Id: lib_user.c,v 1.6 2006-01-08 12:06:14 obarthel Exp $
 *
 * :ts=4
 *
 * Amiga shared library skeleton example
 * Copyright (c) 2002-2006 by Olaf Barthel <olsen (at) sourcery.han.de>
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

#include "lib_user.h"

/****************************************************************************/

#include <dos.h>

/****************************************************************************/

/* The following function is called as part of the library initialization,
   right after the library is loaded. This function should perform only
   initialization operations which can be accomplished quickly and without
   delay because only one library initialization function at a time can
   be run by the operating system. This means that until this function has
   has returned, no other program or operating system component will be able
   to open a library.

   By the time this function is invoked the library base has already been
   initialized. It has to return TRUE for success and FALSE otherwise. */
BOOL
UserLibInit(
	struct Library *		SysBase,
	struct SkeletonBase *	sb,
	struct UserData *		ud)
{
	BOOL result;

	/* We reset the use count of the user data to 0, so that we can
	   keep track of how many users are actually using it. Note that
	   the contents of the UserData structure are completely random
	   when this function is called. */
	ud->ud_UseCount = 0;

	/* Also remember the SysBase pointer. */
	ud->ud_SysBase = SysBase;

	#if defined(__amigaos4__)
	{
		ud->ud_IExec = (struct ExecIFace *)((struct ExecBase *)SysBase)->MainInterface;
	}
	#endif /* __amigaos4__ */

	/* The library base pointer may come in handy if library
	   functions are to call other library functions. These
	   calls should go through the normal library calling
	   mechanism rather than invoke these functions directly. */
	ud->ud_SkeletonBase = sb;

	result = TRUE;

	return(result);
}

/****************************************************************************/

/* The following function is called whenever the library is opened. Since
   by the time is called the library may have had another opener, it cannot
   rely upon the sb->sb_Library.lib_OpenCnt value to be current, it must
   find other means to keep track of whether it was invoked for the first
   time or not. In this function initializations such as resource allocations
   are performed. Unlike the UserLibInit() function, these initializations
   can take their time to complete since these will execute on the
   schedule of the caller. Take care: some callers may be plain Tasks which
   cannot call all dos.library functions! This function has to return TRUE for
   success and FALSE otherwise. */
BOOL
UserLibOpen(struct UserData * ud)
{
	BOOL result = FALSE;

	/* For the AmigaOS4 build, invoke the clib2 shared library
	   initialization code. Note that this is not strictly
	   necessary. In fact, you should not need this functionality
	   if you stick to use Amiga operating system routines only
	   and stay away from using 'C' runtime library functions
	   that are not reentrant, such as malloc() or fprintf().
	   Use this feature only if you are porting code to the Amiga
	   which cannot be easily converted to follow the AmigaOS
	   API definitions only. */
	#if defined(__amigaos4__) && defined(__THREAD_SAFE)
	{
		/* Note that the clib2 library initialization is
		   called exactly once, when the first client
		   opens this library. */
		if(ud->ud_UseCount == 0 && !__lib_init(SysBase))
			goto out;
	}
	#endif /* __amigaos4__ && __THREAD_SAFE */

	/* Remember that one more customer is using this data structure. */
	ud->ud_UseCount++;

	result = TRUE;

 out:

	return(result);
}

/****************************************************************************/

/* The following function is called whenever the library is closed. Its
   purpose is to release any resources allocated by the UserLibOpen()
   function, such as when the last caller has closed the library. */
VOID
UserLibClose(struct UserData * ud)
{
	/* Remember that one less customer is using this data structure. */
	ud->ud_UseCount--;

	/* For the AmigaOS4 build, invoke the clib2 shared library
	   cleanup code. Note that this is not strictly
	   necessary. In fact, you should not need this functionality
	   if you stick to use Amiga operating system routines only
	   and stay away from using 'C' runtime library functions
	   that are not reentrant, such as malloc() or fprintf().
	   Use this feature only if you are porting code to the Amiga
	   which cannot be easily converted to follow the AmigaOS
	   API definitions only. */
	#if defined(__amigaos4__) && defined(__THREAD_SAFE)
	{
		/* Note that the clib2 library cleanup code is
		   called exactly once, when the last client
		   closes this library. */
		if(ud->ud_UseCount == 0)
			__lib_exit();
	}
	#endif /* __amigaos4__ && __THREAD_SAFE */
}

/****************************************************************************/

/* The following function is called shortly before the library is to be
   unloaded from memory. Its purpose is to release any resources
   allocated by the UserLibInit() function. Note that this function must
   not break the Forbid() condition, i.e. it must not call Wait() directly
   or indirectly. */
VOID
UserLibExpunge(struct UserData * ud)
{
	/* In this brief example no special cleanup operations are performed. */
}
