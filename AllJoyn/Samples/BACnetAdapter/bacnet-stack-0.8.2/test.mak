# tools - only if you need them.
# Most platforms have this already defined
# CC = gcc
# AR = ar
# MAKE = $(MAKE)
# SIZE = size
#
# Assumes rm, touch, and cp are available

LOGFILE = test.log

all: abort address arf awf bacapp bacdcode bacerror bacint bacstr \
	cov crc datetime dcc event filename fifo getevent iam ihave \
	indtext keylist key memcopy npdu proplist ptransfer \
	rd reject ringbuf rp rpm sbuf timesync \
	whohas whois wp objects lighting

clean: logfile
	rm ${LOGFILE}

logfile:
	touch ${LOGFILE}

abort: logfile test/abort.mak
	$(MAKE) -s -C test -f abort.mak clean all
	( ./test/abort >> ${LOGFILE} )
	$(MAKE) -s -C test -f abort.mak clean

address: logfile test/address.mak
	$(MAKE) -s -C test -f address.mak clean all
	( ./test/address >> ${LOGFILE} )
	$(MAKE) -s -C test -f address.mak clean

arf: logfile test/arf.mak
	$(MAKE) -s -C test -f arf.mak clean all
	( ./test/arf >> ${LOGFILE} )
	$(MAKE) -s -C test -f arf.mak clean

awf: logfile test/awf.mak
	$(MAKE) -s -C test -f awf.mak clean all
	( ./test/awf >> ${LOGFILE} )
	$(MAKE) -s -C test -f awf.mak clean

bacapp: logfile test/bacapp.mak
	$(MAKE) -s -C test -f bacapp.mak clean all
	( ./test/bacapp >> ${LOGFILE} )
	$(MAKE) -s -C test -f bacapp.mak clean

bacdcode: logfile test/bacdcode.mak
	$(MAKE) -s -C test -f bacdcode.mak clean all
	( ./test/bacdcode >> ${LOGFILE} )
	$(MAKE) -s -C test -f bacdcode.mak clean

bacerror: logfile test/bacerror.mak
	$(MAKE) -s -C test -f bacerror.mak clean all
	( ./test/bacerror >> ${LOGFILE} )
	$(MAKE) -s -C test -f bacerror.mak clean

bacint: logfile test/bacint.mak
	$(MAKE) -s -C test -f bacint.mak clean all
	( ./test/bacint >> ${LOGFILE} )
	$(MAKE) -s -C test -f bacint.mak clean

bacstr: logfile test/bacstr.mak
	$(MAKE) -s -C test -f bacstr.mak clean all
	( ./test/bacstr >> ${LOGFILE} )
	$(MAKE) -s -C test -f bacstr.mak clean

cov: logfile test/cov.mak
	$(MAKE) -s -C test -f cov.mak clean all
	( ./test/cov >> ${LOGFILE} )
	$(MAKE) -s -C test -f cov.mak clean

crc: logfile test/crc.mak
	$(MAKE) -s -C test -f crc.mak clean all
	( ./test/crc >> ${LOGFILE} )
	$(MAKE) -s -C test -f crc.mak clean

datetime: logfile test/datetime.mak
	$(MAKE) -s -C test -f datetime.mak clean all
	( ./test/datetime >> ${LOGFILE} )
	$(MAKE) -s -C test -f datetime.mak clean

dcc: logfile test/dcc.mak
	$(MAKE) -s -C test -f dcc.mak clean all
	( ./test/dcc >> ${LOGFILE} )
	$(MAKE) -s -C test -f dcc.mak clean

event: logfile test/event.mak
	$(MAKE) -s -C test -f event.mak clean all
	( ./test/event >> ${LOGFILE} )
	$(MAKE) -s -C test -f event.mak clean

filename: logfile test/filename.mak
	$(MAKE) -s -C test -f filename.mak clean all
	( ./test/filename >> ${LOGFILE} )
	$(MAKE) -s -C test -f filename.mak clean

fifo: logfile test/fifo.mak
	$(MAKE) -s -C test -f fifo.mak clean all
	( ./test/fifo >> ${LOGFILE} )
	$(MAKE) -s -C test -f fifo.mak clean

