/*
 * $Id: unistd.h,v 1.13 2005-07-12 13:35:58 obarthel Exp $
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

#ifndef _UNISTD_H
#define _UNISTD_H

/****************************************************************************/

/* The following is not part of the ISO 'C' (1994) standard. */

/****************************************************************************/

#ifdef __cplusplus
extern "C" {
#endif /* __cplusplus */

/****************************************************************************/

#ifndef _FCNTL_H
#include <fcntl.h>
#endif /* _FCNTL_H */

/****************************************************************************/

/* Endianness: we assume a big endian memory layout (for 68k and PowerPC). */
#define LITTLE_ENDIAN	1234
#define BIG_ENDIAN		4321
#define BYTE_ORDER		BIG_ENDIAN

/****************************************************************************/

#define STDIN_FILENO	0
#define STDOUT_FILENO	1
#define STDERR_FILENO	2

/****************************************************************************/

#define R_OK 0
#define W_OK 1
#define X_OK 2
#define F_OK 4

/****************************************************************************/

extern int opterr;
extern int optind;
extern int optopt;
extern char * optarg;

/****************************************************************************/

extern int isatty(int file_descriptor);
extern int dup(int file_descriptor);
extern int dup2(int file_descriptor1, int file_descriptor2);
extern int access(const char * path_name, int mode);
extern int chown(const char * path_name, uid_t owner, gid_t group);
extern int fchown(int file_descriptor, uid_t owner, gid_t group);
extern int lchown(const char * path_name, uid_t owner, gid_t group);
extern int truncate(const char * path_name, off_t length);
extern int ftruncate(int file_descriptor, off_t length);
extern int link(const char * existing_path,const char * new_path);
extern int unlink(const char * path_name);
extern int symlink(const char * actual_path, const char * symbolic_path);
extern int readlink(const char * path_name, char * buffer, int buffer_size);
extern int chdir(const char * path_name);
extern unsigned int sleep(unsigned int seconds);
extern void usleep(unsigned long microseconds);
extern int getopt(int argc, char * argv[], char *opts);
extern pid_t getpid(void);
extern char *realpath(const char *file_name, char *resolved_name);
extern int fsync(int file_descriptor);
extern int fdatasync(int file_descriptor);
extern char *ttyname(int);
extern int ttyname_r(int file_descriptor,char *name,size_t buflen);
extern int ttyname_t(int,char *,size_t);

extern int profil(unsigned short *buffer, size_t bufSize, size_t offset, unsigned int scale);

/****************************************************************************/

extern char * getcwd(char * buffer, size_t buffer_size);

#ifdef __MEM_DEBUG
extern char * __getcwd(char * buffer,size_t buffer_size,const char *file,int line);

#define getcwd(buffer,buffer_size) __getcwd((buffer),(buffer_size),__FILE__,__LINE__)
#endif /* __MEM_DEBUG */

/****************************************************************************/

/* If desired, also include the networking API, as integrated into
   the clib2 libnet.a library. */
#if defined(__USE_CLIB2_NETLIB) && !defined(_SYS_CLIB2_NET_H)
#include <sys/clib2_net.h>
#endif /* __USE_CLIB2_NETLIB && !_SYS_CLIB2_NET_H */

/****************************************************************************/

#ifdef __cplusplus
}
#endif /* __cplusplus */

/****************************************************************************/

#endif /* _UNISTD_H */
