/*
 * $Id: socket_init_exit.c,v 1.23 2005-10-20 07:19:15 obarthel Exp $
 *
 * :ts=4
 *
 * Portable ISO 'C' (1994) runtime library for the Amiga computer
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

#if defined(SOCKET_SUPPORT)

/****************************************************************************/

#ifndef _SOCKET_HEADERS_H
#include "socket_headers.h"
#endif /* _SOCKET_HEADERS_H */

#ifndef _SIGNAL_HEADERS_H
#include "signal_headers.h"
#endif /* _SIGNAL_HEADERS_H */

/****************************************************************************/

#ifndef _STDLIB_CONSTRUCTOR_H
#include "stdlib_constructor.h"
#endif /* _STDLIB_CONSTRUCTOR_H */

/****************************************************************************/

/* Code value. */
#define SBTB_CODE 1
#define SBTS_CODE 0x3FFF

/* Set a parameter, passing it by value. */
#ifndef SBTM_SETVAL
#define SBTM_SETVAL(code) (TAG_USER | (((code) & SBTS_CODE) << SBTB_CODE) | 1)
#endif /* SBTM_SETVAL */

#define SBTC_BREAKMASK					1	/* Interrupt signal mask */
#define SBTC_LOGTAGPTR					11	/* Under which name log entries are filed */
#define SBTC_ERRNOLONGPTR				24	/* Pointer to errno, length == sizeof(errno) */
#define SBTC_HERRNOLONGPTR				25	/* 'h_errno' pointer (with sizeof(h_errno) == sizeof(long)) */
#define SBTC_CAN_SHARE_LIBRARY_BASES	51	/* Enable library base sharing among Processes */
#define SBTC_ERROR_HOOK					68	/* Error hook pointer */

/****************************************************************************/

/* Call-back hook for use with SBTC_ERROR_HOOK */
struct ErrorHookMsg
{
	ULONG	ehm_Size;	/* Size of this data structure; this
						   must be >= 12 */
	ULONG	ehm_Action;	/* See below for a list of definitions */

	LONG	ehm_Code;	/* The error code to use */
};

/* Which action the hook is to perform */
#define EHMA_Set_errno		1	/* Set the local 'errno' to what is
								   found in ehm_Code */
#define EHMA_Set_h_errno	2	/* Set the local 'h_errno' to what is
								   found in ehm_Code */

/****************************************************************************/

struct DaemonMessage
{
	struct Message	dm_Message;
	ULONG			dm_Pad1;
	ULONG			dm_Pad2;
	LONG			dm_ID;
	ULONG			dm_Pad3;
	UBYTE			dm_Family;
	UBYTE			dm_Type;
};

/****************************************************************************/

struct Library * NOCOMMON __SocketBase;

/****************************************************************************/

#if defined(__amigaos4__)
struct SocketIFace * NOCOMMON __ISocket;
#endif /* __amigaos4__ */

/****************************************************************************/

int NOCOMMON h_errno;

/****************************************************************************/

#if defined(__THREAD_SAFE)

/****************************************************************************/

BOOL NOCOMMON __can_share_socket_library_base;
BOOL NOCOMMON __thread_safe_errno_h_errno;

/****************************************************************************/

STATIC LONG ASM
error_hook_function(
	REG(a0, struct Hook *			unused_hook),
	REG(a2, APTR					unused_reserved),
	REG(a1, struct ErrorHookMsg *	ehm))
{
	if(ehm != NULL && ehm->ehm_Size >= 12)
	{
		if (ehm->ehm_Action == EHMA_Set_errno)
			__set_errno(ehm->ehm_Code);
		else if (ehm->ehm_Action == EHMA_Set_h_errno)
			__set_h_errno(ehm->ehm_Code);
	}

	return(0);
}

/****************************************************************************/

STATIC struct Hook error_hook =
{
	{ NULL, NULL },
	(HOOKFUNC)error_hook_function,
	(HOOKFUNC)NULL,
	NULL
};

/****************************************************************************/

#endif /* __THREAD_SAFE */

/****************************************************************************/