getevent: logfile test/getevent.mak
	$(MAKE) -s -C test -f getevent.mak clean all
	( ./test/getevent >> ${LOGFILE} )
	$(MAKE) -s -C test -f getevent.mak clean

iam: logfile test/iam.mak
	$(MAKE) -s -C test -f iam.mak clean all
	( ./test/iam >> ${LOGFILE} )
	$(MAKE) -s -C test -f iam.mak clean

ihave: logfile test/ihave.mak
	$(MAKE) -s -C test -f ihave.mak clean all
	( ./test/ihave >> ${LOGFILE} )
	$(MAKE) -s -C test -f ihave.mak clean

indtext: logfile test/indtext.mak
	$(MAKE) -s -C test -f indtext.mak clean all
	( ./test/indtext >> ${LOGFILE} )
	$(MAKE) -s -C test -f indtext.mak clean

keylist: logfile test/keylist.mak
	$(MAKE) -s -C test -f keylist.mak clean all
	( ./test/keylist >> ${LOGFILE} )
	$(MAKE) -s -C test -f keylist.mak clean

key: logfile test/key.mak
	$(MAKE) -s -C test -f key.mak clean all
	( ./test/key >> ${LOGFILE} )
	$(MAKE) -s -C test -f key.mak clean

lighting: lighting test/lighting.mak
	$(MAKE) -s -C test -f lighting.mak clean all
	( ./test/lighting >> ${LOGFILE} )
	$(MAKE) -s -C test -f lighting.mak clean

memcopy: logfile test/memcopy.mak
	$(MAKE) -s -C test -f memcopy.mak clean all
	( ./test/memcopy >> ${LOGFILE} )
	$(MAKE) -s -C test -f memcopy.mak clean

npdu: logfile test/npdu.mak
	$(MAKE) -s -C test -f npdu.mak clean all
	( ./test/npdu >> ${LOGFILE} )
	$(MAKE) -s -C test -f npdu.mak clean

proplist: logfile test/proplist.mak
	$(MAKE) -s -C test -f proplist.mak clean all
	( ./test/proplist >> ${LOGFILE} )
	$(MAKE) -s -C test -f proplist.mak clean

ptransfer: logfile test/ptransfer.mak
	$(MAKE) -s -C test -f ptransfer.mak clean all
	( ./test/ptransfer >> ${LOGFILE} )
	$(MAKE) -s -C test -f ptransfer.mak clean

rd: logfile test/rd.mak
	$(MAKE) -s -C test -f rd.mak clean all
	( ./test/rd >> ${LOGFILE} )
	$(MAKE) -s -C test -f rd.mak clean

reject: logfile test/reject.mak
	$(MAKE) -s -C test -f reject.mak clean all
	( ./test/reject >> ${LOGFILE} )
	$(MAKE) -s -C test -f reject.mak clean

ringbuf: logfile test/ringbuf.mak
	$(MAKE) -s -C test -f ringbuf.mak clean all
	( ./test/ringbuf >> ${LOGFILE} )
	$(MAKE) -s -C test -f ringbuf.mak clean

rp: logfile test/rp.mak
	$(MAKE) -s -C test -f rp.mak clean all
	( ./test/rp >> ${LOGFILE} )
	$(MAKE) -s -C test -f rp.mak clean

rpm: logfile test/rpm.mak
	$(MAKE) -s -C test -f rpm.mak clean all
	( ./test/rpm >> ${LOGFILE} )
	$(MAKE) -s -C test -f rpm.mak clean

sbuf: logfile test/sbuf.mak
	$(MAKE) -s -C test -f sbuf.mak clean all
	( ./test/sbuf >> ${LOGFILE} )
	$(MAKE) -s -C test -f sbuf.mak clean

timesync: logfile test/timesync.mak
	$(MAKE) -s -C test -f timesync.mak clean all
	( ./test/timesync >> ${LOGFILE} )
	$(MAKE) -s -C test -f timesync.mak clean

whohas: logfile test/whohas.mak
	$(MAKE) -s -C test -f whohas.mak clean all
	( ./test/whohas >> ${LOGFILE} )
	$(MAKE) -s -C test -f whohas.mak clean

