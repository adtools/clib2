/*
 * $Id: stdio_grow_file.c,v 1.8 2008-09-04 12:07:58 obarthel Exp $
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
#include "stdio_headers.h"
#endif /* _STDIO_HEADERS_H */

/****************************************************************************/

/* The following is not part of the ISO 'C' (1994) standard. */

/****************************************************************************/

/* Seek to the end of a file, then add a certain number of 0 bytes. Note that
   this function will change the current file position! */
int
__grow_file_size(struct fd * fd,int num_bytes)
{
	unsigned char * aligned_buffer;
	unsigned char * buffer;
	struct FileHandle * fh;
	D_S(struct InfoData,id);
	LONG block_size;
	int bytes_written;
	int buffer_size;
	int size;
	LONG seek_position;
	off_t position;
	off_t current_position;
	int alignment_skip;
	int result = ERROR;

	assert( fd != NULL );

	D(("we have to grow the file by %ld bytes",num_bytes));

	block_size = 0;

	PROFILE_OFF();

	assert( FLAG_IS_CLEAR(fd->fd_Flags,FDF_STDIO) );

	fh = BADDR(fd->fd_File);
	if(fh != NULL && fh->fh_Type != NULL && DoPkt(fh->fh_Type,ACTION_DISK_INFO,MKBADDR(id),0,0,0,0))
		block_size = id->id_BytesPerBlock;

	PROFILE_ON();

	if(block_size < 512)
		block_size = 512;

	/* We have to fill up the file with zero bytes.
	 * That data comes from a local buffer. How
	 * large can we make it?
	 */
	buffer_size = 8 * block_size;
	if(buffer_size > num_bytes)
		buffer_size = num_bytes;

	/* Allocate a little more memory than required to allow for
	 * the buffer to be aligned to a cache line boundary.
	 */
	buffer = malloc((size_t)buffer_size + (__cache_line_size-1));
	if(buffer == NULL)
	{
		SHOWMSG("not enough memory for write buffer");

		SetIoErr(ERROR_NO_FREE_STORE);
		goto out;
	}

	/* Align the buffer to a cache line boundary. */
	aligned_buffer = (unsigned char *)(((ULONG)(buffer + (__cache_line_size-1))) & ~(__cache_line_size-1));

	memset(aligned_buffer,0,(size_t)buffer_size);

	PROFILE_OFF();
	seek_position = Seek(fd->fd_File,0,OFFSET_END);
	PROFILE_ON();

	if(seek_position == SEEK_ERROR && IoErr() != OK)
	{
		SHOWMSG("could not move to the end of the file");
		goto out;
	}

	position = (off_t)seek_position;

	PROFILE_OFF();
	seek_position = Seek(fd->fd_File,0,OFFSET_CURRENT);
	PROFILE_ON();

	current_position = (off_t)seek_position;

	/* Try to make the first write access align the file position
	 * to a block offset. Subsequent writes will then access the
	 * file at positions that are multiples of the block size.
	 */
	if(num_bytes > block_size && (current_position % block_size) != 0)
		alignment_skip = block_size - (current_position % block_size);
	else
		alignment_skip = 0;

	while(num_bytes > 0)
	{
		if(__check_abort_enabled)
			__check_abort();

		size = buffer_size;
		if(size > num_bytes)
			size = num_bytes;

		/* If possible, even out the block offset. */
		if(alignment_skip > 0 && size > alignment_skip)
			size = alignment_skip;

		alignment_skip = 0;

		PROFILE_OFF();
		bytes_written = Write(fd->fd_File,aligned_buffer,size);
		PROFILE_ON();

		if(bytes_written != size)
			goto out;

		num_bytes -= size;
	}

	SHOWMSG("all done.");

	result = OK;

 out:

	if(buffer != NULL)
		free(buffer);

	return(result);
}
