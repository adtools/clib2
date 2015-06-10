/*
 * $Id: socket_obtain_daemon.c,v 1.5 2006-11-16 10:41:15 obarthel Exp $
 *
 * :ts=4
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

#if defined(SOCKET_SUPPORT)

/****************************************************************************/

#ifndef _SOCKET_HEADERS_H
#include "socket_headers.h"
#endif /* _SOCKET_HEADERS_H */

/****************************************************************************/

/* This data structure is attached to the server's Process->pr_ExitData field
   if the server was launched by the inetd Internet super-server. */
struct _DaemonMessage
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

/* Check if this program was launched as a server by the Internet
   superserver. If this is so, obtain the 'daemon message' associated
   with this server process and map the standard I/O streams to the
   server socket. This function must return FALSE if it fails to
   allocate the necessary resources for rebinding the standard I/O
   streams. */
BOOL
__obtain_daemon_message(VOID)
{
	BOOL success = FALSE;

	if(Cli() != NULL && NOT __detach && __check_daemon_startup)
	{
		struct _DaemonMessage * dm;

		/* The socket the superserver may have launched this program
		   with is attached to the exit data entry of the process. */
		#if defined(__amigaos4__)
		{
			dm = (struct _DaemonMessage *)GetExitData();
		}
		#else
		{
			struct Process * this_process = (struct Process *)FindTask(NULL);

			dm = (struct _DaemonMessage *)this_process->pr_ExitData;
		}
		#endif /* __amigaos4__ */

		/* For extra safety, ask if what could be a "struct DaemonMessage"
		   pointer was really placed there by the Internet super-server. */
		if(__SocketBase->lib_Version >= 4)
		{
			LONG have_server_api = FALSE;
			struct TagItem tags[2];

			/* Check if it is safe to call the IsServerProcess() function. */
			tags[0].ti_Tag	= SBTM_GETREF(SBTC_HAVE_SERVER_API);
			tags[0].ti_Data	= (ULONG)&have_server_api;
			tags[1].ti_Tag	= TAG_END;

			PROFILE_OFF();

			if(__SocketBaseTagList(tags) != 0)
				have_server_api = FALSE;

			PROFILE_ON();

			/* If it's safe to call ProcessIsServer(), ask if the
			   "struct DaemonMessage" pointer is valid. If it's not,
			   set the message pointer to NULL, ignoring it altogether. */
			if(have_server_api && NOT __ProcessIsServer(NULL))
				dm = NULL;
		}

		if(dm != NULL && TypeOfMem(dm) != 0 && TypeOfMem(((char *)dm) + sizeof(*dm)-1) != 0)
		{
			struct SignalSemaphore * lock;
			int daemon_socket;
			struct fd * fd;
			int sockfd;
			int i;

			SHOWMSG("we have a daemon message; this is a server");

			/* Try to grab that socket and attach is to the three
			   standard I/O streams. */

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

	return(success);
}

/****************************************************************************/

#endif /* SOCKET_SUPPORT */