whois: logfile test/whois.mak
	$(MAKE) -s -C test -f whois.mak clean all
	( ./test/whois >> ${LOGFILE} )
	$(MAKE) -s -C test -f whois.mak clean

wp: logfile test/wp.mak
	$(MAKE) -s -C test -f wp.mak clean all
	( ./test/wp >> ${LOGFILE} )
	$(MAKE) -s -C test -f wp.mak clean

objects: ai ao av bi bo bv csv lc lo lso lsp mso msv ms-input command

ai: logfile demo/object/ai.mak
	$(MAKE) -s -C demo/object -f ai.mak clean all
	( ./demo/object/analog_input >> ${LOGFILE} )
	$(MAKE) -s -C demo/object -f ai.mak clean

ao: logfile demo/object/ao.mak
	$(MAKE) -s -C demo/object -f ao.mak clean all
	( ./demo/object/analog_output >> ${LOGFILE} )
	$(MAKE) -s -C demo/object -f ao.mak clean

av: logfile demo/object/av.mak
	$(MAKE) -s -C demo/object -f av.mak clean all
	( ./demo/object/analog_value >> ${LOGFILE} )
	$(MAKE) -s -C demo/object -f av.mak clean

bi: logfile demo/object/bi.mak
	$(MAKE) -s -C demo/object -f bi.mak clean all
	$(MAKE) -s -C demo/object -f bi.mak clean

bo: logfile demo/object/bo.mak
	$(MAKE) -s -C demo/object -f bo.mak clean all
	( ./demo/object/binary_output >> ${LOGFILE} )
	$(MAKE) -s -C demo/object -f bo.mak clean

bv: logfile demo/object/bv.mak
	$(MAKE) -s -C demo/object -f bv.mak clean all
	( ./demo/object/binary_value >> ${LOGFILE} )
	$(MAKE) -s -C demo/object -f bv.mak clean

command: logfile demo/object/command.mak
	$(MAKE) -s -C demo/object -f command.mak clean all
	( ./demo/object/command >> ${LOGFILE} )
	$(MAKE) -s -C demo/object -f command.mak clean

csv: logfile demo/object/csv.mak
	$(MAKE) -s -C demo/object -f csv.mak clean all
	( ./demo/object/characterstring_value >> ${LOGFILE} )
	$(MAKE) -s -C demo/object -f csv.mak clean

device: logfile demo/object/device.mak
	$(MAKE) -s -C demo/object -f device.mak clean all
	( ./demo/object/device >> ${LOGFILE} )
	$(MAKE) -s -C demo/object -f device.mak clean

lc: logfile demo/object/lc.mak
	$(MAKE) -s -C demo/object -f lc.mak clean all
	( ./demo/object/load_control >> ${LOGFILE} )
	$(MAKE) -s -C demo/object -f lc.mak clean

lo: logfile demo/object/lo.mak
	$(MAKE) -s -C demo/object -f lo.mak clean all
	( ./demo/object/lighting_output >> ${LOGFILE} )
	$(MAKE) -s -C demo/object -f lo.mak clean

lso: logfile test/lso.mak
	$(MAKE) -s -C test -f lso.mak clean all
	( ./test/lso >> ${LOGFILE} )
	$(MAKE) -s -C test -f lso.mak clean

lsp: logfile demo/object/lsp.mak
	$(MAKE) -s -C demo/object -f lsp.mak clean all
	( ./demo/object/life_safety_point >> ${LOGFILE} )
	$(MAKE) -s -C demo/object -f lsp.mak clean

ms-input: logfile demo/object/ms-input.mak
	$(MAKE) -s -C demo/object -f ms-input.mak clean all
	( ./demo/object/multistate_input >> ${LOGFILE} )
	$(MAKE) -s -C demo/object -f ms-input.mak clean

mso: logfile demo/object/mso.mak
	$(MAKE) -s -C demo/object -f mso.mak clean all
	( ./demo/object/multistate_output >> ${LOGFILE} )
	$(MAKE) -s -C demo/object -f mso.mak clean

msv: logfile demo/object/msv.mak
	$(MAKE) -s -C demo/object -f msv.mak clean all
	( ./demo/object/multistate_value >> ${LOGFILE} )
	$(MAKE) -s -C demo/object -f msv.mak clean
