/*
 * $Id: socket_headers.h,v 1.5 2005-02-03 16:56:15 obarthel Exp $
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

#ifndef _SOCKET_HEADERS_H
#define _SOCKET_HEADERS_H

/****************************************************************************/

#ifndef _NETINET_IN_H
#include <netinet/in.h>
#endif /* _NETINET_IN_H */

/****************************************************************************/

#ifndef _STDIO_HEADERS_H
#include "stdio_headers.h"
#endif /* _STDIO_HEADERS_H */

/****************************************************************************/

#ifndef _STDLIB_HEADERS_H
#include "stdlib_headers.h"
#endif /* _STDLIB_HEADERS_H */

/****************************************************************************/

extern struct Library * NOCOMMON __SocketBase;

/****************************************************************************/

#if defined(__amigaos4__)

extern struct SocketIFace * NOCOMMON __ISocket;

#endif /* __amigaos4__ */

/****************************************************************************/

extern int NOCOMMON h_errno;

/****************************************************************************/

extern struct fd * __get_socket_descriptor(int socket_descriptor);
extern void __socket_hook_entry(struct Hook * hook,struct fd * fd,struct file_hook_message * message);

/****************************************************************************/

#if defined(__GNUC__)

#if defined(__amigaos4__)

#include <interfaces/bsdsocket.h>

#define __socket(domain, type, protocol) __ISocket->socket(domain , type , protocol) 
#define __bind(sock, name, namelen) __ISocket->bind(sock , name , namelen) 
#define __listen(sock, backlog) __ISocket->listen(sock , backlog) 
#define __accept(sock, addr, addrlen) __ISocket->accept(sock , addr , addrlen) 
#define __connect(sock, name, namelen) __ISocket->connect(sock , name , namelen) 
#define __sendto(sock, buf, len, flags, to, tolen) __ISocket->sendto(sock , buf , len , flags , to , tolen) 
#define __send(sock, buf, len, flags) __ISocket->send(sock , buf , len , flags) 
#define __recvfrom(sock, buf, len, flags, addr, addrlen) __ISocket->recvfrom(sock , buf , len , flags , addr , addrlen) 
#define __recv(sock, buf, len, flags) __ISocket->recv(sock , buf , len , flags) 
#define __shutdown(sock, how) __ISocket->shutdown(sock , how) 
#define __setsockopt(sock, level, optname, optval, optlen) __ISocket->setsockopt(sock , level , optname , optval , optlen) 
#define __getsockopt(sock, level, optname, optval, optlen) __ISocket->getsockopt(sock , level , optname , optval , optlen) 
#define __getsockname(sock, name, namelen) __ISocket->getsockname(sock , name , namelen) 
#define __getpeername(sock, name, namelen) __ISocket->getpeername(sock , name , namelen) 
#define __IoctlSocket(sock, req, argp) __ISocket->IoctlSocket(sock , req , argp) 
#define __CloseSocket(sock) __ISocket->CloseSocket(sock) 
#define __WaitSelect(nfds, read_fds, write_fds, except_fds, timeout, signals) __ISocket->WaitSelect(nfds , read_fds , write_fds , except_fds , timeout , signals) 
#define __SetSocketSignals(int_mask, io_mask, urgent_mask) __ISocket->SetSocketSignals(int_mask , io_mask , urgent_mask) 
#define __getdtablesize() __ISocket->getdtablesize() 
#define __ObtainSocket(id, domain, type, protocol) __ISocket->ObtainSocket(id , domain , type , protocol) 
#define __ReleaseSocket(sock, id) __ISocket->ReleaseSocket(sock , id) 
#define __ReleaseCopyOfSocket(sock, id) __ISocket->ReleaseCopyOfSocket(sock , id) 
#define __Errno() __ISocket->Errno() 
#define __SetErrnoPtr(errno_ptr, size) __ISocket->SetErrnoPtr(errno_ptr , size) 
#define __Inet_NtoA(ip) __ISocket->Inet_NtoA(ip) 
#define __Inet_AtoN(cp, ip) __ISocket->inet_aton(cp, ip)
#define __inet_addr(cp) __ISocket->inet_addr(cp) 
#define __Inet_LnaOf(in) __ISocket->Inet_LnaOf(in) 
#define __Inet_NetOf(in) __ISocket->Inet_NetOf(in) 
#define __Inet_MakeAddr(net, host) __ISocket->Inet_MakeAddr(net , host) 
#define __inet_network(cp) __ISocket->inet_network(cp) 
#define __gethostbyname(name) __ISocket->gethostbyname(name) 
#define __gethostbyaddr(addr, len, type) __ISocket->gethostbyaddr(addr , len , type) 
#define __getnetbyname(name) __ISocket->getnetbyname(name) 
#define __getnetbyaddr(net, type) __ISocket->getnetbyaddr(net , type) 
#define __getservbyname(name, proto) __ISocket->getservbyname(name , proto) 
#define __getservbyport(port, proto) __ISocket->getservbyport(port , proto) 
#define __getprotobyname(name) __ISocket->getprotobyname(name) 
#define __getprotobynumber(proto) __ISocket->getprotobynumber(proto) 
#define __vsyslog(pri, msg, args) __ISocket->vsyslog(pri , msg , args) 
#define __syslog(pri, msg, tag1...) __ISocket->syslog(pri , msg , ## tag1) 
#define __Dup2Socket(old_socket, new_socket) __ISocket->Dup2Socket(old_socket , new_socket) 
#define __sendmsg(sock, msg, flags) __ISocket->sendmsg(sock , msg , flags) 
#define __recvmsg(sock, msg, flags) __ISocket->recvmsg(sock , msg , flags) 
#define __gethostname(name, namelen) __ISocket->gethostname(name , namelen) 
#define __gethostid() __ISocket->gethostid() 
#define __SocketBaseTagList(tags) __ISocket->SocketBaseTagList(tags) 
#define __SocketBaseTags(tag1...) __ISocket->SocketBaseTags(## tag1) 

#else

#define __socket(domain, type, protocol) ({ \
  LONG _socket_domain = (domain); \
  LONG _socket_type = (type); \
  LONG _socket_protocol = (protocol); \
  LONG _socket__re = \
  ({ \
  register struct Library * const __socket__bn __asm("a6") = (struct Library *) (__SocketBase);\
  register LONG __socket__re __asm("d0"); \
  register LONG __socket_domain __asm("d0") = (_socket_domain); \
  register LONG __socket_type __asm("d1") = (_socket_type); \
  register LONG __socket_protocol __asm("d2") = (_socket_protocol); \
  __asm volatile ("jsr a6@(-30:W)" \
  : "=r"(__socket__re) \
  : "r"(__socket__bn), "r"(__socket_domain), "r"(__socket_type), "r"(__socket_protocol)  \
  : "d1", "a0", "a1", "fp0", "fp1", "cc", "memory"); \
  __socket__re; \
  }); \
  _socket__re; \
})