SOCKET_DESTRUCTOR(socket_exit)
{
	ENTER();

	/* Disable ^C checking. */
	if(__SocketBase != NULL)
	{
		struct TagItem tags[2];

		tags[0].ti_Tag	= SBTM_SETVAL(SBTC_BREAKMASK);
		tags[0].ti_Data	= 0;
		tags[1].ti_Tag	= TAG_END;

		PROFILE_OFF();
		__SocketBaseTagList(tags);
		PROFILE_ON();
	}

	/*
	 * Careful: if this function is ever invoked, it must make sure that
	 *          the socket file descriptors are invalidated. If that
	 *          does not happen, the stdio cleanup function will
	 *          crash (with bells on).
	 */
	__close_all_files();

	#if defined(__amigaos4__)
	{
		if(__ISocket != NULL)
		{
			DropInterface((struct Interface *)__ISocket);
			__ISocket = NULL;
		}
	}
	#endif /* __amigaos4__ */

	if(__SocketBase != NULL)
	{
		CloseLibrary(__SocketBase);
		__SocketBase = NULL;
	}

	LEAVE();
}

/****************************************************************************/

SOCKET_CONSTRUCTOR(socket_init)
{
	struct TagItem tags[5];
	BOOL success = FALSE;
	LONG status;

	ENTER();

	PROFILE_OFF();

	/* bsdsocket.library V3 is sufficient for all the tasks we
	   may have to perform. */
	__SocketBase = OpenLibrary("bsdsocket.library",3);

	#if defined(__amigaos4__)
	{
		if(__SocketBase != NULL)
		{
			__ISocket = (struct SocketIFace *)GetInterface(__SocketBase, "main", 1, 0);
			if (__ISocket == NULL)
			{
				CloseLibrary(__SocketBase);
				__SocketBase = NULL;
			}
		}
	}
	#endif /* __amigaos4__ */

	PROFILE_ON();

	if(__SocketBase == NULL)
	{
		SHOWMSG("bsdsocket.library V3 didn't open");

		__show_error("\"bsdsocket.library\" V3 could not be opened.");
		goto out;
	}

	/* Wire the library's errno variable to our local errno. */
	tags[0].ti_Tag	= SBTM_SETVAL(SBTC_ERRNOLONGPTR);
	tags[0].ti_Data	= (ULONG)&errno;

	/* Also enable ^C checking if desired. */
	tags[1].ti_Tag = SBTM_SETVAL(SBTC_BREAKMASK);

	if(__check_abort_enabled)
		tags[1].ti_Data	= SIGBREAKF_CTRL_C;
	else
		tags[1].ti_Data	= 0;

	tags[2].ti_Tag	= SBTM_SETVAL(SBTC_LOGTAGPTR);
	tags[2].ti_Data	= (ULONG)__program_name;

	/* Wire the library's h_errno variable to our local h_errno. */
	tags[3].ti_Tag	= SBTM_SETVAL(SBTC_HERRNOLONGPTR);
	tags[3].ti_Data	= (ULONG)&h_errno;

	tags[4].ti_Tag = TAG_END;

	PROFILE_OFF();
	status = __SocketBaseTagList(tags);
	PROFILE_ON();

	if(status != 0)
	{
		SHOWMSG("couldn't initialize the library");

		__show_error("\"bsdsocket.library\" could not be initialized.");
		goto out;
	}

	/* In the thread-safe library we try to enable two features which so
	   far only the Roadshow TCP/IP stack supports: allow more than one
	   Process to use the same bsdsocket.library base and to propagate
	   changes to the errno and h_errno variable through a call-back
	   hook. If either of these features are supported can be checked
	   by looking at the global __can_share_socket_library_base and
	   __thread_safe_errno_h_errno variables. */
	#if defined(__THREAD_SAFE)
	{
		if(__SocketBase->lib_Version >= 4)
		{
			tags[0].ti_Tag	= SBTM_SETVAL(SBTC_CAN_SHARE_LIBRARY_BASES);
			tags[0].ti_Data	= TRUE;

			tags[1].ti_Tag	= TAG_END;

			PROFILE_OFF();

			if(__SocketBaseTagList(tags) == 0)
				__can_share_socket_library_base = TRUE;

			PROFILE_ON();

			if(__can_share_socket_library_base)
			{
				tags[0].ti_Tag	= SBTM_SETVAL(SBTC_ERROR_HOOK);
				tags[0].ti_Data	= (ULONG)&error_hook;

				tags[1].ti_Tag	= TAG_END;

				PROFILE_OFF();

				if(__SocketBaseTagList(tags) == 0)
					__thread_safe_errno_h_errno = TRUE;

				PROFILE_ON();
			}
		}
	}
	#endif /* __THREAD_SAFE */

	/* Check if this program was launched as a server by the Internet
	   superserver. */
	if(CANNOT __obtain_daemon_message())
		goto out;

	success = TRUE;

 out:

	SHOWVALUE(success);
	LEAVE();

	if(success)
		CONSTRUCTOR_SUCCEED();
	else
		CONSTRUCTOR_FAIL();
}

/****************************************************************************/

#endif /* SOCKET_SUPPORT */
