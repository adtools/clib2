/*
 * $Id: stdio_headers.h,v 1.27 2006-09-25 14:05:31 obarthel Exp $
 *
 * :ts=4
 *
 * Portable ISO 'C' (1994) runtime library for the Amiga computer
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

#ifndef _STDIO_HEADERS_H
#define _STDIO_HEADERS_H

/****************************************************************************/

#ifndef EXEC_LIBRARIES_H
#include <exec/libraries.h>
#endif /* EXEC_LIBRARIES_H */

#ifndef EXEC_MEMORY_H
#include <exec/memory.h>
#endif /* EXEC_MEMORY_H */

#ifndef DEVICES_TIMER_H
#include <devices/timer.h>
#endif /* DEVICES_TIMER_H */

#ifndef DOS_DOSEXTENS_H
#include <dos/dosextens.h>
#endif /* DOS_DOSEXTENS_H */

#ifndef DOS_DOSTAGS_H
#include <dos/dostags.h>
#endif /* DOS_DOSTAGS_H */

#ifndef DOS_DOSASL_H
#include <dos/dosasl.h>
#endif /* DOS_DOSASL_H */

#ifndef DOS_RECORD_H
#include <dos/record.h>
#endif /* DOS_RECORD_H */

#ifndef LIBRARIES_LOCALE_H
#include <libraries/locale.h>
#endif /* LIBRARIES_LOCALE_H */

#ifndef UTILITY_HOOKS_H
#include <utility/hooks.h>
#endif /* UTILITY_HOOKS_H */

/****************************************************************************/

#ifndef CLIB_ALIB_PROTOS_H
#include <clib/alib_protos.h>
#endif /* CLIB_ALIB_PROTOS_H */

/****************************************************************************/

#ifndef PROTO_EXEC_H
#include <proto/exec.h>
#endif /* PROTO_EXEC_H */

#ifndef PROTO_DOS_H
#include <proto/dos.h>
#endif /* PROTO_DOS_H */

/****************************************************************************/

#include <signal.h>
#include <stdlib.h>
#include <string.h>
#include <limits.h>
#include <locale.h>
#include <stdio.h>
#include <errno.h>
#include <ctype.h>

/****************************************************************************/

#include <sys/stat.h>
#include <unistd.h>
#include <dirent.h>
#include <utime.h>
#include <fcntl.h>
#include <dos.h>

/****************************************************************************/

#if defined(FLOATING_POINT_SUPPORT)
#include <math.h>
#endif /* FLOATING_POINT_SUPPORT */

/****************************************************************************/

#ifndef _STDLIB_HEADERS_H
#include "stdlib_headers.h"
#endif /* _STDLIB_HEADERS_H */

/****************************************************************************/

#ifndef _MATH_FP_SUPPORT_H
#include "math_fp_support.h"
#endif /* _MATH_FP_SUPPORT_H */

/****************************************************************************/

#ifndef _STDLIB_PROFILE_H
#include "stdlib_profile.h"
#endif /* _STDLIB_PROFILE_H */

/****************************************************************************/

#ifndef _MACROS_H
#include "macros.h"
#endif /* _MACROS_H */

#ifndef _DEBUG_H
#include "debug.h"
#endif /* _DEBUG_H */

/****************************************************************************/

/* Forward declarations for below... */
struct fd;
struct iob;

/****************************************************************************/

/* CPU cache line size; used for alignment purposes with some data structures.
   This should be determined dynamically rather than preset here. For the
   68040/68060 the cache line size is 16 bytes, for the PowerPC G4 it's
   32 bytes and 128 bytes (gross!) for the PowerPC G5. */
#if defined(__PPC__)
#define CACHE_LINE_SIZE 32UL
#else
#define CACHE_LINE_SIZE 16UL
#endif /* __PPC__ */

/****************************************************************************/

/* The directory entry type a socket is identified with (in a FileInfoBlock). */
#define ST_SOCKET (31082002)

/* The same for a console stream. */
#define ST_CONSOLE (20050402)

/* And something for NIL: */
#define ST_NIL (20060920)

/****************************************************************************/

/* Operations that can be performed by the file action function. */
enum file_action_t
{
	file_action_read,
	file_action_write,
	file_action_seek,
	file_action_close,
	file_action_set_blocking,
	file_action_set_async,
	file_action_examine
};