#define __bind(sock, name, namelen) ({ \
  LONG _bind_sock = (sock); \
  struct sockaddr * _bind_name = (name); \
  LONG _bind_namelen = (namelen); \
  LONG _bind__re = \
  ({ \
  register struct Library * const __bind__bn __asm("a6") = (struct Library *) (__SocketBase);\
  register LONG __bind__re __asm("d0"); \
  register LONG __bind_sock __asm("d0") = (_bind_sock); \
  register struct sockaddr * __bind_name __asm("a0") = (_bind_name); \
  register LONG __bind_namelen __asm("d1") = (_bind_namelen); \
  __asm volatile ("jsr a6@(-36:W)" \
  : "=r"(__bind__re) \
  : "r"(__bind__bn), "r"(__bind_sock), "r"(__bind_name), "r"(__bind_namelen)  \
  : "d1", "a0", "a1", "fp0", "fp1", "cc", "memory"); \
  __bind__re; \
  }); \
  _bind__re; \
})

#define __listen(sock, backlog) ({ \
  LONG _listen_sock = (sock); \
  LONG _listen_backlog = (backlog); \
  LONG _listen__re = \
  ({ \
  register struct Library * const __listen__bn __asm("a6") = (struct Library *) (__SocketBase);\
  register LONG __listen__re __asm("d0"); \
  register LONG __listen_sock __asm("d0") = (_listen_sock); \
  register LONG __listen_backlog __asm("d1") = (_listen_backlog); \
  __asm volatile ("jsr a6@(-42:W)" \
  : "=r"(__listen__re) \
  : "r"(__listen__bn), "r"(__listen_sock), "r"(__listen_backlog)  \
  : "d1", "a0", "a1", "fp0", "fp1", "cc", "memory"); \
  __listen__re; \
  }); \
  _listen__re; \
})

#define __accept(sock, addr, addrlen) ({ \
  LONG _accept_sock = (sock); \
  struct sockaddr * _accept_addr = (addr); \
  LONG * _accept_addrlen = (addrlen); \
  LONG _accept__re = \
  ({ \
  register struct Library * const __accept__bn __asm("a6") = (struct Library *) (__SocketBase);\
  register LONG __accept__re __asm("d0"); \
  register LONG __accept_sock __asm("d0") = (_accept_sock); \
  register struct sockaddr * __accept_addr __asm("a0") = (_accept_addr); \
  register LONG * __accept_addrlen __asm("a1") = (_accept_addrlen); \
  __asm volatile ("jsr a6@(-48:W)" \
  : "=r"(__accept__re) \
  : "r"(__accept__bn), "r"(__accept_sock), "r"(__accept_addr), "r"(__accept_addrlen)  \
  : "d1", "a0", "a1", "fp0", "fp1", "cc", "memory"); \
  __accept__re; \
  }); \
  _accept__re; \
})

#define __connect(sock, name, namelen) ({ \
  LONG _connect_sock = (sock); \
  struct sockaddr * _connect_name = (name); \
  LONG _connect_namelen = (namelen); \
  LONG _connect__re = \
  ({ \
  register struct Library * const __connect__bn __asm("a6") = (struct Library *) (__SocketBase);\
  register LONG __connect__re __asm("d0"); \
  register LONG __connect_sock __asm("d0") = (_connect_sock); \
  register struct sockaddr * __connect_name __asm("a0") = (_connect_name); \
  register LONG __connect_namelen __asm("d1") = (_connect_namelen); \
  __asm volatile ("jsr a6@(-54:W)" \
  : "=r"(__connect__re) \
  : "r"(__connect__bn), "r"(__connect_sock), "r"(__connect_name), "r"(__connect_namelen)  \
  : "d1", "a0", "a1", "fp0", "fp1", "cc", "memory"); \
  __connect__re; \
  }); \
  _connect__re; \
})

#define __sendto(sock, buf, len, flags, to, tolen) ({ \
  LONG _sendto_sock = (sock); \
  APTR _sendto_buf = (buf); \
  LONG _sendto_len = (len); \
  LONG _sendto_flags = (flags); \
  struct sockaddr * _sendto_to = (to); \
  LONG _sendto_tolen = (tolen); \
  LONG _sendto__re = \
  ({ \
  register struct Library * const __sendto__bn __asm("a6") = (struct Library *) (__SocketBase);\
  register LONG __sendto__re __asm("d0"); \
  register LONG __sendto_sock __asm("d0") = (_sendto_sock); \
  register APTR __sendto_buf __asm("a0") = (_sendto_buf); \
  register LONG __sendto_len __asm("d1") = (_sendto_len); \
  register LONG __sendto_flags __asm("d2") = (_sendto_flags); \
  register struct sockaddr * __sendto_to __asm("a1") = (_sendto_to); \
  register LONG __sendto_tolen __asm("d3") = (_sendto_tolen); \
  __asm volatile ("jsr a6@(-60:W)" \
  : "=r"(__sendto__re) \
  : "r"(__sendto__bn), "r"(__sendto_sock), "r"(__sendto_buf), "r"(__sendto_len), "r"(__sendto_flags), "r"(__sendto_to), "r"(__sendto_tolen)  \
  : "d1", "a0", "a1", "fp0", "fp1", "cc", "memory"); \
  __sendto__re; \
  }); \
  _sendto__re; \
})

#define __send(sock, buf, len, flags) ({ \
  LONG _send_sock = (sock); \
  APTR _send_buf = (buf); \
  LONG _send_len = (len); \
  LONG _send_flags = (flags); \
  LONG _send__re = \
  ({ \
  register struct Library * const __send__bn __asm("a6") = (struct Library *) (__SocketBase);\
  register LONG __send__re __asm("d0"); \
  register LONG __send_sock __asm("d0") = (_send_sock); \
  register APTR __send_buf __asm("a0") = (_send_buf); \
  register LONG __send_len __asm("d1") = (_send_len); \
  register LONG __send_flags __asm("d2") = (_send_flags); \
  __asm volatile ("jsr a6@(-66:W)" \
  : "=r"(__send__re) \
  : "r"(__send__bn), "r"(__send_sock), "r"(__send_buf), "r"(__send_len), "r"(__send_flags)  \
  : "d1", "a0", "a1", "fp0", "fp1", "cc", "memory"); \
  __send__re; \
  }); \
  _send__re; \
})

#define __recvfrom(sock, buf, len, flags, addr, addrlen) ({ \
  LONG _recvfrom_sock = (sock); \
  APTR _recvfrom_buf = (buf); \
  LONG _recvfrom_len = (len); \
  LONG _recvfrom_flags = (flags); \
  struct sockaddr * _recvfrom_addr = (addr); \
  LONG * _recvfrom_addrlen = (addrlen); \
  LONG _recvfrom__re = \
  ({ \
  register struct Library * const __recvfrom__bn __asm("a6") = (struct Library *) (__SocketBase);\
  register LONG __recvfrom__re __asm("d0"); \
  register LONG __recvfrom_sock __asm("d0") = (_recvfrom_sock); \
  register APTR __recvfrom_buf __asm("a0") = (_recvfrom_buf); \
  register LONG __recvfrom_len __asm("d1") = (_recvfrom_len); \
  register LONG __recvfrom_flags __asm("d2") = (_recvfrom_flags); \
  register struct sockaddr * __recvfrom_addr __asm("a1") = (_recvfrom_addr); \
  register LONG * __recvfrom_addrlen __asm("a2") = (_recvfrom_addrlen); \
  __asm volatile ("jsr a6@(-72:W)" \
  : "=r"(__recvfrom__re) \
  : "r"(__recvfrom__bn), "r"(__recvfrom_sock), "r"(__recvfrom_buf), "r"(__recvfrom_len), "r"(__recvfrom_flags), "r"(__recvfrom_addr), "r"(__recvfrom_addrlen)  \
  : "d1", "a0", "a1", "fp0", "fp1", "cc", "memory"); \
  __recvfrom__re; \
  }); \
  _recvfrom__re; \
})

