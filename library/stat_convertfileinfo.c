/*
 * $Id: stat_convertfileinfo.c,v 1.9 2006-09-20 19:46:57 obarthel Exp $
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

#ifndef _STAT_HEADERS_H
#include "stat_headers.h"
#endif /* _STAT_HEADERS_H */

#ifndef _LOCALE_HEADERS_H
#include "locale_headers.h"
#endif /* _LOCALE_HEADERS_H */

#ifndef _TIME_HEADERS_H
#include "time_headers.h"
#endif /* _TIME_HEADERS_H */

/****************************************************************************/

void
__convert_file_info_to_stat(
	const struct MsgPort *			file_system,
	const struct FileInfoBlock *	fib,
	struct stat *					st)
{
	time_t mtime;
	mode_t mode;
	ULONG flags;

	ENTER();

	assert( fib != NULL && st != NULL );

	SHOWVALUE(fib->fib_DiskKey);
	SHOWVALUE(fib->fib_DirEntryType);
	SHOWSTRING(fib->fib_FileName);
	SHOWVALUE(fib->fib_Protection);
	SHOWVALUE(fib->fib_EntryType);
	SHOWVALUE(fib->fib_Size);
	SHOWVALUE(fib->fib_NumBlocks);
	SHOWVALUE(fib->fib_Date.ds_Days);
	SHOWVALUE(fib->fib_Date.ds_Minute);
	SHOWVALUE(fib->fib_Date.ds_Tick);
	SHOWSTRING(fib->fib_Comment);
	SHOWVALUE(fib->fib_OwnerUID);
	SHOWVALUE(fib->fib_OwnerGID);

	memset(st,0,sizeof(*st));

	flags = fib->fib_Protection ^ (FIBF_READ|FIBF_WRITE|FIBF_EXECUTE|FIBF_DELETE);

	if(fib->fib_DirEntryType == ST_PIPEFILE)
		mode = S_IFIFO;
	else if (fib->fib_DirEntryType == ST_SOCKET)
		mode = S_IFSOCK;
	else if (fib->fib_DirEntryType == ST_CONSOLE || fib->fib_DirEntryType == ST_NIL)
		mode = S_IFCHR;
	else if (fib->fib_DirEntryType < 0)
		mode = S_IFREG;
	else
		mode = S_IFDIR;

	if(FLAG_IS_SET(flags,FIBF_READ))
		SET_FLAG(mode,S_IRUSR);

	if(FLAG_IS_SET(flags,FIBF_WRITE) && FLAG_IS_SET(flags,FIBF_DELETE))
		SET_FLAG(mode,S_IWUSR);

	if(FLAG_IS_SET(flags,FIBF_EXECUTE))
		SET_FLAG(mode,S_IXUSR);

	if(FLAG_IS_SET(flags,FIBF_GRP_READ))
		SET_FLAG(mode,S_IRGRP);

	if(FLAG_IS_SET(flags,FIBF_GRP_WRITE) && FLAG_IS_SET(flags,FIBF_GRP_DELETE))
		SET_FLAG(mode,S_IWGRP);

	if(FLAG_IS_SET(flags,FIBF_GRP_EXECUTE))
		SET_FLAG(mode,S_IXGRP);

	if(FLAG_IS_SET(flags,FIBF_OTR_READ))
		SET_FLAG(mode,S_IROTH);

	if(FLAG_IS_SET(flags,FIBF_OTR_WRITE) && FLAG_IS_SET(flags,FIBF_OTR_DELETE))
		SET_FLAG(mode,S_IWOTH);

	if(FLAG_IS_SET(flags,FIBF_OTR_EXECUTE))
		SET_FLAG(mode,S_IXOTH);

	mtime = __convert_datestamp_to_time(&fib->fib_Date);

	if(fib->fib_DirEntryType < 0)
	{
		st->st_nlink = 1;
		st->st_size  = fib->fib_Size;
	}
	else
	{
		st->st_nlink = 2;
	}

	st->st_ino		= fib->fib_DiskKey;
	st->st_dev		= (dev_t)file_system;
	st->st_mode		= mode;
	st->st_mtime	= mtime;
	st->st_atime	= mtime;
	st->st_ctime	= mtime;
	st->st_uid		= fib->fib_OwnerUID;
	st->st_gid		= fib->fib_OwnerGID;
	st->st_blksize	= 512;
	st->st_blocks	= (st->st_size + st->st_blksize-1) / st->st_blksize;

	SHOWVALUE(st->st_nlink);
	SHOWVALUE(st->st_size);
	SHOWVALUE(st->st_ino);
	SHOWVALUE(st->st_dev);
	SHOWVALUE(st->st_mode);
	SHOWVALUE(st->st_mtime);
	SHOWVALUE(st->st_atime);
	SHOWVALUE(st->st_ctime);
	SHOWVALUE(st->st_uid);
	SHOWVALUE(st->st_gid);
	SHOWVALUE(st->st_blksize);
	SHOWVALUE(st->st_blocks);

	LEAVE();
}