/****************************************************************************/

/* A message sent to a file action function. */
struct file_action_message
{
	enum file_action_t		fam_Action;		/* What to do */
	char *					fam_Data;		/* Where to read/write the data */
	int						fam_Size;		/* How much data to write */

	long int				fam_Offset;		/* The seek offset */
	int						fam_Mode;		/* The seek mode */

	int						fam_Arg;		/* Whether or not this file should
											   be set non-blocking or use
											   asynchronous I/O */

	struct FileInfoBlock *	fam_FileInfo;
	struct MsgPort *		fam_FileSystem;

	int						fam_Error;		/* Error code, if any... */
};

/****************************************************************************/

/* The file action function for buffered files. */
typedef int (*file_action_iob_t)(struct iob * iob,struct file_action_message * fam);

/****************************************************************************/

/* The three file buffering modes supported. */
#define IOBF_BUFFER_MODE_FULL	_IOFBF	/* Buffer is flushed when it fills up */
#define IOBF_BUFFER_MODE_LINE	_IOLBF	/* Buffer is flushed when it fills up
										   or when a line end character is sent
										   to it */
#define IOBF_BUFFER_MODE_NONE	_IONBF	/* The buffer is never used */

/****************************************************************************/

/* CAREFUL: these mask/flag definitions must match the public definitions
            in <stdio.h> or there will be trouble! */

#define IOBF_BUFFER_MODE	3		/* This is actually a mask which covers
									   the first two bits, both of which
									   contain the buffering mode to use. */
#define IOBF_EOF_REACHED	(1<<2)	/* If this flag is set, then the end of
									   the file was reached while reading
									   from it. */
#define IOBF_READ			(1<<3)	/* If this flag is set then data can
									   be read from this file. */
#define IOBF_WRITE			(1<<4)	/* If this flag is set then data can be
									   written to this file. */
#define IOBF_IN_USE			(1<<5)	/* If this flag is set then this file
									   is not available for allocation */
#define IOBF_ERROR			(1<<6)	/* An error was generated by an I/O
									   operation. */
#define IOBF_TEMP			(1<<7)	/* If this flag is set then the file will
									   be deleted after it has been closed */
#define IOBF_NO_NUL			(1<<8)	/* Don't NUL-terminate strings written to
									   this stream. */
#define IOBF_INTERNAL		(1<<9)	/* Not a regular file, but a buffered
									   file handle made up on the spot in
									   vsprintf(), etc. */

/****************************************************************************/

/* Each file handle is represented by the following structure. Note that this
   must match the layout of the public 'FILE' structure in <stdio.h> or
   things will take a turn for the worse! */
struct iob
{
	ULONG				iob_Flags;				/* Properties and options
												   associated with this file */

	UBYTE *				iob_Buffer;				/* Points to the file buffer */
	LONG				iob_BufferSize;			/* Size of the buffer in bytes */
	LONG				iob_BufferPosition;		/* Current read position
												   in the buffer (grows when any
												   data is read from the buffer) */
	LONG				iob_BufferReadBytes;	/* Number of bytes available for
												   reading (shrinks when any data
												   is read from the buffer) */
	LONG				iob_BufferWriteBytes;	/* Number of bytes written to the
												   buffer which still need to be
												   flushed to disk (grows when any
												   data is written to the buffer) */

	/************************************************************************/
	/* Public portion ends here                                             */
	/************************************************************************/

	file_action_iob_t	iob_Action;				/* The function to invoke for file
												   operations, such as read,
												   write and seek. */

	int					iob_SlotNumber;			/* Points back to the iob table
												   entry number. */

	int					iob_Descriptor;			/* Associated file descriptor */

	STRPTR				iob_String;				/* Alternative source of data;
												   a pointer to a string */
	LONG				iob_StringSize;			/* Number of bytes that may be
												   stored in the string */
	LONG				iob_StringPosition;		/* Current read/write position
												   in the string */
	LONG				iob_StringLength;		/* Number of characters stored
												   in the string */

	char *				iob_File;				/* For access tracking with the
												   memory allocator. */
	int					iob_Line;

	APTR				iob_CustomBuffer;		/* A custom buffer allocated
												   by setvbuf() */

