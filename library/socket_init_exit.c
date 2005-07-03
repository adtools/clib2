/*
 * $Id: socket_init_exit.c,v 1.20 2005-07-03 10:36:47 obarthel Exp $
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

#define SBTC_BREAKMASK		1	/* Interrupt signal mask */
#define SBTC_LOGTAGPTR		11	/* Under which name log entries are filed */
#define SBTC_ERRNOLONGPTR	24	/* Pointer to errno, length == sizeof(errno) */
#define SBTC_HERRNOLONGPTR	25	/* 'h_errno' pointer (with sizeof(h_errno) == sizeof(long)) */

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

	/* Careful: if this function is ever invoked, it must make sure that
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
	 * may have to perform.
	 */
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

	/* If this library is built with the Roadshow SDK header files, and
	 * it should be thread-safe, then we'll try to allow multiple
	 * concurred processes to share the bsdsocket.library base opened
	 * above. This only works for Roadshow, though, and has the
	 * drawback that error reporting through 'errno' and 'h_errno'
	 * is no longer safe.
	 */
	#if defined(__THREAD_SAFE) && defined(SBTC_CAN_SHARE_LIBRARY_BASES)
	{
		if(__SocketBase->lib_Version >= 4)
		{
			tags[0].ti_Tag	= SBTM_SETVAL(SBTC_CAN_SHARE_LIBRARY_BASES);
			tags[0].ti_Data	= TRUE;

			tags[1].ti_Tag	= TAG_END;

			PROFILE_OFF();
			__SocketBaseTagList(tags);
			PROFILE_ON();
		}
	}
	#endif /* __THREAD_SAFE && SBTC_CAN_SHARE_LIBRARY_BASES */

	/* Check if this program was launched as a server by the Internet
	 * superserver.
	 */
	if(Cli() != NULL && NOT __detach)
	{
		struct DaemonMessage * dm;

		/* The socket the superserver may have launched this program
		 * with is attached to the exit data entry of the process.
		 */
		#if defined(__amigaos4__)
		{
			dm = (struct DaemonMessage *)GetExitData();
		}
		#else
		{
			struct Process * this_process = (struct Process *)FindTask(NULL);

			dm = (struct DaemonMessage *)this_process->pr_ExitData;
		}
		#endif /* __amigaos4__ */

		if(TypeOfMem(dm) != 0 && TypeOfMem(((char *)dm) + sizeof(*dm)-1) != 0)
		{
			struct SignalSemaphore * lock;
			int daemon_socket;
			struct fd * fd;
			int sockfd;
			int i;

			SHOWMSG("we have a daemon message; this is a server");

			/* Try to grab that socket and attach is to the three
			 * standard I/O streams.
			 */

			PROFILE_OFF();
			daemon_socket = __ObtainSocket(dm->dm_ID,dm->dm_Family,dm->dm_Type,0);
			PROFILE_ON();

			if(daemon_socket == -1)
			{
				__show_error("Network server streams could not be initialized.");
				goto out;
			}

			SHOWVALUE(daemon_socket);

			/* Shut down the three standard I/O streams. */
			for(i = STDIN_FILENO ; i <= STDERR_FILENO ; i++)
				close(i);

			/* The standard I/O streams are no longer attached to a console. */
			__no_standard_io = TRUE;

			/* Put the socket into the three standard I/O streams. */
			for(i = STDIN_FILENO ; i <= STDERR_FILENO ; i++)
			{
				#if defined(__THREAD_SAFE)
				{
					lock = __create_semaphore();
					if(lock == NULL)
						goto out;
				}
				#else
				{
					lock = NULL;
				}
				#endif /* __THREAD_SAFE */

				fd = __fd[i];

				assert( fd != NULL && FLAG_IS_CLEAR(fd->fd_Flags,FDF_IN_USE) );

				if(i == STDIN_FILENO)
				{
					sockfd = daemon_socket;
				}
				else
				{
					PROFILE_OFF();
					sockfd = __Dup2Socket(daemon_socket,-1);
					PROFILE_ON();

					if(sockfd == -1)
					{
						SHOWMSG("could not duplicate daemon socket");

						#if defined(__THREAD_SAFE)
						{
							__delete_semaphore(lock);
						}
						#endif /* __THREAD_SAFE */

						__show_error("Network server streams could not be initialized.");
						goto out;
					}
				}

				__initialize_fd(fd,__socket_hook_entry,(BPTR)sockfd,FDF_IN_USE | FDF_IS_SOCKET | FDF_READ | FDF_WRITE,lock);
			}

			/* This program now runs as an internet superserver client (daemon). */
			__is_daemon = TRUE;
		}
	}

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
