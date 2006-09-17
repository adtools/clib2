/*
 * $Id: unistd_common_pathconf.c,v 1.2 2006-09-17 17:53:15 obarthel Exp $
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

#ifndef _STDLIB_NULL_POINTER_CHECK_H
#include "stdlib_null_pointer_check.h"
#endif /* _STDLIB_NULL_POINTER_CHECK_H */

/****************************************************************************/

#ifndef _UNISTD_HEADERS_H
#include "unistd_headers.h"
#endif /* _UNISTD_HEADERS_H */

/****************************************************************************/

/* The following is not part of the ISO 'C' (1994) standard. */

/****************************************************************************/

#ifndef ID_CON
#define ID_CON (0x434F4E00L) /* 'CON\0' */
#endif /* ID_CON */

#ifndef ID_RAWCON
#define ID_RAWCON (0x52415700L) /* 'RAW\0' */
#endif /* ID_RAWCON */

/****************************************************************************/

struct fs_info
{
	ULONG	dos_type;
	int		name_max,path_max;
	int		link_max,symlink_max;
	int		file_size_bits;
};

/****************************************************************************/

static const struct fs_info fs_info[] =
{
	{ID_NO_DISK_PRESENT,	0,		0,		0,	0,	0},		/* No disk */
	{ID_UNREADABLE_DISK,	0,		0,		0,	0,	0},		/* BAD\0 */
	{ID_BUSY_DISK,			0,		0,		0,	0,	0},		/* BUSY - Diskchange in progress? */
	{ID_CON,				0,		0,		0,	0,	0},		/* CON\0 - Not really a disk. */
	{ID_RAWCON,				0,		0,		0,	0,	0},		/* RAW\0 - Not really a disk. */
	{ID_NOT_REALLY_DOS,		0,		0,		0,	0,	0},		/* Unreadable disk. */
	{ID_KICKSTART_DISK,		0,		0,		0,	0,	0},		/* Kickstart disk. */
	{ID_MSDOS_DISK,			8,		31,		0,	0,	30},	/* MSDOS - TODO: Check what the limits really are. */
	{ID_DOS_DISK,			30,		254,	-1,	-1,	31},	/* DOS\0 = OFS */
	{ID_FFS_DISK,			30,		254,	-1,	-1,	31},	/* DOS\1 = FFS */
	{ID_INTER_DOS_DISK,		30,		254,	-1,	-1,	31},	/* DOS\2 = OFS INTL */
	{ID_INTER_FFS_DISK,		30,		254,	-1,	-1,	31},	/* DOS\3 = FFS INTL */
	{ID_FASTDIR_DOS_DISK,	30,		254,	-1,	-1,	31},	/* DOS\4 = OFS DC */
	{ID_FASTDIR_FFS_DISK,	30,		254,	-1,	-1,	31},	/* DOS\5 = FFS DC */
	{ID_LONGNAME_DOS_DISK,	107,	8192,	-1,	-1,	31},	/* DOS\6 = OFS LONGNAMES */
	{ID_LONGNAME_FFS_DISK,	107,	8192,	-1,	-1,	63},	/* DOS\7 = FFS LONGNAMES */
	{0x53465300,			107,	8192,	0,	-1,	31},	/* SFS\0 = Smartfilesystem */
	{0x53465302,			107,	8192,	0,	-1,	63},	/* SFS\2 = Smartfilesystem2 */
	{0x50465300,			30,		254,	0,	0,	31},	/* PFS\0 = Professional File System */
	{0x46545854,			0,		0,		0,	0,	0},		/* FTXT - Textclip device */
};

/****************************************************************************/

long
__pathconf(struct MsgPort *port,int name)
{
	ULONG dos_type = 0;
	size_t fs_index = 0;
	long ret = -1;

	if(port != NULL)
	{
		D_S(struct InfoData,id);

		if(DoPkt(port,ACTION_IS_FILESYSTEM,0,0,0,0,0) == DOSFALSE) /* Not a filesystem. */
		{
			SHOWMSG("Not a filesystem.");

			__set_errno(ENODEV);
			goto out;
		}

		if(DoPkt(port,ACTION_DISK_INFO,MKBADDR(id),0,0,0,0))	/* Managed to obtain disk info. */
		{
			size_t i;

			switch(id->id_DiskState)
			{
				case ID_VALIDATING:	/* Consider this an error condition? */
				case ID_WRITE_PROTECTED:
				case ID_VALIDATED:

					dos_type = id->id_DiskType;

					/* See if we know anything about this file-system */
					for(i = 0 ; i < NUM_ENTRIES(fs_info) ; i++)
					{
						if(dos_type == fs_info[i].dos_type)
						{
							fs_index = i;
							break;
						}
					}

					break;

				default:

					SHOWMSG("Invalid disk state.");
					break;
			}
		}
		else
		{
			/* Treat error as no disk present by having default fs_index=0 */
			SHOWMSG("Unable to query DISK_INFO");
		}
	}

	switch(name)
	{
		case _PC_FILESIZEBITS:

			ret = fs_info[fs_index].file_size_bits;
			break;

		case _PC_LINK_MAX:

			ret = fs_info[fs_index].link_max;
			break;

		case _PC_MAX_CANON:

			ret = 510;	/* I could not find any documentation regarding this. */
			break;

		case _PC_MAX_INPUT:

			#if defined(__amigaos4__)
			{
				uint32 Bufsize;
				struct TagItem TagList[2]=
				{
					{DC_FHBufferR,	&Bufsize},
					{TAG_DONE,		0}
				};

				DosControl(TagList);
				ret = Bufsize;	/* Default is 2048 bytes. */
			}
			#else
			{
				ret = 204;
			}
			#endif	/* __amigaos4__ */

			break;

		case _PC_NAME_MAX:

			ret = fs_info[fs_index].name_max;
			break;

		case _PC_PATH_MAX:

			ret = fs_info[fs_index].path_max;
			break;

		case _PC_PIPE_BUF:

			ret = 512;	/* One buffer. The PIPE: device usually has 8 of these. */
			break;

		case _PC_XATTR_ENABLED:

			ret = 0;
			break;

		case _PC_XATTR_EXISTS:

			ret = 0;
			break;

		case _PC_CHOWN_RESTRICTED:

			ret = 0;
			break;

		case _PC_NO_TRUNC:

			ret = 0;
			break;

		case _PC_VDISABLE:

			ret = 0;	/* TODO: Implement this in the termios emulation. */
			break;

		case _PC_ASYNC_IO:

			ret = 0;
			break;

		case _PC_PRIO_IO:

			ret = 0;
			break;

		case _PC_SYNC_IO:

			ret = 0;
			break;

		case _PC_SYMLINK_MAX:

			ret = fs_info[fs_index].symlink_max;
			break;

		case _PC_DOSTYPE:	/* Amiga-specific extension. */

			ret = dos_type;
			break;

		default:

			SHOWMSG("Invalid option name");
			__set_errno(EINVAL);
			goto out;
	}

 out:

	return(ret);
}