#define __recv(sock, buf, len, flags) ({ \
  LONG _recv_sock = (sock); \
  APTR _recv_buf = (buf); \
  LONG _recv_len = (len); \
  LONG _recv_flags = (flags); \
  LONG _recv__re = \
  ({ \
  register struct Library * const __recv__bn __asm("a6") = (struct Library *) (__SocketBase);\
  register LONG __recv__re __asm("d0"); \
  register LONG __recv_sock __asm("d0") = (_recv_sock); \
  register APTR __recv_buf __asm("a0") = (_recv_buf); \
  register LONG __recv_len __asm("d1") = (_recv_len); \
  register LONG __recv_flags __asm("d2") = (_recv_flags); \
  __asm volatile ("jsr a6@(-78:W)" \
  : "=r"(__recv__re) \
  : "r"(__recv__bn), "r"(__recv_sock), "r"(__recv_buf), "r"(__recv_len), "r"(__recv_flags)  \
  : "d1", "a0", "a1", "fp0", "fp1", "cc", "memory"); \
  __recv__re; \
  }); \
  _recv__re; \
})

#define __shutdown(sock, how) ({ \
  LONG _shutdown_sock = (sock); \
  LONG _shutdown_how = (how); \
  LONG _shutdown__re = \
  ({ \
  register struct Library * const __shutdown__bn __asm("a6") = (struct Library *) (__SocketBase);\
  register LONG __shutdown__re __asm("d0"); \
  register LONG __shutdown_sock __asm("d0") = (_shutdown_sock); \
  register LONG __shutdown_how __asm("d1") = (_shutdown_how); \
  __asm volatile ("jsr a6@(-84:W)" \
  : "=r"(__shutdown__re) \
  : "r"(__shutdown__bn), "r"(__shutdown_sock), "r"(__shutdown_how)  \
  : "d1", "a0", "a1", "fp0", "fp1", "cc", "memory"); \
  __shutdown__re; \
  }); \
  _shutdown__re; \
})

#define __setsockopt(sock, level, optname, optval, optlen) ({ \
  LONG _setsockopt_sock = (sock); \
  LONG _setsockopt_level = (level); \
  LONG _setsockopt_optname = (optname); \
  APTR _setsockopt_optval = (optval); \
  LONG _setsockopt_optlen = (optlen); \
  LONG _setsockopt__re = \
  ({ \
  register struct Library * const __setsockopt__bn __asm("a6") = (struct Library *) (__SocketBase);\
  register LONG __setsockopt__re __asm("d0"); \
  register LONG __setsockopt_sock __asm("d0") = (_setsockopt_sock); \
  register LONG __setsockopt_level __asm("d1") = (_setsockopt_level); \
  register LONG __setsockopt_optname __asm("d2") = (_setsockopt_optname); \
  register APTR __setsockopt_optval __asm("a0") = (_setsockopt_optval); \
  register LONG __setsockopt_optlen __asm("d3") = (_setsockopt_optlen); \
  __asm volatile ("jsr a6@(-90:W)" \
  : "=r"(__setsockopt__re) \
  : "r"(__setsockopt__bn), "r"(__setsockopt_sock), "r"(__setsockopt_level), "r"(__setsockopt_optname), "r"(__setsockopt_optval), "r"(__setsockopt_optlen)  \
  : "d1", "a0", "a1", "fp0", "fp1", "cc", "memory"); \
  __setsockopt__re; \
  }); \
  _setsockopt__re; \
})

#define __getsockopt(sock, level, optname, optval, optlen) ({ \
  LONG _getsockopt_sock = (sock); \
  LONG _getsockopt_level = (level); \
  LONG _getsockopt_optname = (optname); \
  APTR _getsockopt_optval = (optval); \
  LONG * _getsockopt_optlen = (optlen); \
  LONG _getsockopt__re = \
  ({ \
  register struct Library * const __getsockopt__bn __asm("a6") = (struct Library *) (__SocketBase);\
  register LONG __getsockopt__re __asm("d0"); \
  register LONG __getsockopt_sock __asm("d0") = (_getsockopt_sock); \
  register LONG __getsockopt_level __asm("d1") = (_getsockopt_level); \
  register LONG __getsockopt_optname __asm("d2") = (_getsockopt_optname); \
  register APTR __getsockopt_optval __asm("a0") = (_getsockopt_optval); \
  register LONG * __getsockopt_optlen __asm("a1") = (_getsockopt_optlen); \
  __asm volatile ("jsr a6@(-96:W)" \
  : "=r"(__getsockopt__re) \
  : "r"(__getsockopt__bn), "r"(__getsockopt_sock), "r"(__getsockopt_level), "r"(__getsockopt_optname), "r"(__getsockopt_optval), "r"(__getsockopt_optlen)  \
  : "d1", "a0", "a1", "fp0", "fp1", "cc", "memory"); \
  __getsockopt__re; \
  }); \
  _getsockopt__re; \
})

#define __getsockname(sock, name, namelen) ({ \
  LONG _getsockname_sock = (sock); \
  struct sockaddr * _getsockname_name = (name); \
  LONG * _getsockname_namelen = (namelen); \
  LONG _getsockname__re = \
  ({ \
  register struct Library * const __getsockname__bn __asm("a6") = (struct Library *) (__SocketBase);\
  register LONG __getsockname__re __asm("d0"); \
  register LONG __getsockname_sock __asm("d0") = (_getsockname_sock); \
  register struct sockaddr * __getsockname_name __asm("a0") = (_getsockname_name); \
  register LONG * __getsockname_namelen __asm("a1") = (_getsockname_namelen); \
  __asm volatile ("jsr a6@(-102:W)" \
  : "=r"(__getsockname__re) \
  : "r"(__getsockname__bn), "r"(__getsockname_sock), "r"(__getsockname_name), "r"(__getsockname_namelen)  \
  : "d1", "a0", "a1", "fp0", "fp1", "cc", "memory"); \
  __getsockname__re; \
  }); \
  _getsockname__re; \
})

#define __getpeername(sock, name, namelen) ({ \
  LONG _getpeername_sock = (sock); \
  struct sockaddr * _getpeername_name = (name); \
  LONG * _getpeername_namelen = (namelen); \
  LONG _getpeername__re = \
  ({ \
  register struct Library * const __getpeername__bn __asm("a6") = (struct Library *) (__SocketBase);\
  register LONG __getpeername__re __asm("d0"); \
  register LONG __getpeername_sock __asm("d0") = (_getpeername_sock); \
  register struct sockaddr * __getpeername_name __asm("a0") = (_getpeername_name); \
  register LONG * __getpeername_namelen __asm("a1") = (_getpeername_namelen); \
  __asm volatile ("jsr a6@(-108:W)" \
  : "=r"(__getpeername__re) \
  : "r"(__getpeername__bn), "r"(__getpeername_sock), "r"(__getpeername_name), "r"(__getpeername_namelen)  \
  : "d1", "a0", "a1", "fp0", "fp1", "cc", "memory"); \
  __getpeername__re; \
  }); \
  _getpeername__re; \
})