	char *				iob_TempFileName;		/* If this is a temporary
												   file, this is its name */
	BPTR				iob_TempFileLock;		/* The directory in which this
												   temporary file is stored */

	UBYTE				iob_SingleByte;			/* Fall-back buffer for 'unbuffered'
												   files */

	struct SignalSemaphore * iob_Lock;			/* For thread locking */
};

/****************************************************************************/

#define __getc(f) \
	((((struct iob *)(f))->iob_BufferPosition < ((struct iob *)(f))->iob_BufferReadBytes) ? \
	  ((struct iob *)(f))->iob_Buffer[((struct iob *)(f))->iob_BufferPosition++] : \
	  __fgetc((FILE *)(f)))

#define __putc(c,f,m) \
	(((((struct iob *)(f))->iob_BufferWriteBytes < ((struct iob *)(f))->iob_BufferSize)) ? \
	  (((struct iob *)(f))->iob_Buffer[((struct iob *)(f))->iob_BufferWriteBytes++] = (c), \
	  (((m) == IOBF_BUFFER_MODE_LINE && \
	  ((struct iob *)(f))->iob_Buffer[((struct iob *)(f))->iob_BufferWriteBytes-1] == '\n') ? \
	   __flush(f) : \
	   (((struct iob *)(f))->iob_Buffer[((struct iob *)(f))->iob_BufferWriteBytes-1]))) : \
	  __fputc((c),(f),(m)))

#define __putc_fully_buffered(c,f) \
	(((((struct iob *)(f))->iob_BufferWriteBytes < ((struct iob *)(f))->iob_BufferSize)) ? \
	  (((struct iob *)(f))->iob_Buffer[((struct iob *)(f))->iob_BufferWriteBytes++] = (c)) : \
	  __fputc((c),(f),IOBF_BUFFER_MODE_FULL))

#define __putc_line_buffered(c,f) \
	(((((struct iob *)(f))->iob_BufferWriteBytes < ((struct iob *)(f))->iob_BufferSize)) ? \
	  (((struct iob *)(f))->iob_Buffer[((struct iob *)(f))->iob_BufferWriteBytes++] = (c), \
	  (((c) == '\n') ? \
	   __flush(f) : \
	   ((c)))) : \
	  __fputc((c),(f),IOBF_BUFFER_MODE_LINE))

/****************************************************************************/

#define FDF_READ				(1UL<<0)	/* Data can be read from this file */
#define FDF_WRITE				(1UL<<1)	/* Data can be written to this file */
#define FDF_APPEND				(1UL<<2)	/* Before any data is written to it,
											   the file position must be set to the
											   end of the file */
#define FDF_NO_CLOSE			(1UL<<3)	/* Never close this file */
#define FDF_NON_BLOCKING		(1UL<<4)	/* File was switched into non-blocking
											   mode (console streams only) */
#define FDF_IS_SOCKET			(1UL<<5)	/* This is not a disk file but a socket */
#define FDF_IS_LOCKED			(1UL<<6)	/* This file has an advisory record lock set */
#define FDF_IN_USE				(1UL<<7)	/* This file is in use */
#define FDF_CREATED				(1UL<<8)	/* This file was newly created and may need
											   to have its protection bits updated after
											   it has been closed */
#define FDF_CACHE_POSITION		(1UL<<9)	/* Cache the file position. */
#define FDF_ASYNC_IO			(1UL<<10)	/* File was switched into asynchronous I/O
											   mode (sockets only). */
#define FDF_IS_INTERACTIVE		(1UL<<11)	/* File is attached to a console window or
											   something like it. */
#define FDF_STDIO				(1UL<<12)	/* File is to be attached to one of the
											   standard input/output/error streams. */
#define FDF_TERMIOS				(1UL<<13)	/* File is under termios control.
											   FDF_IS_INTERACTIVE should also be set. */
/****************************************************************************/

/* The file action function for unbuffered files. */
typedef int (*file_action_fd_t)(struct fd * fd,struct file_action_message * fam);

/****************************************************************************/

/* Function to be called before a file descriptor is "closed". */
typedef void (*fd_cleanup_t)(struct fd * fd);

/****************************************************************************/

