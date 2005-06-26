/*
 * $Id: mount_convertinfo.c,v 1.4 2005-06-26 11:59:37 tfrieden Exp $
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

#ifndef _MOUNT_HEADERS_H
#include "mount_headers.h"
#endif /* _MOUNT_HEADERS_H */

/****************************************************************************/

#include <limits.h>
#include <string.h>

/****************************************************************************/

void
__convert_info_to_statfs(struct InfoData * id,struct statfs * f)
{
	LONG num_blocks,num_blocks_used,bytes_per_block;

	ENTER();

	assert( id != NULL && f != NULL );

	SHOWVALUE(id->id_NumSoftErrors);
	SHOWVALUE(id->id_UnitNumber);
	SHOWVALUE(id->id_DiskState);
	SHOWVALUE(id->id_NumBlocks);
	SHOWVALUE(id->id_NumBlocksUsed);
	SHOWVALUE(id->id_BytesPerBlock);
	SHOWVALUE(id->id_DiskType);
	SHOWVALUE(id->id_VolumeNode);
	SHOWVALUE(id->id_InUse);

	memset(f,0,sizeof(*f));

	if(id->id_NumBlocks > 0)
	{
		num_blocks		= id->id_NumBlocks;
		num_blocks_used	= id->id_NumBlocksUsed;
	}
	else
	{
		num_blocks		= 1;
		num_blocks_used	= 1;
	}

	if(id->id_BytesPerBlock > 0)
		bytes_per_block = id->id_BytesPerBlock;
	else
		bytes_per_block = 512;

	f->f_bsize	= bytes_per_block;
	f->f_blocks	= num_blocks;
	f->f_bfree	= num_blocks - num_blocks_used;
	f->f_iosize	= f->f_bsize;
	f->f_bavail	= f->f_bfree;
	f->f_ffree	= LONG_MAX;
	f->f_flags	= MNT_NOATIME|MNT_SYMPERM|MNT_LOCAL;

	if(id->id_DiskState != ID_VALIDATED)
		SET_FLAG(f->f_flags,MNT_RDONLY);


	switch (id->id_DiskType)
	{
		case ID_NO_DISK_PRESENT:
			strcpy(f->f_fstypename, "None");
			break;
		case ID_UNREADABLE_DISK:
			strcpy(f->f_fstypename, "NDOS");
			break;
		case ID_BUSY_DISK:
			strcpy(f->f_fstypename, "Busy");
			break;
		case ID_DOS_DISK:
			strcpy(f->f_fstypename, "DOS\\0");
			break;
		case ID_FFS_DISK:
			strcpy(f->f_fstypename, "DOS\\1");
			break;
		case ID_INTER_DOS_DISK:
			strcpy(f->f_fstypename, "DOS\\2");
			break;
		case ID_INTER_FFS_DISK:
			strcpy(f->f_fstypename, "DOS\\3");
			break;
		case ID_FASTDIR_DOS_DISK:
			strcpy(f->f_fstypename, "DOS\\4");
			break;
		case ID_FASTDIR_FFS_DISK:
			strcpy(f->f_fstypename, "DOS\\5");
			break;
		case ID_LONGNAME_DOS_DISK:
			strcpy(f->f_fstypename, "DOS\\6");
			break;
		case ID_LONGNAME_FFS_DISK:
			strcpy(f->f_fstypename, "DOS\\7");
			break;
		case 0x53465300:
			strcpy(f->f_fstypename, "SFS\\0");
			break;
		case ID_NOT_REALLY_DOS:
			strcpy(f->f_fstypename, "NDOS");
			break;
		default:
			strcpy(f->f_fstypename, "Unkown");
			break;
	}

	SHOWVALUE(f->f_bsize);
	SHOWVALUE(f->f_blocks);
	SHOWVALUE(f->f_bfree);
	SHOWVALUE(f->f_iosize);
	SHOWVALUE(f->f_bavail);
	SHOWVALUE(f->f_ffree);
	SHOWVALUE(f->f_flags);

	LEAVE();
}