#define __IoctlSocket(sock, req, argp) ({ \
  LONG _IoctlSocket_sock = (sock); \
  ULONG _IoctlSocket_req = (req); \
  APTR _IoctlSocket_argp = (argp); \
  LONG _IoctlSocket__re = \
  ({ \
  register struct Library * const __IoctlSocket__bn __asm("a6") = (struct Library *) (__SocketBase);\
  register LONG __IoctlSocket__re __asm("d0"); \
  register LONG __IoctlSocket_sock __asm("d0") = (_IoctlSocket_sock); \
  register ULONG __IoctlSocket_req __asm("d1") = (_IoctlSocket_req); \
  register APTR __IoctlSocket_argp __asm("a0") = (_IoctlSocket_argp); \
  __asm volatile ("jsr a6@(-114:W)" \
  : "=r"(__IoctlSocket__re) \
  : "r"(__IoctlSocket__bn), "r"(__IoctlSocket_sock), "r"(__IoctlSocket_req), "r"(__IoctlSocket_argp)  \
  : "d1", "a0", "a1", "fp0", "fp1", "cc", "memory"); \
  __IoctlSocket__re; \
  }); \
  _IoctlSocket__re; \
})

#define __CloseSocket(sock) ({ \
  LONG _CloseSocket_sock = (sock); \
  LONG _CloseSocket__re = \
  ({ \
  register struct Library * const __CloseSocket__bn __asm("a6") = (struct Library *) (__SocketBase);\
  register LONG __CloseSocket__re __asm("d0"); \
  register LONG __CloseSocket_sock __asm("d0") = (_CloseSocket_sock); \
  __asm volatile ("jsr a6@(-120:W)" \
  : "=r"(__CloseSocket__re) \
  : "r"(__CloseSocket__bn), "r"(__CloseSocket_sock)  \
  : "d1", "a0", "a1", "fp0", "fp1", "cc", "memory"); \
  __CloseSocket__re; \
  }); \
  _CloseSocket__re; \
})

#define __WaitSelect(nfds, read_fds, write_fds, except_fds, timeout, signals) ({ \
  LONG _WaitSelect_nfds = (nfds); \
  APTR _WaitSelect_read_fds = (read_fds); \
  APTR _WaitSelect_write_fds = (write_fds); \
  APTR _WaitSelect_except_fds = (except_fds); \
  struct timeval * _WaitSelect_timeout = (timeout); \
  ULONG * _WaitSelect_signals = (signals); \
  LONG _WaitSelect__re = \
  ({ \
  register struct Library * const __WaitSelect__bn __asm("a6") = (struct Library *) (__SocketBase);\
  register LONG __WaitSelect__re __asm("d0"); \
  register LONG __WaitSelect_nfds __asm("d0") = (_WaitSelect_nfds); \
  register APTR __WaitSelect_read_fds __asm("a0") = (_WaitSelect_read_fds); \
  register APTR __WaitSelect_write_fds __asm("a1") = (_WaitSelect_write_fds); \
  register APTR __WaitSelect_except_fds __asm("a2") = (_WaitSelect_except_fds); \
  register struct timeval * __WaitSelect_timeout __asm("a3") = (_WaitSelect_timeout); \
  register ULONG * __WaitSelect_signals __asm("d1") = (_WaitSelect_signals); \
  __asm volatile ("jsr a6@(-126:W)" \
  : "=r"(__WaitSelect__re) \
  : "r"(__WaitSelect__bn), "r"(__WaitSelect_nfds), "r"(__WaitSelect_read_fds), "r"(__WaitSelect_write_fds), "r"(__WaitSelect_except_fds), "r"(__WaitSelect_timeout), "r"(__WaitSelect_signals)  \
  : "d1", "a0", "a1", "fp0", "fp1", "cc", "memory"); \
  __WaitSelect__re; \
  }); \
  _WaitSelect__re; \
})

#define __SetSocketSignals(int_mask, io_mask, urgent_mask) ({ \
  ULONG _SetSocketSignals_int_mask = (int_mask); \
  ULONG _SetSocketSignals_io_mask = (io_mask); \
  ULONG _SetSocketSignals_urgent_mask = (urgent_mask); \
  { \
  register struct Library * const __SetSocketSignals__bn __asm("a6") = (struct Library *) (__SocketBase);\
  register ULONG __SetSocketSignals_int_mask __asm("d0") = (_SetSocketSignals_int_mask); \
  register ULONG __SetSocketSignals_io_mask __asm("d1") = (_SetSocketSignals_io_mask); \
  register ULONG __SetSocketSignals_urgent_mask __asm("d2") = (_SetSocketSignals_urgent_mask); \
  __asm volatile ("jsr a6@(-132:W)" \
  : \
  : "r"(__SetSocketSignals__bn), "r"(__SetSocketSignals_int_mask), "r"(__SetSocketSignals_io_mask), "r"(__SetSocketSignals_urgent_mask)  \
  : "d0", "d1", "a0", "a1", "fp0", "fp1", "cc", "memory"); \
  } \
})

#define __getdtablesize() ({ \
  LONG _getdtablesize__re = \
  ({ \
  register struct Library * const __getdtablesize__bn __asm("a6") = (struct Library *) (__SocketBase);\
  register LONG __getdtablesize__re __asm("d0"); \
  __asm volatile ("jsr a6@(-138:W)" \
  : "=r"(__getdtablesize__re) \
  : "r"(__getdtablesize__bn)  \
  : "d1", "a0", "a1", "fp0", "fp1", "cc", "memory"); \
  __getdtablesize__re; \
  }); \
  _getdtablesize__re; \
})

#define __ObtainSocket(id, domain, type, protocol) ({ \
  LONG _ObtainSocket_id = (id); \
  LONG _ObtainSocket_domain = (domain); \
  LONG _ObtainSocket_type = (type); \
  LONG _ObtainSocket_protocol = (protocol); \
  LONG _ObtainSocket__re = \
  ({ \
  register struct Library * const __ObtainSocket__bn __asm("a6") = (struct Library *) (__SocketBase);\
  register LONG __ObtainSocket__re __asm("d0"); \
  register LONG __ObtainSocket_id __asm("d0") = (_ObtainSocket_id); \
  register LONG __ObtainSocket_domain __asm("d1") = (_ObtainSocket_domain); \
  register LONG __ObtainSocket_type __asm("d2") = (_ObtainSocket_type); \
  register LONG __ObtainSocket_protocol __asm("d3") = (_ObtainSocket_protocol); \
  __asm volatile ("jsr a6@(-144:W)" \
  : "=r"(__ObtainSocket__re) \
  : "r"(__ObtainSocket__bn), "r"(__ObtainSocket_id), "r"(__ObtainSocket_domain), "r"(__ObtainSocket_type), "r"(__ObtainSocket_protocol)  \
  : "d1", "a0", "a1", "fp0", "fp1", "cc", "memory"); \
  __ObtainSocket__re; \
  }); \
  _ObtainSocket__re; \
})

