/*
 * $Id: socket_init_exit.c,v 1.1.1.1 2004-07-26 16:31:16 obarthel Exp $
 *
 * :ts=4
 *
 * Portable ISO 'C' (1994) runtime library for the Amiga computer
 * Copyright (c) 2002-2004 by Olaf Barthel <olsen@sourcery.han.de>
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

/****************************************************************************/

#include "stdlib_protos.h"

/****************************************************************************/

extern BOOL __detach;

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

extern BOOL		__termination_message_disabled;
extern char *	__program_name;
extern BOOL		__is_daemon;

/****************************************************************************/

extern void __show_error(const char * message);

/****************************************************************************/

extern void __socket_hook_entry(struct Hook * hook,struct fd * fd,struct file_hook_message * message);

/****************************************************************************/

void
__socket_exit(void)
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
	if(__fd != NULL)
	{
		int i;

		for(i = 0 ; i < __num_fd ; i++)
		{
			if(FLAG_IS_SET(__fd[i]->fd_Flags,FDF_IS_SOCKET) && FLAG_IS_SET(__fd[i]->fd_Flags,FDF_IN_USE))
				close(i);
		}
	}

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

int
__socket_init(void)
{
	struct Process * this_process;
	struct TagItem tags[4];
	int result = ERROR;
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

	tags[3].ti_Tag = TAG_END;

	PROFILE_OFF();
	status = __SocketBaseTagList(tags);
	PROFILE_ON();

	if(status != 0)
	{
		SHOWMSG("couldn't initialize the library");

		__show_error("\"bsdsocket.library\" could not be initialized.");
		goto out;
	}

	/* Check if this program was launched as a server by the Internet
	 * superserver.
	 */
	this_process = (struct Process *)FindTask(NULL);
	if(this_process->pr_CLI != ZERO && NOT __detach)
	{
		struct DaemonMessage * dm;

		/* The socket the superserver may have launched this program
		 * with is attached to the exit data entry of the process.
		 */
		dm = (struct DaemonMessage *)this_process->pr_ExitData;
		if(TypeOfMem(dm) != 0 && TypeOfMem(((char *)dm) + sizeof(*dm)-1) != 0)
		{
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

			/* Whatever happens, the following likely won't end up
			 * in the console...
			 */
			__termination_message_disabled = TRUE;

			/* Shut down the three standard I/O streams. */
			for(i = STDIN_FILENO ; i <= STDERR_FILENO ; i++)
				close(i);

			/* Put the socket into the three standard I/O streams. */
			for(i = STDIN_FILENO ; i <= STDERR_FILENO ; i++)
			{
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
	
						__show_error("Network server streams could not be initialized.");
						goto out;
					}
				}

				__initialize_fd(fd,(HOOKFUNC)__socket_hook_entry,(BPTR)sockfd,FDF_IN_USE | FDF_IS_SOCKET | FDF_READ | FDF_WRITE);
			}

			/* This program now runs as an internet superserver client (daemon). */
			__is_daemon = TRUE;

			/* The standard I/O streams are no longer attached to a console. */
			__no_standard_io = TRUE;
		}
	}

	result = OK;

 out:

	RETURN(result);
	return(result);
}

/****************************************************************************/

#endif /* SOCKET_SUPPORT */
