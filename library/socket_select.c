/*
 * $Id: socket_select.c,v 1.1.1.1 2004-07-26 16:31:19 obarthel Exp $
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

#ifndef _STDLIB_MEM_DEBUG_H
#include "stdlib_mem_debug.h"
#endif /* _STDLIB_MEM_DEBUG_H */

/****************************************************************************/

#ifndef _SOCKET_HEADERS_H
#include "socket_headers.h"
#endif /* _SOCKET_HEADERS_H */

/****************************************************************************/

static void
copy_fd_set(fd_set * to,fd_set * from,int num_fds)
{
	ENTER();

	SHOWPOINTER(to);
	SHOWPOINTER(from);
	SHOWVALUE(num_fds);

	if(to != NULL && from != NULL && num_fds > 0)
	{
		size_t num_bytes;

		num_bytes = sizeof(unsigned long) * ((num_fds + 31) / 32);

		SHOWVALUE(num_bytes);

		memmove(to,from,num_bytes);
	}

	LEAVE();
}

/****************************************************************************/

static void
zero_fd_set(fd_set * set,int num_fds)
{
	ENTER();

	SHOWPOINTER(set);
	SHOWVALUE(num_fds);

	if(set != NULL && num_fds > 0)
	{
		size_t num_bytes;

		num_bytes = sizeof(unsigned long) * ((num_fds + 31) / 32);

		SHOWVALUE(num_bytes);

		memset(set,0,num_bytes);
	}

	LEAVE();
}

/****************************************************************************/

static fd_set *
allocate_fd_set(int num_fds,fd_set * duplicate_this_set)
{
	fd_set * result = NULL;
	size_t num_bytes;
	fd_set * set;

	ENTER();

	assert( num_fds > 0 );

	SHOWVALUE(num_fds);

	num_bytes = sizeof(unsigned long) * ((num_fds + 31) / 32);

	SHOWVALUE(num_bytes);

	set = (fd_set *)malloc(num_bytes);
	if(set != NULL)
	{
		if(duplicate_this_set != NULL)
			copy_fd_set(set,duplicate_this_set,num_fds);
		else
			zero_fd_set(set,num_fds);

		result = set;
	}

	RETURN(result);
	return(result);
}

/****************************************************************************/

static void
free_fd_set(fd_set * set)
{
	if(set != NULL)
		free(set);
}

/****************************************************************************/

static struct fd *
get_file_descriptor(int file_descriptor)
{
	struct fd * result = NULL;
	struct fd * fd;

	if(file_descriptor < 0 || file_descriptor >= __num_fd)
		goto out;

	assert( __fd != NULL );

	fd = __fd[file_descriptor];

	assert( fd != NULL );

	if(FLAG_IS_CLEAR(fd->fd_Flags,FDF_IN_USE))
		goto out;

	result = fd;

 out:

	return(result);
}

/****************************************************************************/

static void
fix_datestamp(struct DateStamp * ds)
{
	const LONG ticks_per_minute	= 60 * TICKS_PER_SECOND;
	const LONG minutes_per_day	= 24 * 60;

	assert( ds != NULL );

	while(ds->ds_Minute	>= minutes_per_day ||
	      ds->ds_Tick	>= ticks_per_minute)
	{
		if(ds->ds_Minute >= minutes_per_day)
		{
			ds->ds_Days++;

			ds->ds_Minute -= minutes_per_day;
		}

		if(ds->ds_Tick >= ticks_per_minute)
		{
			ds->ds_Minute++;

			ds->ds_Tick -= ticks_per_minute;
		}
	}
}

/****************************************************************************/

static struct DateStamp *
timeval_to_datestamp(struct DateStamp * ds,const struct timeval * tv)
{
	assert( ds != NULL && tv != NULL );

	ds->ds_Days		= (tv->tv_secs / (24 * 60 * 60));
	ds->ds_Minute	= (tv->tv_secs % (24 * 60 * 60)) / 60;
	ds->ds_Tick		= (tv->tv_secs % 60) * TICKS_PER_SECOND + (TICKS_PER_SECOND * tv->tv_micro) / 1000000;

	fix_datestamp(ds);

	return(ds);
}

/****************************************************************************/

static void
add_dates(struct DateStamp * to,const struct DateStamp * from)
{
	assert( to != NULL && from != NULL );

	to->ds_Tick		+= from->ds_Tick;
	to->ds_Minute	+= from->ds_Minute;
	to->ds_Days		+= from->ds_Days;

	fix_datestamp(to);
}

