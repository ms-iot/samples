BACnet open source protocol stack for embedded systems, Linux, and Windows
http://bacnet.sourceforge.net/

Welcome to the wonderful world of BACnet and true device interoperability!

About this Project
------------------

This BACnet library provides a BACnet application layer, network layer and
media access (MAC) layer communications services for an embedded system.

BACnet - A Data Communication Protocol for Building Automation and Control
Networks - see bacnet.org. BACnet is a standard data communication protocol for
Building Automation and Control Networks. BACnet is an open protocol, which
means anyone can contribute to the standard, and anyone may use it. The only
caveat is that the BACnet standard document itself is copyrighted by ASHRAE,
and they sell the document to help defray costs of developing and maintaining
the standard (just like IEEE or ANSI or ISO).

For software developers, the BACnet protocol is a standard way to send and
receive messages on the wire containing data that is understood by other BACnet
compliant devices. The BACnet standard defines a standard way to communicate
over various wires, known as Data Link/Physical Layers: Ethernet, EIA-485,
EIA-232, ARCNET, and LonTalk. The BACnet standard also defines a standard way
to communicate using UDP, IP and HTTP (Web Services).

This BACnet protocol stack implementation is specifically designed for the
embedded BACnet appliance, using a GPL with exception license (like eCos),
which means that any changes to the core code that are distributed get to come
back into the core code, but the BACnet library can be linked to proprietary
code without the proprietary code becoming GPL. Note that some of the source
files are designed as skeleton or example files, and are not copyrighted.

The text of the GPL exception included in each source file is as follows: 

"As a special exception, if other files instantiate templates or use macros or
inline functions from this file, or you compile this file and link it with
other works to produce a work based on this file, this file does not by itself
cause the resulting work to be covered by the GNU General Public License.
However the source code for this file must still be made available in
accordance with section (3) of the GNU General Public License."

The code is written in C for portability, and includes unit tests (PC based
unit tests). Since the code is designed to be portable, it compiles with GCC as
well as other compilers, such as Borland C++ or MicroChip C18.

The BACnet protocol is an ASHRAE/ANSI/ISO standard, so this library adheres to
that standard. BACnet has no royalties or licensing restrictions, and
registration for a BACnet vendor ID is free.

What the code does
------------------

The stack comes with unit tests that can be run in a command shell using the
test.sh script. The unit tests can also be run using individual .mak files.
They were tested on a Linux PC.

The BACnet stack was functionally tested using VTS (Visual Test Shell), another
project hosted on SourceForge, as well as various controllers and workstations.
Using the Makefile in the project root directory, a dozen sample applications
are created that run under Windows or Linux. They use the BACnet/IP datalink
layer for communication by default, but could be compiled to use BACnet 
Ethernet, ARCNET, or MS/TP.

Linux/Unix/Cygwin
$ make clean all

Windows
c:\> build.bat

The BACnet stack can be compiled by a variety of compilers.  The most common
free compiler is GCC (MinGW under Windows).  The makefiles use GCC by
default.  Makefile.b32 are written for the Borland C++ 5.5 compiler, and
projects are also included for Microsoft Visual Studio and Code::Blocks.

The demo applications are all client applications that provide one main BACnet
service, except the one server application.  Each application will accept 
command line parameters, and prints the output to stdout or stderr.  The client
applications are command line based and can be used in scripts or for 
troubleshooting.  The demo applications make use of environment variables to 
setup the network options.  See each individual demo for the options.

There are also projects in the ports/ directory for ARM7, AVR, RTOS-32, 
and PIC.  Each of those projects has a demo application for specific hardware.
In the case of the ARM7 and AVR, the makefile works with GCC compilers and
there are project files for IAR Embedded Workbench.

Project Documentation
---------------------

The project documentation is in the doc/ directory.  Similar documents are
on the project website at <http://bacnet.sourceforge.net/>.

Project Mailing List
--------------------

If you want to help this project, or have a problem getting it to work for
your device, or have a BACnet question, join the developers mailing list at:
http://lists.sourceforge.net/mailman/listinfo/bacnet-developers

I hope that you get your BACnet Device working!  If not, join us on the 
mailing list and we can help.

Steve Karg
Birmingham, Alabama USA
skarg@users.sourceforge.net