#define __ReleaseSocket(sock, id) ({ \
  LONG _ReleaseSocket_sock = (sock); \
  LONG _ReleaseSocket_id = (id); \
  LONG _ReleaseSocket__re = \
  ({ \
  register struct Library * const __ReleaseSocket__bn __asm("a6") = (struct Library *) (__SocketBase);\
  register LONG __ReleaseSocket__re __asm("d0"); \
  register LONG __ReleaseSocket_sock __asm("d0") = (_ReleaseSocket_sock); \
  register LONG __ReleaseSocket_id __asm("d1") = (_ReleaseSocket_id); \
  __asm volatile ("jsr a6@(-150:W)" \
  : "=r"(__ReleaseSocket__re) \
  : "r"(__ReleaseSocket__bn), "r"(__ReleaseSocket_sock), "r"(__ReleaseSocket_id)  \
  : "d1", "a0", "a1", "fp0", "fp1", "cc", "memory"); \
  __ReleaseSocket__re; \
  }); \
  _ReleaseSocket__re; \
})

#define __ReleaseCopyOfSocket(sock, id) ({ \
  LONG _ReleaseCopyOfSocket_sock = (sock); \
  LONG _ReleaseCopyOfSocket_id = (id); \
  LONG _ReleaseCopyOfSocket__re = \
  ({ \
  register struct Library * const __ReleaseCopyOfSocket__bn __asm("a6") = (struct Library *) (__SocketBase);\
  register LONG __ReleaseCopyOfSocket__re __asm("d0"); \
  register LONG __ReleaseCopyOfSocket_sock __asm("d0") = (_ReleaseCopyOfSocket_sock); \
  register LONG __ReleaseCopyOfSocket_id __asm("d1") = (_ReleaseCopyOfSocket_id); \
  __asm volatile ("jsr a6@(-156:W)" \
  : "=r"(__ReleaseCopyOfSocket__re) \
  : "r"(__ReleaseCopyOfSocket__bn), "r"(__ReleaseCopyOfSocket_sock), "r"(__ReleaseCopyOfSocket_id)  \
  : "d1", "a0", "a1", "fp0", "fp1", "cc", "memory"); \
  __ReleaseCopyOfSocket__re; \
  }); \
  _ReleaseCopyOfSocket__re; \
})

#define __Errno() ({ \
  LONG _Errno__re = \
  ({ \
  register struct Library * const __Errno__bn __asm("a6") = (struct Library *) (__SocketBase);\
  register LONG __Errno__re __asm("d0"); \
  __asm volatile ("jsr a6@(-162:W)" \
  : "=r"(__Errno__re) \
  : "r"(__Errno__bn)  \
  : "d1", "a0", "a1", "fp0", "fp1", "cc", "memory"); \
  __Errno__re; \
  }); \
  _Errno__re; \
})

#define __SetErrnoPtr(errno_ptr, size) ({ \
  APTR _SetErrnoPtr_errno_ptr = (errno_ptr); \
  LONG _SetErrnoPtr_size = (size); \
  { \
  register struct Library * const __SetErrnoPtr__bn __asm("a6") = (struct Library *) (__SocketBase);\
  register APTR __SetErrnoPtr_errno_ptr __asm("a0") = (_SetErrnoPtr_errno_ptr); \
  register LONG __SetErrnoPtr_size __asm("d0") = (_SetErrnoPtr_size); \
  __asm volatile ("jsr a6@(-168:W)" \
  : \
  : "r"(__SetErrnoPtr__bn), "r"(__SetErrnoPtr_errno_ptr), "r"(__SetErrnoPtr_size)  \
  : "d0", "d1", "a0", "a1", "fp0", "fp1", "cc", "memory"); \
  } \
})

#define __Inet_NtoA(ip) ({ \
  ULONG _Inet_NtoA_ip = (ip); \
  STRPTR _Inet_NtoA__re = \
  ({ \
  register struct Library * const __Inet_NtoA__bn __asm("a6") = (struct Library *) (__SocketBase);\
  register STRPTR __Inet_NtoA__re __asm("d0"); \
  register ULONG __Inet_NtoA_ip __asm("d0") = (_Inet_NtoA_ip); \
  __asm volatile ("jsr a6@(-174:W)" \
  : "=r"(__Inet_NtoA__re) \
  : "r"(__Inet_NtoA__bn), "r"(__Inet_NtoA_ip)  \
  : "d1", "a0", "a1", "fp0", "fp1", "cc", "memory"); \
  __Inet_NtoA__re; \
  }); \
  _Inet_NtoA__re; \
})

#define __inet_addr(cp) ({ \
  STRPTR _inet_addr_cp = (cp); \
  ULONG _inet_addr__re = \
  ({ \
  register struct Library * const __inet_addr__bn __asm("a6") = (struct Library *) (__SocketBase);\
  register ULONG __inet_addr__re __asm("d0"); \
  register STRPTR __inet_addr_cp __asm("a0") = (_inet_addr_cp); \
  __asm volatile ("jsr a6@(-180:W)" \
  : "=r"(__inet_addr__re) \
  : "r"(__inet_addr__bn), "r"(__inet_addr_cp)  \
  : "d1", "a0", "a1", "fp0", "fp1", "cc", "memory"); \
  __inet_addr__re; \
  }); \
  _inet_addr__re; \
})

#define __Inet_LnaOf(in) ({ \
  ULONG _Inet_LnaOf_in = (in); \
  ULONG _Inet_LnaOf__re = \
  ({ \
  register struct Library * const __Inet_LnaOf__bn __asm("a6") = (struct Library *) (__SocketBase);\
  register ULONG __Inet_LnaOf__re __asm("d0"); \
  register ULONG __Inet_LnaOf_in __asm("d0") = (_Inet_LnaOf_in); \
  __asm volatile ("jsr a6@(-186:W)" \
  : "=r"(__Inet_LnaOf__re) \
  : "r"(__Inet_LnaOf__bn), "r"(__Inet_LnaOf_in)  \
  : "d1", "a0", "a1", "fp0", "fp1", "cc", "memory"); \
  __Inet_LnaOf__re; \
  }); \
  _Inet_LnaOf__re; \
})

#define __Inet_NetOf(in) ({ \
  ULONG _Inet_NetOf_in = (in); \
  ULONG _Inet_NetOf__re = \
  ({ \
  register struct Library * const __Inet_NetOf__bn __asm("a6") = (struct Library *) (__SocketBase);\
  register ULONG __Inet_NetOf__re __asm("d0"); \
  register ULONG __Inet_NetOf_in __asm("d0") = (_Inet_NetOf_in); \
  __asm volatile ("jsr a6@(-192:W)" \
  : "=r"(__Inet_NetOf__re) \
  : "r"(__Inet_NetOf__bn), "r"(__Inet_NetOf_in)  \
  : "d1", "a0", "a1", "fp0", "fp1", "cc", "memory"); \
  __Inet_NetOf__re; \
  }); \
  _Inet_NetOf__re; \
})