/****************************************************************************/

static void
map_descriptor_sets(
	const fd_set *	input_fds,
	int				num_input_fds,

	fd_set *		socket_fds,
	int				num_socket_fds,
	int *			total_socket_fd_ptr,

	fd_set *		file_fds,
	int				num_file_fds,
	int *			total_file_fd_ptr)
{
	ENTER();

	SHOWPOINTER(input_fds);
	SHOWVALUE(num_input_fds);

	SHOWPOINTER(socket_fds);
	SHOWVALUE(num_socket_fds);

	SHOWPOINTER(file_fds);
	SHOWVALUE(num_file_fds);

	/* This routine maps file descriptor sets
	 * from one format to another. We map
	 * socket descriptors and regular file
	 * descriptor sets.
	 */
	if(input_fds != NULL && num_input_fds > 0)
	{
		int total_socket_fd;
		int total_file_fd;
		struct fd * fd;
		int file_fd;

		total_socket_fd	= (*total_socket_fd_ptr);
		total_file_fd	= (*total_file_fd_ptr);

		SHOWVALUE(total_socket_fd);
		SHOWVALUE(total_file_fd);

		for(file_fd = 0 ; file_fd < num_input_fds ; file_fd++)
		{
			if(NOT FD_ISSET(file_fd,input_fds))
				continue;

			D(("descriptor %ld is set",file_fd));

			fd = get_file_descriptor(file_fd);
			if(fd == NULL)
			{
				SHOWMSG("but no file is attached to it");
				continue;
			}

			/* Is this a socket descriptor? */
			if(FLAG_IS_SET(fd->fd_Flags,FDF_IS_SOCKET))
			{
				int socket_fd = (int)fd->fd_DefaultFile;

				D(("corresponds to socket #%ld",socket_fd));

				if(socket_fds != NULL && socket_fd < num_socket_fds)
				{
					SHOWMSG("setting it");

					FD_SET(socket_fd,socket_fds);

					if(total_socket_fd < socket_fd+1)
						total_socket_fd = socket_fd+1;
				}
				else
				{
					SHOWMSG("can't set it, though");
				}
			}
			else
			{
				LONG is_interactive;

				/* We only watch files bound to
				 * console streams.
				 */
				PROFILE_OFF();
				is_interactive = IsInteractive(fd->fd_DefaultFile);
				PROFILE_ON();

				if(NOT is_interactive)
				{
					SHOWMSG("this is a file");
					continue;
				}

				SHOWMSG("this is an interactive stream");

				if(file_fds != NULL && file_fd < num_file_fds)
				{
					SHOWMSG("setting it");

					FD_SET(file_fd,file_fds);

					if(total_file_fd < file_fd+1)
						total_file_fd = file_fd+1;
				}
				else
				{
					SHOWMSG("can't set it, though");
				}
			}
		}

		(*total_socket_fd_ptr)	= total_socket_fd;
		(*total_file_fd_ptr)	= total_file_fd;

		SHOWVALUE(total_socket_fd);
		SHOWVALUE(total_file_fd);
	}

	LEAVE();
}

/****************************************************************************/

static void
remap_descriptor_sets(
	const fd_set *	socket_fds,
	int				num_socket_fds,

	const fd_set *	file_fds,
	int				num_file_fds,

	fd_set *		output_fds,
	int				num_output_fds)
{
	ENTER();

	SHOWPOINTER(socket_fds);
	SHOWVALUE(num_socket_fds);

	SHOWPOINTER(file_fds);
	SHOWVALUE(num_file_fds);

	SHOWPOINTER(output_fds);
	SHOWVALUE(num_output_fds);

	/* This routine reverses the mapping established
	 * above. We map the file and socket descriptor
	 * sets back into the original set.
	 */
	if(output_fds != NULL && num_output_fds > 0)
	{
		zero_fd_set(output_fds,num_output_fds);

		if(socket_fds != NULL && num_socket_fds > 0)
		{
			struct fd * fd;
			int output_fd;
			int socket_fd;

			SHOWMSG("taking care of the sockets");

			for(socket_fd = 0 ; socket_fd < num_socket_fds ; socket_fd++)
			{
				if(NOT FD_ISSET(socket_fd,socket_fds))
					continue;

				for(output_fd = 0 ; output_fd < num_output_fds ; output_fd++)
				{
					fd = get_file_descriptor(output_fd);
					if(fd != NULL && FLAG_IS_SET(fd->fd_Flags,FDF_IS_SOCKET) && (int)fd->fd_DefaultFile == socket_fd)
					{
						assert( output_fd < num_output_fds );
						assert( FLAG_IS_SET(__fd[output_fd]->fd_Flags,FDF_IS_SOCKET) );

						D(("setting file %ld for socket #%ld",output_fd,socket_fd));

						FD_SET(output_fd,output_fds);
					}
				}
			}
		}

		if(file_fds != NULL && num_file_fds > 0)
		{
			int file_fd;

			SHOWMSG("taking care of the files");

			for(file_fd = 0 ; file_fd < num_file_fds ; file_fd++)
			{
				if(FD_ISSET(file_fd,file_fds))
				{
					int output_fd = file_fd;

					assert( output_fd < num_output_fds );
					assert( FLAG_IS_CLEAR(__fd[output_fd]->fd_Flags,FDF_IS_SOCKET) );

					D(("setting file %ld",file_fd));

					FD_SET(output_fd,output_fds);
				}
			}
		}
	}
	else
	{
		SHOWMSG("no output necessary");
	}

	LEAVE();
}

