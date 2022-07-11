/* -*- c++ -*-
 * SOCLIB_LGPL_HEADER_BEGIN
 * 
 * This file is part of SoCLib, GNU LGPLv2.1.
 * 
 * SoCLib is free software; you can redistribute it and/or modify it
 * under the terms of the GNU Lesser General Public License as published
 * by the Free Software Foundation; version 2.1 of the License.
 * 
 * SoCLib is distributed in the hope that it will be useful, but
 * WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * Lesser General Public License for more details.
 * 
 * You should have received a copy of the GNU Lesser General Public
 * License along with SoCLib; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA
 * 02110-1301 USA
 * 
 * SOCLIB_LGPL_HEADER_END
 *
 * Copyright (c) UPMC, Lip6, Asim
 *         Nicolas Pouillon <nipo@ssji.net>, 2007
 */

#include "exception.h"
#include "xterm_wrapper.h"

#include <cstdio>
#include <errno.h>
#include <stdlib.h>
#include <unistd.h>
#include <signal.h>
#include <fcntl.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <sys/select.h>
#include <vector>

namespace soclib { namespace common {

XtermWrapper::XtermWrapper(const std::string &name)
{
    static int n=0;
    char *geometry[]={
        "100x80+0+0",
        "80x25+625+0",
        "80x25+625+388",
        "80x25+625+746",
    };
    char *geo = geometry[n];
    n = (n+1) % 4;
    int fds[2];
    if ( socketpair( AF_UNIX, SOCK_STREAM, 0, fds ) < 0 )
        throw soclib::exception::RunTimeError("socketpair() failed");

    m_pid = fork();
    if ( m_pid < 0 ) {
        throw soclib::exception::RunTimeError("fork() failed");
    }

    if ( m_pid ) {
        // parent

		// Xterm gives out a line with its window ID; we dont care
		// about it and we dont want it to get through to the
		// simulated platform... Let's strip it.
		char buf;

        ::close(fds[0]);
        m_fd = fds[1];

		do {
			::read( m_fd, &buf, 1 );
		} while( buf != '\n' && buf != '\r' );

		// Change xterm CR/LF mode
		::write( m_fd, "\x1b[20h", 5 );

		fcntl( m_fd, F_SETFL, O_NONBLOCK );
		// The last char we want to strip is not always present,
		// so dont block if not here
		::read( m_fd, &buf, 1 );

        m_poller = FdPoller( m_fd, true );
    } else {
        const int maxlen = 10;
		char pname_arg[maxlen];
		int fd = fds[0];
		snprintf(pname_arg, maxlen, "-S0/%d", fd);

        ::close(fds[1]);
        unlink(name.c_str());

        
        execlp("xterm", "xterm",
               pname_arg,
               "-T", name.c_str(),
               "-bg", "black",
               "-fg", "green",
               "-l", "-lf", name.c_str(),
               "-geometry", geo,
               NULL);
        perror("execlp");
        /** \todo Replace this with some advertisement mechanism, and
         * report error back in parent process in a clean way
         */
        ::kill(getppid(), SIGKILL);
        _exit(2);
    }
}
    
XtermWrapper::~XtermWrapper()
{
    kill(SIGTERM);
    close(m_fd);
}

ssize_t XtermWrapper::read( void *buffer, size_t len )
{
    ssize_t r = ::read( m_fd, buffer, len );
    m_poller.reset();
    return r;
}

ssize_t XtermWrapper::write( const void *buffer, size_t len )
{
    return ::write( m_fd, buffer, len );
}

bool XtermWrapper::poll()
{
    return m_poller.has_data();
}

void XtermWrapper::kill(int sig)
{
    m_poller = FdPoller();
    ::kill(m_pid, sig);
}

}}

// Local Variables:
// tab-width: 4
// c-basic-offset: 4
// c-file-offsets:((innamespace . 0)(inline-open . 0))
// indent-tabs-mode: nil
// End:

// vim: filetype=cpp:expandtab:shiftwidth=4:tabstop=4:softtabstop=4