#define __Inet_MakeAddr(net, host) ({ \
  ULONG _Inet_MakeAddr_net = (net); \
  ULONG _Inet_MakeAddr_host = (host); \
  ULONG _Inet_MakeAddr__re = \
  ({ \
  register struct Library * const __Inet_MakeAddr__bn __asm("a6") = (struct Library *) (__SocketBase);\
  register ULONG __Inet_MakeAddr__re __asm("d0"); \
  register ULONG __Inet_MakeAddr_net __asm("d0") = (_Inet_MakeAddr_net); \
  register ULONG __Inet_MakeAddr_host __asm("d1") = (_Inet_MakeAddr_host); \
  __asm volatile ("jsr a6@(-198:W)" \
  : "=r"(__Inet_MakeAddr__re) \
  : "r"(__Inet_MakeAddr__bn), "r"(__Inet_MakeAddr_net), "r"(__Inet_MakeAddr_host)  \
  : "d1", "a0", "a1", "fp0", "fp1", "cc", "memory"); \
  __Inet_MakeAddr__re; \
  }); \
  _Inet_MakeAddr__re; \
})

#define __inet_network(cp) ({ \
  STRPTR _inet_network_cp = (cp); \
  ULONG _inet_network__re = \
  ({ \
  register struct Library * const __inet_network__bn __asm("a6") = (struct Library *) (__SocketBase);\
  register ULONG __inet_network__re __asm("d0"); \
  register STRPTR __inet_network_cp __asm("a0") = (_inet_network_cp); \
  __asm volatile ("jsr a6@(-204:W)" \
  : "=r"(__inet_network__re) \
  : "r"(__inet_network__bn), "r"(__inet_network_cp)  \
  : "d1", "a0", "a1", "fp0", "fp1", "cc", "memory"); \
  __inet_network__re; \
  }); \
  _inet_network__re; \
})

#define __gethostbyname(name) ({ \
  STRPTR _gethostbyname_name = (name); \
  struct hostent * _gethostbyname__re = \
  ({ \
  register struct Library * const __gethostbyname__bn __asm("a6") = (struct Library *) (__SocketBase);\
  register struct hostent * __gethostbyname__re __asm("d0"); \
  register STRPTR __gethostbyname_name __asm("a0") = (_gethostbyname_name); \
  __asm volatile ("jsr a6@(-210:W)" \
  : "=r"(__gethostbyname__re) \
  : "r"(__gethostbyname__bn), "r"(__gethostbyname_name)  \
  : "d1", "a0", "a1", "fp0", "fp1", "cc", "memory"); \
  __gethostbyname__re; \
  }); \
  _gethostbyname__re; \
})

#define __gethostbyaddr(addr, len, type) ({ \
  STRPTR _gethostbyaddr_addr = (addr); \
  LONG _gethostbyaddr_len = (len); \
  LONG _gethostbyaddr_type = (type); \
  struct hostent * _gethostbyaddr__re = \
  ({ \
  register struct Library * const __gethostbyaddr__bn __asm("a6") = (struct Library *) (__SocketBase);\
  register struct hostent * __gethostbyaddr__re __asm("d0"); \
  register STRPTR __gethostbyaddr_addr __asm("a0") = (_gethostbyaddr_addr); \
  register LONG __gethostbyaddr_len __asm("d0") = (_gethostbyaddr_len); \
  register LONG __gethostbyaddr_type __asm("d1") = (_gethostbyaddr_type); \
  __asm volatile ("jsr a6@(-216:W)" \
  : "=r"(__gethostbyaddr__re) \
  : "r"(__gethostbyaddr__bn), "r"(__gethostbyaddr_addr), "r"(__gethostbyaddr_len), "r"(__gethostbyaddr_type)  \
  : "d1", "a0", "a1", "fp0", "fp1", "cc", "memory"); \
  __gethostbyaddr__re; \
  }); \
  _gethostbyaddr__re; \
})

#define __getnetbyname(name) ({ \
  STRPTR _getnetbyname_name = (name); \
  struct netent * _getnetbyname__re = \
  ({ \
  register struct Library * const __getnetbyname__bn __asm("a6") = (struct Library *) (__SocketBase);\
  register struct netent * __getnetbyname__re __asm("d0"); \
  register STRPTR __getnetbyname_name __asm("a0") = (_getnetbyname_name); \
  __asm volatile ("jsr a6@(-222:W)" \
  : "=r"(__getnetbyname__re) \
  : "r"(__getnetbyname__bn), "r"(__getnetbyname_name)  \
  : "d1", "a0", "a1", "fp0", "fp1", "cc", "memory"); \
  __getnetbyname__re; \
  }); \
  _getnetbyname__re; \
})

#define __getnetbyaddr(net, type) ({ \
  ULONG _getnetbyaddr_net = (net); \
  LONG _getnetbyaddr_type = (type); \
  struct netent * _getnetbyaddr__re = \
  ({ \
  register struct Library * const __getnetbyaddr__bn __asm("a6") = (struct Library *) (__SocketBase);\
  register struct netent * __getnetbyaddr__re __asm("d0"); \
  register ULONG __getnetbyaddr_net __asm("d0") = (_getnetbyaddr_net); \
  register LONG __getnetbyaddr_type __asm("d1") = (_getnetbyaddr_type); \
  __asm volatile ("jsr a6@(-228:W)" \
  : "=r"(__getnetbyaddr__re) \
  : "r"(__getnetbyaddr__bn), "r"(__getnetbyaddr_net), "r"(__getnetbyaddr_type)  \
  : "d1", "a0", "a1", "fp0", "fp1", "cc", "memory"); \
  __getnetbyaddr__re; \
  }); \
  _getnetbyaddr__re; \
})

#define __getservbyname(name, proto) ({ \
  STRPTR _getservbyname_name = (name); \
  STRPTR _getservbyname_proto = (proto); \
  struct servent * _getservbyname__re = \
  ({ \
  register struct Library * const __getservbyname__bn __asm("a6") = (struct Library *) (__SocketBase);\
  register struct servent * __getservbyname__re __asm("d0"); \
  register STRPTR __getservbyname_name __asm("a0") = (_getservbyname_name); \
  register STRPTR __getservbyname_proto __asm("a1") = (_getservbyname_proto); \
  __asm volatile ("jsr a6@(-234:W)" \
  : "=r"(__getservbyname__re) \
  : "r"(__getservbyname__bn), "r"(__getservbyname_name), "r"(__getservbyname_proto)  \
  : "d1", "a0", "a1", "fp0", "fp1", "cc", "memory"); \
  __getservbyname__re; \
  }); \
  _getservbyname__re; \
})

#define __getservbyport(port, proto) ({ \
  LONG _getservbyport_port = (port); \
  STRPTR _getservbyport_proto = (proto); \
  struct servent * _getservbyport__re = \
  ({ \
  register struct Library * const __getservbyport__bn __asm("a6") = (struct Library *) (__SocketBase);\
  register struct servent * __getservbyport__re __asm("d0"); \
  register LONG __getservbyport_port __asm("d0") = (_getservbyport_port); \
  register STRPTR __getservbyport_proto __asm("a0") = (_getservbyport_proto); \
  __asm volatile ("jsr a6@(-240:W)" \
  : "=r"(__getservbyport__re) \
  : "r"(__getservbyport__bn), "r"(__getservbyport_port), "r"(__getservbyport_proto)  \
  : "d1", "a0", "a1", "fp0", "fp1", "cc", "memory"); \
  __getservbyport__re; \
  }); \
  _getservbyport__re; \
})