struct fd
{
	file_action_fd_t	fd_Action;			/* Function to invoke to perform actions */
	ULONG				fd_Flags;			/* File properties */
	struct fd *			fd_Original;		/* NULL if this is not a dup()ed file
											   descriptor; points to original
											   descriptor if non-NULL */
	struct fd *			fd_NextLink;		/* Points to next duplicate of this
											   file descriptor; NULL for none */
	BPTR				fd_DefaultFile;		/* A dos.library file handle */
	ULONG				fd_Position;		/* Cached file position (seek offset). */
	fd_cleanup_t		fd_Cleanup;			/* Cleanup function, if any. */

	struct SignalSemaphore * fd_Lock;		/* For thread locking */

	void *				fd_Aux;				/* Auxilliary data for "special" files,
											   e.g. termios support. */
};

/****************************************************************************/

/* Files and directories to be unlinked when the program exits. */
struct UnlinkNode
{
	struct MinNode	uln_MinNode;

	BPTR			uln_Lock;		/* Lock on the directory to which the
									   name is relative */
	STRPTR			uln_Name;		/* The name of the file to remove */
};

/****************************************************************************/

struct bcpl_name
{
	unsigned char name[256];
};

/****************************************************************************/

extern struct WBStartup * NOCOMMON __WBenchMsg;

/****************************************************************************/

/* The file handle table. */
extern struct iob ** NOCOMMON	__iob;
extern int NOCOMMON				__num_iob;

/****************************************************************************/

/* The file descriptor table. */
extern struct fd ** NOCOMMON	__fd;
extern int NOCOMMON				__num_fd;

/****************************************************************************/

extern const char * NOCOMMON __file_lock_semaphore_name;

/****************************************************************************/

extern BOOL NOCOMMON __no_standard_io;

/****************************************************************************/

/*extern int __iob_write_buffer_is_full(struct iob * file);*/

#define __iob_write_buffer_is_full(file) \
	(((struct iob *)file)->iob_BufferSize > 0 && \
	((struct iob *)file)->iob_BufferWriteBytes == ((struct iob *)file)->iob_BufferSize)

/****************************************************************************/

#define __iob_write_buffer_is_valid(file) \
	(((struct iob *)file)->iob_BufferWriteBytes > 0)

/****************************************************************************/

#define __iob_num_unwritten_bytes(file) \
	(((struct iob *)file)->iob_BufferWriteBytes)

/****************************************************************************/

/*extern int __iob_read_buffer_is_empty(struct iob * file);*/

#define __iob_read_buffer_is_empty(file) \
	(((struct iob *)file)->iob_BufferReadBytes == 0 || \
	((struct iob *)file)->iob_BufferPosition == ((struct iob *)file)->iob_BufferReadBytes)

/****************************************************************************/

#define __iob_num_unread_bytes(file) \
	((((struct iob *)file)->iob_BufferReadBytes > 0 && \
	 ((struct iob *)file)->iob_BufferPosition < ((struct iob *)file)->iob_BufferReadBytes) \
	  ? (((struct iob *)file)->iob_BufferReadBytes - ((struct iob *)file)->iob_BufferPosition) \
	  : 0)

/****************************************************************************/

#define __iob_read_buffer_is_valid(file) \
	(((struct iob *)file)->iob_BufferReadBytes > 0)

/****************************************************************************/

#define __fd_is_aliased(fd) \
	((fd)->fd_Original != NULL || (fd)->fd_NextLink != NULL)

/****************************************************************************/

#if defined(__THREAD_SAFE)

/****************************************************************************/

extern void	__stdio_lock(void);
extern void	__stdio_unlock(void);
extern void	__stdio_lock_exit(void);
extern int	__stdio_lock_init(void);

/****************************************************************************/

extern void __fd_lock(struct fd *fd);
extern void __fd_unlock(struct fd *fd);

/****************************************************************************/

#else

/****************************************************************************/

#define __stdio_lock()		((void)0)
#define __stdio_unlock()	((void)0)
#define __stdio_lock_exit()	((void)0)
#define __stdio_lock_init()	(0)

/****************************************************************************/

#define __fd_lock(fd)		((void)0)
#define __fd_unlock(fd)		((void)0)

/****************************************************************************/

#endif /* __THREAD_SAFE */

/****************************************************************************/

#ifndef _STDIO_PROTOS_H
#include "stdio_protos.h"
#endif /* _STDIO_PROTOS_H */

/****************************************************************************/

#endif /* _STDIO_HEADERS_H */
