/*
 * $Id: unistd.h,v 1.1.1.1 2004-07-26 16:32:56 obarthel Exp $
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

#ifndef _UNISTD_H
#define _UNISTD_H

/****************************************************************************/

/* The following is not part of the ISO 'C' standard. */

/****************************************************************************/

#ifdef __cplusplus
extern "C" {
#endif /* __cplusplus */

/****************************************************************************/

#ifndef _FCNTL_H
#include <fcntl.h>
#endif /* _FCNTL_H */

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
extern int getopt(int argc, char * argv[], char *opts);
extern pid_t getpid(void);
extern char *realpath(const char *file_name, char *resolved_name);

/****************************************************************************/

extern char * getcwd(char * buffer, size_t buffer_size);

/* This is the version for use with memory debugging; do not call
   it directly! */
extern char * __getcwd(char * buffer,size_t buffer_size,const char *file,int line);

#ifdef __MEM_DEBUG
#define getcwd(buffer,buffer_size) __getcwd((buffer),(buffer_size),__FILE__,__LINE__)
#endif /* __MEM_DEBUG */

/****************************************************************************/

/*
 * The following may have been predefined by local TCP/IP header files. We
 * don't want to be any trouble...
 */

#if !defined(FD_SET) && !defined(__NO_NET_API)

/****************************************************************************/

#ifndef _STRING_H
#include <string.h>
#endif /* _STRING_H */

#ifndef _STDLIB_H
#include <stdlib.h>
#endif /* _STDLIB_H */

#ifndef _SYS_TIME_H
#include <sys/time.h>
#endif /* _SYS_TIME_H */

#ifndef _SYS_TYPES_H
#include <sys/types.h>
#endif /* _SYS_TYPES_H */

/****************************************************************************/

/*
 * select() uses bit masks of file descriptors in longs. These macros
 * manipulate such bit fields.
 *
 * FD_SETSIZE may be defined by the user, but the default here should
 * be enough for most uses.
 */
#ifndef	FD_SETSIZE
#define	FD_SETSIZE 256
#endif

typedef	struct
{
	unsigned long bits[(FD_SETSIZE + 31) / 32];
} fd_set;

#define	FD_SET(n,p)		((void)((p)->bits[((unsigned long)n) >> 5] |=  (1UL << (((unsigned long)n) & 31))))
#define	FD_CLR(n,p)		((void)((p)->bits[((unsigned long)n) >> 5] &= ~(1UL << (((unsigned long)n) & 31))))
#define	FD_ISSET(n,p)	(((p)->bits[((unsigned long)n) >> 5] & (1UL << (((unsigned long)n) & 31))) != 0)
#define	FD_COPY(f,t)	((void)memmove(t,f,sizeof(*(f))))
#define	FD_ZERO(p)		((void)memset(p,0,sizeof(*(p))))

/****************************************************************************/

/* Forward declarations for below... */
struct hostent;
struct netent;
struct protoent;
struct servent;
struct passwd;
struct msghdr;
struct sockaddr;

/****************************************************************************/

/*
 * You might want to have <netinet/in.h> included in place of
 * this local definition.
 */
#if defined(__USE_NETINET_IN_H)

#include <netinet/in.h>

#else

/*
 * These two symbols are typically defined by <netinet/in.h>, which also
 * happens to define 'struct in_addr'. We don't want to redefine it.
 */
#if !defined(_NETINET_IN_H) && !defined(IPPROTO_IP)

/* Internet address (a structure for historical reasons) */
struct in_addr
{
	unsigned long s_addr;
};

#endif /* !_NETINET_IN_H && !IPPROTO_IP */

#endif /* __USE_NETINET_IN_H */

/****************************************************************************/

extern int accept(int sockfd,struct sockaddr *cliaddr,int *addrlen);
extern int bind(int sockfd,struct sockaddr *name,int namelen);
extern int connect(int sockfd,struct sockaddr *name,int namelen);
extern struct hostent * gethostbyaddr(const char *addr, int len, int type);
extern struct hostent * gethostbyname(const char *name);
extern int gethostname(const char *name, int namelen);
extern struct netent * getnetbyname(const char *name);
extern int getpeername(int sockfd,struct sockaddr *name,int *namelen);
extern int getsockname(int sockfd,struct sockaddr *name,int *namelen);
extern int getsockopt(int sockfd,int level,int optname,void *optval,int *optlen);
extern unsigned long inet_addr(const char *addr);
extern char * inet_ntoa(struct in_addr in);
extern int ioctl(int fd,unsigned long request, ... /* char *arg */);
extern int listen(int sockfd,int backlog);
extern int recv(int fd,void *buff,size_t nbytes,int flags);
extern int recvfrom(int sockfd,void *buff,int len,int flags,struct sockaddr *from,int *fromlen);
extern int recvmsg(int socket,struct msghdr *msg,int flags);
extern int select(int num_fds,fd_set *read_fds,fd_set *write_fds,fd_set *except_fds,struct timeval *timeout);
extern int send(int fd,void *buff,size_t nbytes,int flags);
extern int sendmsg(int socket,struct msghdr *msg,int flags);
extern int sendto(int sockfd,void *buff,int len,int flags,struct sockaddr *to,int tolen);
extern int setsockopt(int sockfd,int level,int optname,void *optval,int optlen);
extern int shutdown(int socket, int how);
extern int socket(int domain,int type,int protocol);
extern long gethostid(void);
extern struct netent * getnetbyaddr(long net,int type);
extern struct servent * getservbyname(const char *name, const char *proto);
extern struct servent * getservbyport(int port, const char *proto);
extern struct protoent * getprotobyname(const char *name);
extern struct protoent * getprotobynumber(int proto);
extern int inet_aton(const char *cp, struct in_addr *addr);
extern unsigned long inet_lnaof(struct in_addr in);
extern struct in_addr inet_makeaddr(int net,int host);
extern unsigned long inet_netof(struct in_addr in);
extern unsigned long inet_network(const char *cp);

/****************************************************************************/

extern char *crypt(const char *key, const char *salt);
extern gid_t getegid(void);
extern uid_t geteuid(void);
extern gid_t getgid(void);
extern struct group *getgrgid(gid_t gid);
extern struct group *getgrnam(const char *name);
extern int getgroups(int ngroups, gid_t *groups);
extern char *getpass(const char *prompt);
extern struct passwd *getpwnam(const char *name);
extern struct passwd *getpwuid(uid_t uid);
extern uid_t getuid(void);
extern int initgroups(const char *name, gid_t basegroup);
extern int setegid(gid_t g);
extern int seteuid(uid_t u);
extern int setgid(gid_t id);
extern int setgroups(int ngroups, const gid_t *groups);
extern int setregid(gid_t real, gid_t eff);
extern int setreuid(uid_t real, uid_t eff);
extern long setsid(void);
extern int setuid(uid_t id);
extern void endgrent(void);
extern void endpwent(void);
extern struct group *getgrent(void);
extern struct passwd *getpwent(void);
extern void setgrent(void);
extern void setpwent(void);

/****************************************************************************/

#endif /* !FD_SET && !__NO_NET_API */

/****************************************************************************/

#ifdef __cplusplus
}
#endif /* __cplusplus */

/****************************************************************************/

#endif /* _UNISTD_H */