#define __getprotobyname(name) ({ \
  STRPTR _getprotobyname_name = (name); \
  struct protoent * _getprotobyname__re = \
  ({ \
  register struct Library * const __getprotobyname__bn __asm("a6") = (struct Library *) (__SocketBase);\
  register struct protoent * __getprotobyname__re __asm("d0"); \
  register STRPTR __getprotobyname_name __asm("a0") = (_getprotobyname_name); \
  __asm volatile ("jsr a6@(-246:W)" \
  : "=r"(__getprotobyname__re) \
  : "r"(__getprotobyname__bn), "r"(__getprotobyname_name)  \
  : "d1", "a0", "a1", "fp0", "fp1", "cc", "memory"); \
  __getprotobyname__re; \
  }); \
  _getprotobyname__re; \
})

#define __getprotobynumber(proto) ({ \
  LONG _getprotobynumber_proto = (proto); \
  struct protoent * _getprotobynumber__re = \
  ({ \
  register struct Library * const __getprotobynumber__bn __asm("a6") = (struct Library *) (__SocketBase);\
  register struct protoent * __getprotobynumber__re __asm("d0"); \
  register LONG __getprotobynumber_proto __asm("d0") = (_getprotobynumber_proto); \
  __asm volatile ("jsr a6@(-252:W)" \
  : "=r"(__getprotobynumber__re) \
  : "r"(__getprotobynumber__bn), "r"(__getprotobynumber_proto)  \
  : "d1", "a0", "a1", "fp0", "fp1", "cc", "memory"); \
  __getprotobynumber__re; \
  }); \
  _getprotobynumber__re; \
})

#define __vsyslog(pri, msg, args) ({ \
  LONG _vsyslog_pri = (pri); \
  STRPTR _vsyslog_msg = (msg); \
  APTR _vsyslog_args = (args); \
  { \
  register struct Library * const __vsyslog__bn __asm("a6") = (struct Library *) (__SocketBase);\
  register LONG __vsyslog_pri __asm("d0") = (_vsyslog_pri); \
  register STRPTR __vsyslog_msg __asm("a0") = (_vsyslog_msg); \
  register APTR __vsyslog_args __asm("a1") = (_vsyslog_args); \
  __asm volatile ("jsr a6@(-258:W)" \
  : \
  : "r"(__vsyslog__bn), "r"(__vsyslog_pri), "r"(__vsyslog_msg), "r"(__vsyslog_args)  \
  : "d0", "d1", "a0", "a1", "fp0", "fp1", "cc", "memory"); \
  } \
})

#define __Dup2Socket(old_socket, new_socket) ({ \
  LONG _Dup2Socket_old_socket = (old_socket); \
  LONG _Dup2Socket_new_socket = (new_socket); \
  LONG _Dup2Socket__re = \
  ({ \
  register struct Library * const __Dup2Socket__bn __asm("a6") = (struct Library *) (__SocketBase);\
  register LONG __Dup2Socket__re __asm("d0"); \
  register LONG __Dup2Socket_old_socket __asm("d0") = (_Dup2Socket_old_socket); \
  register LONG __Dup2Socket_new_socket __asm("d1") = (_Dup2Socket_new_socket); \
  __asm volatile ("jsr a6@(-264:W)" \
  : "=r"(__Dup2Socket__re) \
  : "r"(__Dup2Socket__bn), "r"(__Dup2Socket_old_socket), "r"(__Dup2Socket_new_socket)  \
  : "d1", "a0", "a1", "fp0", "fp1", "cc", "memory"); \
  __Dup2Socket__re; \
  }); \
  _Dup2Socket__re; \
})

#define __sendmsg(sock, msg, flags) ({ \
  LONG _sendmsg_sock = (sock); \
  struct msghdr * _sendmsg_msg = (msg); \
  LONG _sendmsg_flags = (flags); \
  LONG _sendmsg__re = \
  ({ \
  register struct Library * const __sendmsg__bn __asm("a6") = (struct Library *) (__SocketBase);\
  register LONG __sendmsg__re __asm("d0"); \
  register LONG __sendmsg_sock __asm("d0") = (_sendmsg_sock); \
  register struct msghdr * __sendmsg_msg __asm("a0") = (_sendmsg_msg); \
  register LONG __sendmsg_flags __asm("d1") = (_sendmsg_flags); \
  __asm volatile ("jsr a6@(-270:W)" \
  : "=r"(__sendmsg__re) \
  : "r"(__sendmsg__bn), "r"(__sendmsg_sock), "r"(__sendmsg_msg), "r"(__sendmsg_flags)  \
  : "d1", "a0", "a1", "fp0", "fp1", "cc", "memory"); \
  __sendmsg__re; \
  }); \
  _sendmsg__re; \
})

#define __recvmsg(sock, msg, flags) ({ \
  LONG _recvmsg_sock = (sock); \
  struct msghdr * _recvmsg_msg = (msg); \
  LONG _recvmsg_flags = (flags); \
  LONG _recvmsg__re = \
  ({ \
  register struct Library * const __recvmsg__bn __asm("a6") = (struct Library *) (__SocketBase);\
  register LONG __recvmsg__re __asm("d0"); \
  register LONG __recvmsg_sock __asm("d0") = (_recvmsg_sock); \
  register struct msghdr * __recvmsg_msg __asm("a0") = (_recvmsg_msg); \
  register LONG __recvmsg_flags __asm("d1") = (_recvmsg_flags); \
  __asm volatile ("jsr a6@(-276:W)" \
  : "=r"(__recvmsg__re) \
  : "r"(__recvmsg__bn), "r"(__recvmsg_sock), "r"(__recvmsg_msg), "r"(__recvmsg_flags)  \
  : "d1", "a0", "a1", "fp0", "fp1", "cc", "memory"); \
  __recvmsg__re; \
  }); \
  _recvmsg__re; \
})

#define __gethostname(name, namelen) ({ \
  STRPTR _gethostname_name = (name); \
  LONG _gethostname_namelen = (namelen); \
  LONG _gethostname__re = \
  ({ \
  register struct Library * const __gethostname__bn __asm("a6") = (struct Library *) (__SocketBase);\
  register LONG __gethostname__re __asm("d0"); \
  register STRPTR __gethostname_name __asm("a0") = (_gethostname_name); \
  register LONG __gethostname_namelen __asm("d0") = (_gethostname_namelen); \
  __asm volatile ("jsr a6@(-282:W)" \
  : "=r"(__gethostname__re) \
  : "r"(__gethostname__bn), "r"(__gethostname_name), "r"(__gethostname_namelen)  \
  : "d1", "a0", "a1", "fp0", "fp1", "cc", "memory"); \
  __gethostname__re; \
  }); \
  _gethostname__re; \
})

#define __gethostid() ({ \
  ULONG _gethostid__re = \
  ({ \
  register struct Library * const __gethostid__bn __asm("a6") = (struct Library *) (__SocketBase);\
  register ULONG __gethostid__re __asm("d0"); \
  __asm volatile ("jsr a6@(-288:W)" \
  : "=r"(__gethostid__re) \
  : "r"(__gethostid__bn)  \
  : "d1", "a0", "a1", "fp0", "fp1", "cc", "memory"); \
  __gethostid__re; \
  }); \
  _gethostid__re; \
})

