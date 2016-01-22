/**************************************************************************
*
* Copyright (C) 2005 Steve Karg <skarg@users.sourceforge.net>
*
* Permission is hereby granted, free of charge, to any person obtaining
* a copy of this software and associated documentation files (the
* "Software"), to deal in the Software without restriction, including
* without limitation the rights to use, copy, modify, merge, publish,
* distribute, sublicense, and/or sell copies of the Software, and to
* permit persons to whom the Software is furnished to do so, subject to
* the following conditions:
*
* The above copyright notice and this permission notice shall be included
* in all copies or substantial portions of the Software.
*
* THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND,
* EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF
* MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT.
* IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY
* CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT,
* TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION WITH THE
* SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.
*
*********************************************************************/

#ifndef NET_H
#define NET_H

/* common unix sockets headers needed */
#include	<sys/types.h>   /* basic system data types */
#include	<sys/time.h>    /* timeval{} for select() */
#include	<time.h>        /* timespec{} for pselect() */
#include	<netinet/in.h>  /* sockaddr_in{} and other Internet defns */
#include	<arpa/inet.h>   /* inet(3) functions */
#include	<fcntl.h>       /* for nonblocking */
#include	<netdb.h>
#include	<errno.h>
#include	<signal.h>
#include	<stdio.h>
#include	<stdlib.h>
#include	<string.h>
#include	<sys/stat.h>    /* for S_xxx file mode constants */
#include	<sys/uio.h>     /* for iovec{} and readv/writev */
#include	<unistd.h>
#include	<sys/wait.h>
#include	<sys/un.h>      /* for Unix domain sockets */

#ifdef	HAVE_SYS_SELECT_H
#include	<sys/select.h>  /* for convenience */
#endif

#ifdef	HAVE_POLL_H
#include	<poll.h>        /* for convenience */
#endif

#ifdef	HAVE_STRINGS_H
#include	<strings.h>     /* for convenience */
#endif

/* Three headers are normally needed for socket/file ioctl's:
 * <sys/ioctl.h>, <sys/filio.h>, and <sys/sockio.h>.
 */
#ifdef	HAVE_SYS_IOCTL_H
#include	<sys/ioctl.h>
#endif
#ifdef	HAVE_SYS_FILIO_H
#include	<sys/filio.h>
#endif
#ifdef	HAVE_SYS_SOCKIO_H
#include	<sys/sockio.h>
#endif

#include <pthread.h>
#include <semaphore.h>

#define ENUMS
#include <sys/socket.h>
#include <net/route.h>
#include <net/if.h>
#include <net/if_var.h>
#include <net/if_arp.h>
#include <net/if_dl.h>
#include <ifaddrs.h>
#include <net/ethernet.h>       /* the L2 protocols */
#include <netinet/in.h>
#include <netinet/in_var.h>
#include <arpa/inet.h>
#include <sys/un.h>
#include <sys/ioctl.h>
#include <netdb.h>

/** @file bsd/net.h  Includes BSD network headers. */

/* Local helper functions for this port */
extern int bip_get_local_netmask(
    struct in_addr *netmask);


#endif