/****************************************************************************/

static void
get_num_descriptors_used(int num_fds,int * num_socket_used_ptr,int * num_file_used_ptr)
{
	int num_socket_used = 0;
	int num_file_used = 0;
	int which_file_fd;
	struct fd * fd;

	assert( num_socket_used_ptr != NULL );
	assert( num_file_used_ptr != NULL );

	SHOWMSG("figuring out which file descriptors are in use");

	for(which_file_fd = 0 ; which_file_fd < num_fds ; which_file_fd++)
	{
		fd = get_file_descriptor(which_file_fd);
		if(fd != NULL)
		{
			if(FLAG_IS_SET(fd->fd_Flags,FDF_IS_SOCKET))
			{
				int which_socket_fd = (int)fd->fd_DefaultFile;

				if(num_socket_used < which_socket_fd+1)
					num_socket_used = which_socket_fd+1;
			}
			else
			{
				if(num_file_used < which_file_fd+1)
					num_file_used = which_file_fd+1;
			}
		}
	}

	(*num_socket_used_ptr)	= num_socket_used;
	(*num_file_used_ptr)	= num_file_used;
}

/****************************************************************************/

int
select(int num_fds,fd_set *read_fds,fd_set *write_fds,fd_set *except_fds,struct timeval *timeout)
{
	fd_set * backup_socket_read_fds = NULL;
	fd_set * backup_socket_write_fds = NULL;
	fd_set * backup_socket_except_fds = NULL;

	fd_set * backup_file_read_fds = NULL;
	fd_set * backup_file_write_fds = NULL;

	fd_set * socket_read_fds = NULL;
	fd_set * socket_write_fds = NULL;
	fd_set * socket_except_fds = NULL;
	int total_socket_fd;

	fd_set * file_read_fds = NULL;
	fd_set * file_write_fds = NULL;
	int total_file_fd;

	struct fd * fd;
	int result = -1;

	int num_socket_used;
	int num_file_used;
	int i;

	ENTER();

	SHOWVALUE(num_fds);
	SHOWPOINTER(read_fds);
	SHOWPOINTER(write_fds);
	SHOWPOINTER(except_fds);
	SHOWPOINTER(timeout);

	if(timeout != NULL)
	{
		SHOWVALUE(timeout->tv_secs);
		SHOWVALUE(timeout->tv_micro);
	}

	assert(__SocketBase != NULL);

	if(__check_abort_enabled)
		__check_abort();

	/* Figure out the number of file and socket descriptors in use. */
	get_num_descriptors_used(num_fds,&num_socket_used,&num_file_used);

	SHOWVALUE(num_socket_used);
	SHOWVALUE(num_file_used);

	/* Dynamically allocate the tables to keep track of which descriptor
	 * is ready for I/O.
	 */
	if(read_fds != NULL)
	{
		if(num_socket_used > 0)
		{
			SHOWMSG("allocating read socket fd_set");

			socket_read_fds = allocate_fd_set(num_socket_used,NULL);
			if(socket_read_fds == NULL)
				goto out;
		}

		if(num_file_used > 0)
		{
			SHOWMSG("allocating read file fd_set");

			file_read_fds = allocate_fd_set(num_file_used,NULL);
			if(file_read_fds == NULL)
				goto out;
		}
	}

	if(write_fds != NULL)
	{
		if(num_socket_used > 0)
		{
			SHOWMSG("allocating write socket fd_set");

			socket_write_fds = allocate_fd_set(num_socket_used,NULL);
			if(socket_write_fds == NULL)
				goto out;
		}

		if(num_file_used > 0)
		{
			SHOWMSG("allocating write file fd_set");

			file_write_fds = allocate_fd_set(num_file_used,NULL);
			if(file_write_fds == NULL)
				goto out;
		}
	}

	if(except_fds != NULL)
	{
		if(num_socket_used > 0)
		{
			SHOWMSG("allocating except socket fd_set");

			socket_except_fds = allocate_fd_set(num_socket_used,NULL);
			if(socket_except_fds == NULL)
				goto out;
		}
	}

	total_socket_fd	= 0;
	total_file_fd	= 0;

	SHOWMSG("mapping the fd_sets");

	/* Translate from the tables the caller provided to us
	 * to the local copies, which take the files and sockets
	 * into account.
	 */
	map_descriptor_sets(read_fds,	num_fds,	socket_read_fds,	num_socket_used,	&total_socket_fd,	file_read_fds,		num_file_used,	&total_file_fd);
	map_descriptor_sets(write_fds,	num_fds,	socket_write_fds,	num_socket_used,	&total_socket_fd,	file_write_fds,		num_file_used,	&total_file_fd);
	map_descriptor_sets(except_fds,	num_fds,	socket_except_fds,	num_socket_used,	&total_socket_fd,	NULL,				0,				&total_file_fd);

	/* Wait for socket input? */
	if(total_socket_fd > 0)
	{
		SHOWMSG("we have to deal with sockets");

		/* Wait for file input, too? */
		if((total_file_fd > 0) && (timeout == NULL || timeout->tv_secs > 0 || timeout->tv_micro > 0))
		{
			struct DateStamp stop_when;
			struct timeval zero;
			ULONG break_mask;
			BOOL got_input;
			BOOL got_output;

			SHOWMSG("we also have to deal with files");

			/* We may poll the sockets and files several times in a row.
			 * The results stored in the tables can tell only a single
			 * story, though. This is why we need to keep a backup copy
			 * of each table around, to be used for each iteration of
			 * the loop in which the sockets and files are checked.
			 */
			if(read_fds != NULL)
			{
				if(num_socket_used > 0)
				{
					SHOWMSG("allocating backup read socket fd_set");

					backup_socket_read_fds = allocate_fd_set(num_socket_used,socket_read_fds);
					if(backup_socket_read_fds == NULL)
						goto out;
				}

				if(num_file_used > 0)
				{
					SHOWMSG("allocating backup read file fd_set");

					backup_file_read_fds = allocate_fd_set(num_file_used,file_read_fds);
					if(backup_file_read_fds == NULL)
						goto out;
				}
			}

			if(write_fds != NULL)
			{
				if(num_socket_used > 0)
				{
					SHOWMSG("allocating backup write socket fd_set");

					backup_socket_write_fds = allocate_fd_set(num_socket_used,socket_write_fds);
					if(backup_socket_write_fds == NULL)
						goto out;
				}

				if(num_file_used > 0)
				{
					SHOWMSG("allocating backup write file fd_set");

					backup_file_write_fds = allocate_fd_set(num_file_used,file_write_fds);
					if(backup_file_write_fds == NULL)
						goto out;
				}
			}

			if(except_fds != NULL)
			{
				if(num_socket_used > 0)
				{
					SHOWMSG("allocating backup except socket fd_set");

					backup_socket_except_fds = allocate_fd_set(num_socket_used,socket_except_fds);
					if(backup_socket_except_fds	== NULL)
						goto out;
				}
			}

			/* We are going to poll all streams; for the timeout
			 * feature to work, we absolutely must know when to
			 * stop polling.
			 *
			 * Why aren't we using asynchronous DOS packets?
			 * The answer is that once a packet is sent, you
			 * cannot easily abort it. Polling is safer in
			 * that respect. Yes, I know that ACTION_STACK
			 * can be used to fake input to a console stream,
			 * but I'd rather not rely upon it.
			 */
			if(timeout != NULL)
			{
				struct DateStamp datestamp_timeout;

				PROFILE_OFF();
				DateStamp(&stop_when);
				PROFILE_ON();

				add_dates(&stop_when,timeval_to_datestamp(&datestamp_timeout,timeout));
			}
			else
			{
				/* No timeout, poll until we are interrupted
				 * or get input from any of the files. It's
				 * not really necessary to initialize this
				 * timeval, but it keeps the compiler happy.
				 */
				memset(&stop_when,0,sizeof(stop_when));
			}

			while(TRUE)
			{
				/* Check for break signal. */
				if(__check_abort_enabled)
					__check_abort();

				/* Delay for a tick to avoid busy-waiting. */
				PROFILE_OFF();
				Delay(1);
				PROFILE_ON();

				/* This tells WaitSelect() to poll the sockets for input. */
				zero.tv_secs	= 0;
				zero.tv_micro	= 0;

				/* Signals to stop on; we want to stop when a break signal arrives. */
				if(__check_abort_enabled)
					break_mask = SIGBREAKF_CTRL_C;
				else
					break_mask = 0;

				/* Check for socket input. */
				PROFILE_OFF();
				result = __WaitSelect(total_socket_fd,socket_read_fds,socket_write_fds,socket_except_fds,&zero,&break_mask);
				PROFILE_ON();

				/* Stop if a break signal arrives. */
				if((result < 0 && errno == EINTR) || FLAG_IS_SET(break_mask,SIGBREAKF_CTRL_C))
				{
					SetSignal(SIGBREAKF_CTRL_C,SIGBREAKF_CTRL_C);
					__check_abort();
				}

				/* Stop if the return value from WaitSelect() is negative (timeout, abort or serious error). */
				if(result < 0)
					break;

				/* Check all files for input. We also poll
				 * them for input, but each with a little
				 * delay of about 1 microsecond. We stop
				 * as soon as we find one file that has
				 * input in it.
				 */
				for(i = 0 ; i < total_file_fd ; i++)
				{
					got_input = got_output = FALSE;

					fd = get_file_descriptor(i);
					if(fd != NULL)
					{
						if(file_read_fds != NULL && FD_ISSET(i,file_read_fds))
						{
							if(FLAG_IS_SET(fd->fd_Flags,FDF_READ))
							{
								assert( FLAG_IS_CLEAR(fd->fd_Flags,FDF_IS_SOCKET) );

								/* Does this one have input? */
								PROFILE_OFF();

								if(WaitForChar(fd->fd_DefaultFile,1))
									got_input = TRUE;

								PROFILE_ON();
							}
						}

						if(file_write_fds != NULL && FD_ISSET(i,file_write_fds))
						{
							/* If this is a writable stream, assume that we can
							 * write to it. ZZZ AmigaDOS needs a method to check
							 * if the write will block.
							 */
							if(FLAG_IS_SET(fd->fd_Flags,FDF_WRITE))
							{
								assert( FLAG_IS_CLEAR(fd->fd_Flags,FDF_IS_SOCKET) );

								got_output = TRUE;
							}
						}
					}

					if(got_input || got_output)
					{
						/* Mark one more descriptor as
						 * having input/output.
						 */
						result++;
					}

					if(file_read_fds != NULL && NOT got_input)
						FD_CLR(i,file_read_fds);

					if(file_write_fds != NULL && NOT got_output)
						FD_CLR(i,file_write_fds);
				}

				/* Did we get any input? If so, stop polling. */
				if(result > 0)
					break;

				/* If a timeout was set, check if we are already
				 * beyond the point of time when we should have
				 * stopped polling.
				 */
				if(timeout != NULL)
				{
					struct DateStamp now;

					PROFILE_OFF();
					DateStamp(&now);
					PROFILE_ON();

					if(CompareDates(&now,&stop_when) <= 0)
						break;
				}

				/* No I/O ready yet. Restore the sets and retry... */
				copy_fd_set(socket_read_fds,	backup_socket_read_fds,		num_socket_used);
				copy_fd_set(socket_write_fds,	backup_socket_write_fds,	num_socket_used);
				copy_fd_set(socket_except_fds,	backup_socket_except_fds,	num_socket_used);

				copy_fd_set(file_read_fds,		backup_file_read_fds,		num_file_used);
				copy_fd_set(file_write_fds,		backup_file_write_fds,		num_file_used);
			}
		}
		else
		{
			ULONG break_mask;

			if(__check_abort_enabled)
				break_mask = SIGBREAKF_CTRL_C;
			else
				break_mask = 0;

			PROFILE_OFF();
			result = __WaitSelect(total_socket_fd,socket_read_fds,socket_write_fds,socket_except_fds,timeout,&break_mask);
			PROFILE_ON();

			if((result < 0 && errno == EINTR) || FLAG_IS_SET(break_mask,SIGBREAKF_CTRL_C))
			{
				SetSignal(SIGBREAKF_CTRL_C,SIGBREAKF_CTRL_C);
				__check_abort();
			}
		}
	}
	else
	{
		/* Wait for file input? */
		if((total_file_fd > 0) && (timeout == NULL || timeout->tv_secs > 0 || timeout->tv_micro > 0))
		{
			struct DateStamp stop_when;
			BOOL got_input;
			BOOL got_output;

			SHOWMSG("we have to deal with files");

			if(num_file_used > 0)
			{
				if(read_fds != NULL)
				{
					SHOWMSG("allocating backup file read fd_set");

					backup_file_read_fds = allocate_fd_set(num_file_used,file_read_fds);
					if(backup_file_read_fds == NULL)
						goto out;
				}

				if(write_fds != NULL)
				{
					SHOWMSG("allocating backup file write fd_set");

					backup_file_write_fds = allocate_fd_set(num_file_used,file_write_fds);
					if(backup_file_write_fds == NULL)
						goto out;
				}
			}

			if(timeout != NULL)
			{
				struct DateStamp datestamp_timeout;

				PROFILE_OFF();
				DateStamp(&stop_when);
				PROFILE_ON();

				add_dates(&stop_when,timeval_to_datestamp(&datestamp_timeout,timeout));
			}
			else
			{
				memset(&stop_when,0,sizeof(stop_when));
			}

			while(TRUE)
			{
				if(__check_abort_enabled)
					__check_abort();

				PROFILE_OFF();
				Delay(1);
				PROFILE_ON();

				result = 0;

				for(i = 0 ; i < total_file_fd ; i++)
				{
					got_input = got_output = FALSE;

					fd = get_file_descriptor(i);
					if(fd != NULL)
					{
						if(file_read_fds != NULL && FD_ISSET(i,file_read_fds))
						{
							if(FLAG_IS_SET(fd->fd_Flags,FDF_READ))
							{
								assert( FLAG_IS_CLEAR(fd->fd_Flags,FDF_IS_SOCKET) );

								PROFILE_OFF();

								if(WaitForChar(fd->fd_DefaultFile,1))
									got_input = TRUE;

								PROFILE_ON();
							}
						}

						if(file_write_fds != NULL && FD_ISSET(i,file_write_fds))
						{
							if(FLAG_IS_SET(fd->fd_Flags,FDF_WRITE))
							{
								assert( FLAG_IS_CLEAR(fd->fd_Flags,FDF_IS_SOCKET) );

								got_output = TRUE;
							}
						}
					}

					if(got_input || got_output)
						result++;

					if(file_read_fds != NULL && NOT got_input)
						FD_CLR(i,file_read_fds);

					if(file_write_fds != NULL && NOT got_output)
						FD_CLR(i,file_write_fds);
				}

				if(result > 0)
					break;

				if(timeout != NULL)
				{
					struct DateStamp now;

					PROFILE_OFF();
					DateStamp(&now);
					PROFILE_ON();

					if(CompareDates(&now,&stop_when) <= 0)
						break;
				}

				copy_fd_set(file_read_fds,	backup_file_read_fds,	num_file_used);
				copy_fd_set(file_write_fds,	backup_file_write_fds,	num_file_used);
			}
		}
		else
		{
			SHOWMSG("no files to worry about");
		}
	}

	/* The descriptor sets remain unchanged in
	 * case of error.
	 */
	if(result >= 0)
	{
		SHOWMSG("remapping fd_sets");

		remap_descriptor_sets(socket_read_fds,		total_socket_fd,	file_read_fds,		total_file_fd,	read_fds,	num_fds);
		remap_descriptor_sets(socket_write_fds,		total_socket_fd,	file_write_fds,		total_file_fd,	write_fds,	num_fds);
		remap_descriptor_sets(socket_except_fds,	total_socket_fd,	NULL,				0,				except_fds,	num_fds);
	}

	if(__check_abort_enabled)
		__check_abort();

 out:

	free_fd_set(socket_read_fds);
	free_fd_set(socket_write_fds);
	free_fd_set(socket_except_fds);

	free_fd_set(file_read_fds);
	free_fd_set(file_write_fds);

	free_fd_set(backup_socket_read_fds);
	free_fd_set(backup_socket_write_fds);
	free_fd_set(backup_socket_except_fds);

	free_fd_set(backup_file_read_fds);
	free_fd_set(backup_file_write_fds);

	RETURN(result);
	return(result);
}

/****************************************************************************/

#endif /* SOCKET_SUPPORT */