#define __SocketBaseTagList(tags) ({ \
  struct TagItem * _SocketBaseTagList_tags = (tags); \
  LONG _SocketBaseTagList__re = \
  ({ \
  register struct Library * const __SocketBaseTagList__bn __asm("a6") = (struct Library *) (__SocketBase);\
  register LONG __SocketBaseTagList__re __asm("d0"); \
  register struct TagItem * __SocketBaseTagList_tags __asm("a0") = (_SocketBaseTagList_tags); \
  __asm volatile ("jsr a6@(-294:W)" \
  : "=r"(__SocketBaseTagList__re) \
  : "r"(__SocketBaseTagList__bn), "r"(__SocketBaseTagList_tags)  \
  : "d1", "a0", "a1", "fp0", "fp1", "cc", "memory"); \
  __SocketBaseTagList__re; \
  }); \
  _SocketBaseTagList__re; \
})

#endif /* __amigaos4__ */

#endif /* __GNUC__ */

/****************************************************************************/

#if defined(__SASC)

LONG __socket(LONG domain,LONG type,LONG protocol);
LONG __bind(LONG sock,struct sockaddr *name,LONG namelen);
LONG __listen(LONG sock,LONG backlog);
LONG __accept(LONG sock,struct sockaddr *addr,LONG *addrlen);
LONG __connect(LONG sock,struct sockaddr *name,LONG namelen);
LONG __sendto(LONG sock,APTR buf,LONG len,LONG flags,struct sockaddr *to,LONG tolen);
LONG __send(LONG sock,APTR buf,LONG len,LONG flags);
LONG __recvfrom(LONG sock,APTR buf,LONG len,LONG flags,struct sockaddr *addr,LONG *addrlen);
LONG __recv(LONG sock,APTR buf,LONG len,LONG flags);
LONG __shutdown(LONG sock,LONG how);
LONG __setsockopt(LONG sock,LONG level,LONG optname,APTR optval,LONG optlen);
LONG __getsockopt(LONG sock,LONG level,LONG optname,APTR optval,LONG *optlen);
LONG __getsockname(LONG sock,struct sockaddr *name,LONG *namelen);
LONG __getpeername(LONG sock,struct sockaddr *name,LONG *namelen);
LONG __IoctlSocket(LONG sock,ULONG req,APTR argp);
LONG __CloseSocket(LONG sock);
LONG __WaitSelect(LONG nfds,APTR read_fds,APTR write_fds,APTR except_fds,struct timeval *timeout,ULONG *signals);
VOID __SetSocketSignals(ULONG int_mask,ULONG io_mask,ULONG urgent_mask);
LONG __getdtablesize(VOID);
LONG __ObtainSocket(LONG id,LONG domain,LONG type,LONG protocol);
LONG __ReleaseSocket(LONG sock,LONG id);
LONG __ReleaseCopyOfSocket(LONG sock,LONG id);
LONG __Errno(VOID);
VOID __SetErrnoPtr(APTR errno_ptr,LONG size);
STRPTR __Inet_NtoA(ULONG ip);
ULONG __inet_addr(STRPTR cp);
ULONG __Inet_LnaOf(ULONG in);
ULONG __Inet_NetOf(ULONG in);
ULONG __Inet_MakeAddr(ULONG net,ULONG host);
ULONG __inet_network(STRPTR cp);
struct hostent *__gethostbyname(STRPTR name);
struct hostent *__gethostbyaddr(STRPTR addr,LONG len,LONG type);
struct netent *__getnetbyname(STRPTR name);
struct netent *__getnetbyaddr(LONG net,LONG type);
struct servent *__getservbyname(STRPTR name,STRPTR proto);
struct servent *__getservbyport(LONG port,STRPTR proto);
struct protoent *__getprotobyname(STRPTR name);
struct protoent *__getprotobynumber(LONG proto);
VOID __vsyslog(LONG pri,STRPTR msg,APTR args);
LONG __Dup2Socket(LONG old_socket,LONG new_socket);
LONG __sendmsg(LONG sock,struct msghdr *msg,LONG flags);
LONG __recvmsg(LONG sock,struct msghdr *msg,LONG flags);
LONG __gethostname(STRPTR name,LONG namelen);
ULONG __gethostid(VOID);
LONG __SocketBaseTagList(struct TagItem *tags);

#pragma libcall __SocketBase __socket              01e 21003
#pragma libcall __SocketBase __bind                024 18003
#pragma libcall __SocketBase __listen              02a 1002
#pragma libcall __SocketBase __accept              030 98003
#pragma libcall __SocketBase __connect             036 18003
#pragma libcall __SocketBase __sendto              03c 39218006
#pragma libcall __SocketBase __send                042 218004
#pragma libcall __SocketBase __recvfrom            048 a9218006
#pragma libcall __SocketBase __recv                04e 218004
#pragma libcall __SocketBase __shutdown            054 1002
#pragma libcall __SocketBase __setsockopt          05a 3821005
#pragma libcall __SocketBase __getsockopt          060 9821005
#pragma libcall __SocketBase __getsockname         066 98003
#pragma libcall __SocketBase __getpeername         06c 98003
#pragma libcall __SocketBase __IoctlSocket         072 81003
#pragma libcall __SocketBase __CloseSocket         078 001
#pragma libcall __SocketBase __WaitSelect          07e 1ba98006
#pragma libcall __SocketBase __SetSocketSignals    084 21003
#pragma libcall __SocketBase __getdtablesize       08a 00
#pragma libcall __SocketBase __ObtainSocket        090 321004
#pragma libcall __SocketBase __ReleaseSocket       096 1002
#pragma libcall __SocketBase __ReleaseCopyOfSocket 09c 1002
#pragma libcall __SocketBase __Errno               0a2 00
#pragma libcall __SocketBase __SetErrnoPtr         0a8 0802
#pragma libcall __SocketBase __Inet_NtoA           0ae 001
#pragma libcall __SocketBase __inet_addr           0b4 801
#pragma libcall __SocketBase __Inet_LnaOf          0ba 001
#pragma libcall __SocketBase __Inet_NetOf          0c0 001
#pragma libcall __SocketBase __Inet_MakeAddr       0c6 1002
#pragma libcall __SocketBase __inet_network        0cc 801
#pragma libcall __SocketBase __gethostbyname       0d2 801
#pragma libcall __SocketBase __gethostbyaddr       0d8 10803
#pragma libcall __SocketBase __getnetbyname        0de 801
#pragma libcall __SocketBase __getnetbyaddr        0e4 1002
#pragma libcall __SocketBase __getservbyname       0ea 9802
#pragma libcall __SocketBase __getservbyport       0f0 8002
#pragma libcall __SocketBase __getprotobyname      0f6 801
#pragma libcall __SocketBase __getprotobynumber    0fc 001
#pragma libcall __SocketBase __vsyslog             102 98003
#pragma libcall __SocketBase __Dup2Socket          108 1002
#pragma libcall __SocketBase __sendmsg             10e 18003
#pragma libcall __SocketBase __recvmsg             114 18003
#pragma libcall __SocketBase __gethostname         11a 0802
#pragma libcall __SocketBase __gethostid           120 00
#pragma libcall __SocketBase __SocketBaseTagList   126 801

#endif /* __SASC */

/****************************************************************************/

#endif /* _SOCKET_HEADERS_H */
