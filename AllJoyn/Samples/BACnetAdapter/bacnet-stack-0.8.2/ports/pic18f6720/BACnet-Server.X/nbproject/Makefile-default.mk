#
# Generated Makefile - do not edit!
#
# Edit the Makefile in the project folder instead (../Makefile). Each target
# has a -pre and a -post target defined where you can add customized code.
#
# This makefile implements configuration specific macros and targets.


# Include project Makefile
ifeq "${IGNORE_LOCAL}" "TRUE"
# do not include local makefile. User is passing all local related variables already
else
include Makefile
# Include makefile containing local settings
ifeq "$(wildcard nbproject/Makefile-local-default.mk)" "nbproject/Makefile-local-default.mk"
include nbproject/Makefile-local-default.mk
endif
endif

# Environment
MKDIR=mkdir -p
RM=rm -f 
MV=mv 
CP=cp 

# Macros
CND_CONF=default
ifeq ($(TYPE_IMAGE), DEBUG_RUN)
IMAGE_TYPE=debug
OUTPUT_SUFFIX=cof
DEBUGGABLE_SUFFIX=cof
FINAL_IMAGE=dist/${CND_CONF}/${IMAGE_TYPE}/BACnet-Server.X.${IMAGE_TYPE}.${OUTPUT_SUFFIX}
else
IMAGE_TYPE=production
OUTPUT_SUFFIX=hex
DEBUGGABLE_SUFFIX=cof
FINAL_IMAGE=dist/${CND_CONF}/${IMAGE_TYPE}/BACnet-Server.X.${IMAGE_TYPE}.${OUTPUT_SUFFIX}
endif

# Object Directory
OBJECTDIR=build/${CND_CONF}/${IMAGE_TYPE}

# Distribution Directory
DISTDIR=dist/${CND_CONF}/${IMAGE_TYPE}

# Object Files Quoted if spaced
OBJECTFILES_QUOTED_IF_SPACED=${OBJECTDIR}/_ext/1386528437/abort.o ${OBJECTDIR}/_ext/1386528437/bacapp.o ${OBJECTDIR}/_ext/1386528437/bacdcode.o ${OBJECTDIR}/_ext/1386528437/bacerror.o ${OBJECTDIR}/_ext/1386528437/bacstr.o ${OBJECTDIR}/_ext/1386528437/crc.o ${OBJECTDIR}/_ext/1386528437/dcc.o ${OBJECTDIR}/_ext/1386528437/iam.o ${OBJECTDIR}/_ext/1386528437/rd.o ${OBJECTDIR}/_ext/1386528437/reject.o ${OBJECTDIR}/_ext/1386528437/rp.o ${OBJECTDIR}/_ext/1386528437/whois.o ${OBJECTDIR}/_ext/1394255507/h_dcc.o ${OBJECTDIR}/_ext/1394255507/h_rd.o ${OBJECTDIR}/_ext/1472/main.o ${OBJECTDIR}/_ext/1472/dlmstp.o ${OBJECTDIR}/_ext/1472/device.o ${OBJECTDIR}/_ext/1472/rs485.o ${OBJECTDIR}/_ext/1472/isr.o ${OBJECTDIR}/_ext/1386528437/datetime.o ${OBJECTDIR}/_ext/1394255507/txbuf.o ${OBJECTDIR}/_ext/1394255507/h_whois.o ${OBJECTDIR}/_ext/1472/mstp.o ${OBJECTDIR}/_ext/1472/bv.o ${OBJECTDIR}/_ext/1472/ai.o ${OBJECTDIR}/_ext/1472/bi.o ${OBJECTDIR}/_ext/1472/av.o ${OBJECTDIR}/_ext/1386528437/wp.o ${OBJECTDIR}/_ext/1394255507/h_npdu.o ${OBJECTDIR}/_ext/1394255507/s_iam.o ${OBJECTDIR}/_ext/1386528437/bacreal.o ${OBJECTDIR}/_ext/1386528437/bacint.o ${OBJECTDIR}/_ext/1386528437/npdu.o ${OBJECTDIR}/_ext/1472/apdu.o ${OBJECTDIR}/_ext/1394255507/noserv.o ${OBJECTDIR}/_ext/1386528437/fifo.o ${OBJECTDIR}/_ext/1394255507/h_rp.o ${OBJECTDIR}/_ext/1394255507/h_wp.o ${OBJECTDIR}/_ext/1386528437/bacaddr.o
POSSIBLE_DEPFILES=${OBJECTDIR}/_ext/1386528437/abort.o.d ${OBJECTDIR}/_ext/1386528437/bacapp.o.d ${OBJECTDIR}/_ext/1386528437/bacdcode.o.d ${OBJECTDIR}/_ext/1386528437/bacerror.o.d ${OBJECTDIR}/_ext/1386528437/bacstr.o.d ${OBJECTDIR}/_ext/1386528437/crc.o.d ${OBJECTDIR}/_ext/1386528437/dcc.o.d ${OBJECTDIR}/_ext/1386528437/iam.o.d ${OBJECTDIR}/_ext/1386528437/rd.o.d ${OBJECTDIR}/_ext/1386528437/reject.o.d ${OBJECTDIR}/_ext/1386528437/rp.o.d ${OBJECTDIR}/_ext/1386528437/whois.o.d ${OBJECTDIR}/_ext/1394255507/h_dcc.o.d ${OBJECTDIR}/_ext/1394255507/h_rd.o.d ${OBJECTDIR}/_ext/1472/main.o.d ${OBJECTDIR}/_ext/1472/dlmstp.o.d ${OBJECTDIR}/_ext/1472/device.o.d ${OBJECTDIR}/_ext/1472/rs485.o.d ${OBJECTDIR}/_ext/1472/isr.o.d ${OBJECTDIR}/_ext/1386528437/datetime.o.d ${OBJECTDIR}/_ext/1394255507/txbuf.o.d ${OBJECTDIR}/_ext/1394255507/h_whois.o.d ${OBJECTDIR}/_ext/1472/mstp.o.d ${OBJECTDIR}/_ext/1472/bv.o.d ${OBJECTDIR}/_ext/1472/ai.o.d ${OBJECTDIR}/_ext/1472/bi.o.d ${OBJECTDIR}/_ext/1472/av.o.d ${OBJECTDIR}/_ext/1386528437/wp.o.d ${OBJECTDIR}/_ext/1394255507/h_npdu.o.d ${OBJECTDIR}/_ext/1394255507/s_iam.o.d ${OBJECTDIR}/_ext/1386528437/bacreal.o.d ${OBJECTDIR}/_ext/1386528437/bacint.o.d ${OBJECTDIR}/_ext/1386528437/npdu.o.d ${OBJECTDIR}/_ext/1472/apdu.o.d ${OBJECTDIR}/_ext/1394255507/noserv.o.d ${OBJECTDIR}/_ext/1386528437/fifo.o.d ${OBJECTDIR}/_ext/1394255507/h_rp.o.d ${OBJECTDIR}/_ext/1394255507/h_wp.o.d ${OBJECTDIR}/_ext/1386528437/bacaddr.o.d

# Object Files
OBJECTFILES=${OBJECTDIR}/_ext/1386528437/abort.o ${OBJECTDIR}/_ext/1386528437/bacapp.o ${OBJECTDIR}/_ext/1386528437/bacdcode.o ${OBJECTDIR}/_ext/1386528437/bacerror.o ${OBJECTDIR}/_ext/1386528437/bacstr.o ${OBJECTDIR}/_ext/1386528437/crc.o ${OBJECTDIR}/_ext/1386528437/dcc.o ${OBJECTDIR}/_ext/1386528437/iam.o ${OBJECTDIR}/_ext/1386528437/rd.o ${OBJECTDIR}/_ext/1386528437/reject.o ${OBJECTDIR}/_ext/1386528437/rp.o ${OBJECTDIR}/_ext/1386528437/whois.o ${OBJECTDIR}/_ext/1394255507/h_dcc.o ${OBJECTDIR}/_ext/1394255507/h_rd.o ${OBJECTDIR}/_ext/1472/main.o ${OBJECTDIR}/_ext/1472/dlmstp.o ${OBJECTDIR}/_ext/1472/device.o ${OBJECTDIR}/_ext/1472/rs485.o ${OBJECTDIR}/_ext/1472/isr.o ${OBJECTDIR}/_ext/1386528437/datetime.o ${OBJECTDIR}/_ext/1394255507/txbuf.o ${OBJECTDIR}/_ext/1394255507/h_whois.o ${OBJECTDIR}/_ext/1472/mstp.o ${OBJECTDIR}/_ext/1472/bv.o ${OBJECTDIR}/_ext/1472/ai.o ${OBJECTDIR}/_ext/1472/bi.o ${OBJECTDIR}/_ext/1472/av.o ${OBJECTDIR}/_ext/1386528437/wp.o ${OBJECTDIR}/_ext/1394255507/h_npdu.o ${OBJECTDIR}/_ext/1394255507/s_iam.o ${OBJECTDIR}/_ext/1386528437/bacreal.o ${OBJECTDIR}/_ext/1386528437/bacint.o ${OBJECTDIR}/_ext/1386528437/npdu.o ${OBJECTDIR}/_ext/1472/apdu.o ${OBJECTDIR}/_ext/1394255507/noserv.o ${OBJECTDIR}/_ext/1386528437/fifo.o ${OBJECTDIR}/_ext/1394255507/h_rp.o ${OBJECTDIR}/_ext/1394255507/h_wp.o ${OBJECTDIR}/_ext/1386528437/bacaddr.o


CFLAGS=
ASFLAGS=
LDLIBSOPTIONS=

############# Tool locations ##########################################
# If you copy a project from one host to another, the path where the  #
# compiler is installed may be different.                             #
# If you open this project with MPLAB X in the new host, this         #
# makefile will be regenerated and the paths will be corrected.       #
#######################################################################
# fixDeps replaces a bunch of sed/cat/printf statements that slow down the build
FIXDEPS=fixDeps

.build-conf:  ${BUILD_SUBPROJECTS}
	${MAKE}  -f nbproject/Makefile-default.mk dist/${CND_CONF}/${IMAGE_TYPE}/BACnet-Server.X.${IMAGE_TYPE}.${OUTPUT_SUFFIX}

MP_PROCESSOR_OPTION=18F6720
MP_PROCESSOR_OPTION_LD=18f6720
MP_LINKER_DEBUG_OPTION= -u_DEBUGCODESTART=0x1fd30 -u_DEBUGCODELEN=0x2d0 -u_DEBUGDATASTART=0xef4 -u_DEBUGDATALEN=0xb
# ------------------------------------------------------------------------------------
# Rules for buildStep: assemble
ifeq ($(TYPE_IMAGE), DEBUG_RUN)
else
endif

# ------------------------------------------------------------------------------------
# Rules for buildStep: compile
ifeq ($(TYPE_IMAGE), DEBUG_RUN)
${OBJECTDIR}/_ext/1386528437/abort.o: ../../../src/abort.c  nbproject/Makefile-${CND_CONF}.mk
	@${MKDIR} ${OBJECTDIR}/_ext/1386528437 
	@${RM} ${OBJECTDIR}/_ext/1386528437/abort.o.d 
	${MP_CC} $(MP_EXTRA_CC_PRE) -D__DEBUG -D__MPLAB_DEBUGGER_ICD3=1 -p$(MP_PROCESSOR_OPTION) -DPRINT_ENABLED=0 -DBACDL_MSTP=1 -DBIG_ENDIAN=0 -DMAX_APDU=50 -DMAX_TSM_TRANSACTIONS=0 -DBACAPP_MINIMAL -I"../" -I"../../../include" -I"../../../demo/object" -ms -oa-  -I ${MP_CC_DIR}/../h  -fo ${OBJECTDIR}/_ext/1386528437/abort.o   ../../../src/abort.c 
	@${DEP_GEN} -d ${OBJECTDIR}/_ext/1386528437/abort.o 
	@${FIXDEPS} "${OBJECTDIR}/_ext/1386528437/abort.o.d" $(SILENT) -rsi ${MP_CC_DIR}../ -c18 
	
${OBJECTDIR}/_ext/1386528437/bacapp.o: ../../../src/bacapp.c  nbproject/Makefile-${CND_CONF}.mk
	@${MKDIR} ${OBJECTDIR}/_ext/1386528437 
	@${RM} ${OBJECTDIR}/_ext/1386528437/bacapp.o.d 
	${MP_CC} $(MP_EXTRA_CC_PRE) -D__DEBUG -D__MPLAB_DEBUGGER_ICD3=1 -p$(MP_PROCESSOR_OPTION) -DPRINT_ENABLED=0 -DBACDL_MSTP=1 -DBIG_ENDIAN=0 -DMAX_APDU=50 -DMAX_TSM_TRANSACTIONS=0 -DBACAPP_MINIMAL -I"../" -I"../../../include" -I"../../../demo/object" -ms -oa-  -I ${MP_CC_DIR}/../h  -fo ${OBJECTDIR}/_ext/1386528437/bacapp.o   ../../../src/bacapp.c 
	@${DEP_GEN} -d ${OBJECTDIR}/_ext/1386528437/bacapp.o 
	@${FIXDEPS} "${OBJECTDIR}/_ext/1386528437/bacapp.o.d" $(SILENT) -rsi ${MP_CC_DIR}../ -c18 
	
${OBJECTDIR}/_ext/1386528437/bacdcode.o: ../../../src/bacdcode.c  nbproject/Makefile-${CND_CONF}.mk
	@${MKDIR} ${OBJECTDIR}/_ext/1386528437 
	@${RM} ${OBJECTDIR}/_ext/1386528437/bacdcode.o.d 
	${MP_CC} $(MP_EXTRA_CC_PRE) -D__DEBUG -D__MPLAB_DEBUGGER_ICD3=1 -p$(MP_PROCESSOR_OPTION) -DPRINT_ENABLED=0 -DBACDL_MSTP=1 -DBIG_ENDIAN=0 -DMAX_APDU=50 -DMAX_TSM_TRANSACTIONS=0 -DBACAPP_MINIMAL -I"../" -I"../../../include" -I"../../../demo/object" -ms -oa-  -I ${MP_CC_DIR}/../h  -fo ${OBJECTDIR}/_ext/1386528437/bacdcode.o   ../../../src/bacdcode.c 
	@${DEP_GEN} -d ${OBJECTDIR}/_ext/1386528437/bacdcode.o 
	@${FIXDEPS} "${OBJECTDIR}/_ext/1386528437/bacdcode.o.d" $(SILENT) -rsi ${MP_CC_DIR}../ -c18 
	
${OBJECTDIR}/_ext/1386528437/bacerror.o: ../../../src/bacerror.c  nbproject/Makefile-${CND_CONF}.mk
	@${MKDIR} ${OBJECTDIR}/_ext/1386528437 
	@${RM} ${OBJECTDIR}/_ext/1386528437/bacerror.o.d 
	${MP_CC} $(MP_EXTRA_CC_PRE) -D__DEBUG -D__MPLAB_DEBUGGER_ICD3=1 -p$(MP_PROCESSOR_OPTION) -DPRINT_ENABLED=0 -DBACDL_MSTP=1 -DBIG_ENDIAN=0 -DMAX_APDU=50 -DMAX_TSM_TRANSACTIONS=0 -DBACAPP_MINIMAL -I"../" -I"../../../include" -I"../../../demo/object" -ms -oa-  -I ${MP_CC_DIR}/../h  -fo ${OBJECTDIR}/_ext/1386528437/bacerror.o   ../../../src/bacerror.c 
	@${DEP_GEN} -d ${OBJECTDIR}/_ext/1386528437/bacerror.o 
	@${FIXDEPS} "${OBJECTDIR}/_ext/1386528437/bacerror.o.d" $(SILENT) -rsi ${MP_CC_DIR}../ -c18 
	
${OBJECTDIR}/_ext/1386528437/bacstr.o: ../../../src/bacstr.c  nbproject/Makefile-${CND_CONF}.mk
	@${MKDIR} ${OBJECTDIR}/_ext/1386528437 
	@${RM} ${OBJECTDIR}/_ext/1386528437/bacstr.o.d 
	${MP_CC} $(MP_EXTRA_CC_PRE) -D__DEBUG -D__MPLAB_DEBUGGER_ICD3=1 -p$(MP_PROCESSOR_OPTION) -DPRINT_ENABLED=0 -DBACDL_MSTP=1 -DBIG_ENDIAN=0 -DMAX_APDU=50 -DMAX_TSM_TRANSACTIONS=0 -DBACAPP_MINIMAL -I"../" -I"../../../include" -I"../../../demo/object" -ms -oa-  -I ${MP_CC_DIR}/../h  -fo ${OBJECTDIR}/_ext/1386528437/bacstr.o   ../../../src/bacstr.c 
	@${DEP_GEN} -d ${OBJECTDIR}/_ext/1386528437/bacstr.o 
	@${FIXDEPS} "${OBJECTDIR}/_ext/1386528437/bacstr.o.d" $(SILENT) -rsi ${MP_CC_DIR}../ -c18 
	
${OBJECTDIR}/_ext/1386528437/crc.o: ../../../src/crc.c  nbproject/Makefile-${CND_CONF}.mk
	@${MKDIR} ${OBJECTDIR}/_ext/1386528437 
	@${RM} ${OBJECTDIR}/_ext/1386528437/crc.o.d 
	${MP_CC} $(MP_EXTRA_CC_PRE) -D__DEBUG -D__MPLAB_DEBUGGER_ICD3=1 -p$(MP_PROCESSOR_OPTION) -DPRINT_ENABLED=0 -DBACDL_MSTP=1 -DBIG_ENDIAN=0 -DMAX_APDU=50 -DMAX_TSM_TRANSACTIONS=0 -DBACAPP_MINIMAL -I"../" -I"../../../include" -I"../../../demo/object" -ms -oa-  -I ${MP_CC_DIR}/../h  -fo ${OBJECTDIR}/_ext/1386528437/crc.o   ../../../src/crc.c 
	@${DEP_GEN} -d ${OBJECTDIR}/_ext/1386528437/crc.o 
	@${FIXDEPS} "${OBJECTDIR}/_ext/1386528437/crc.o.d" $(SILENT) -rsi ${MP_CC_DIR}../ -c18 
	
${OBJECTDIR}/_ext/1386528437/dcc.o: ../../../src/dcc.c  nbproject/Makefile-${CND_CONF}.mk
	@${MKDIR} ${OBJECTDIR}/_ext/1386528437 
	@${RM} ${OBJECTDIR}/_ext/1386528437/dcc.o.d 
	${MP_CC} $(MP_EXTRA_CC_PRE) -D__DEBUG -D__MPLAB_DEBUGGER_ICD3=1 -p$(MP_PROCESSOR_OPTION) -DPRINT_ENABLED=0 -DBACDL_MSTP=1 -DBIG_ENDIAN=0 -DMAX_APDU=50 -DMAX_TSM_TRANSACTIONS=0 -DBACAPP_MINIMAL -I"../" -I"../../../include" -I"../../../demo/object" -ms -oa-  -I ${MP_CC_DIR}/../h  -fo ${OBJECTDIR}/_ext/1386528437/dcc.o   ../../../src/dcc.c 
	@${DEP_GEN} -d ${OBJECTDIR}/_ext/1386528437/dcc.o 
	@${FIXDEPS} "${OBJECTDIR}/_ext/1386528437/dcc.o.d" $(SILENT) -rsi ${MP_CC_DIR}../ -c18 
	
${OBJECTDIR}/_ext/1386528437/iam.o: ../../../src/iam.c  nbproject/Makefile-${CND_CONF}.mk
	@${MKDIR} ${OBJECTDIR}/_ext/1386528437 
	@${RM} ${OBJECTDIR}/_ext/1386528437/iam.o.d 
	${MP_CC} $(MP_EXTRA_CC_PRE) -D__DEBUG -D__MPLAB_DEBUGGER_ICD3=1 -p$(MP_PROCESSOR_OPTION) -DPRINT_ENABLED=0 -DBACDL_MSTP=1 -DBIG_ENDIAN=0 -DMAX_APDU=50 -DMAX_TSM_TRANSACTIONS=0 -DBACAPP_MINIMAL -I"../" -I"../../../include" -I"../../../demo/object" -ms -oa-  -I ${MP_CC_DIR}/../h  -fo ${OBJECTDIR}/_ext/1386528437/iam.o   ../../../src/iam.c 
	@${DEP_GEN} -d ${OBJECTDIR}/_ext/1386528437/iam.o 
	@${FIXDEPS} "${OBJECTDIR}/_ext/1386528437/iam.o.d" $(SILENT) -rsi ${MP_CC_DIR}../ -c18 
	
${OBJECTDIR}/_ext/1386528437/rd.o: ../../../src/rd.c  nbproject/Makefile-${CND_CONF}.mk
	@${MKDIR} ${OBJECTDIR}/_ext/1386528437 
	@${RM} ${OBJECTDIR}/_ext/1386528437/rd.o.d 
	${MP_CC} $(MP_EXTRA_CC_PRE) -D__DEBUG -D__MPLAB_DEBUGGER_ICD3=1 -p$(MP_PROCESSOR_OPTION) -DPRINT_ENABLED=0 -DBACDL_MSTP=1 -DBIG_ENDIAN=0 -DMAX_APDU=50 -DMAX_TSM_TRANSACTIONS=0 -DBACAPP_MINIMAL -I"../" -I"../../../include" -I"../../../demo/object" -ms -oa-  -I ${MP_CC_DIR}/../h  -fo ${OBJECTDIR}/_ext/1386528437/rd.o   ../../../src/rd.c 
	@${DEP_GEN} -d ${OBJECTDIR}/_ext/1386528437/rd.o 
	@${FIXDEPS} "${OBJECTDIR}/_ext/1386528437/rd.o.d" $(SILENT) -rsi ${MP_CC_DIR}../ -c18 
	
${OBJECTDIR}/_ext/1386528437/reject.o: ../../../src/reject.c  nbproject/Makefile-${CND_CONF}.mk
	@${MKDIR} ${OBJECTDIR}/_ext/1386528437 
	@${RM} ${OBJECTDIR}/_ext/1386528437/reject.o.d 
	${MP_CC} $(MP_EXTRA_CC_PRE) -D__DEBUG -D__MPLAB_DEBUGGER_ICD3=1 -p$(MP_PROCESSOR_OPTION) -DPRINT_ENABLED=0 -DBACDL_MSTP=1 -DBIG_ENDIAN=0 -DMAX_APDU=50 -DMAX_TSM_TRANSACTIONS=0 -DBACAPP_MINIMAL -I"../" -I"../../../include" -I"../../../demo/object" -ms -oa-  -I ${MP_CC_DIR}/../h  -fo ${OBJECTDIR}/_ext/1386528437/reject.o   ../../../src/reject.c 
	@${DEP_GEN} -d ${OBJECTDIR}/_ext/1386528437/reject.o 
	@${FIXDEPS} "${OBJECTDIR}/_ext/1386528437/reject.o.d" $(SILENT) -rsi ${MP_CC_DIR}../ -c18 
	
${OBJECTDIR}/_ext/1386528437/rp.o: ../../../src/rp.c  nbproject/Makefile-${CND_CONF}.mk
	@${MKDIR} ${OBJECTDIR}/_ext/1386528437 
	@${RM} ${OBJECTDIR}/_ext/1386528437/rp.o.d 
	${MP_CC} $(MP_EXTRA_CC_PRE) -D__DEBUG -D__MPLAB_DEBUGGER_ICD3=1 -p$(MP_PROCESSOR_OPTION) -DPRINT_ENABLED=0 -DBACDL_MSTP=1 -DBIG_ENDIAN=0 -DMAX_APDU=50 -DMAX_TSM_TRANSACTIONS=0 -DBACAPP_MINIMAL -I"../" -I"../../../include" -I"../../../demo/object" -ms -oa-  -I ${MP_CC_DIR}/../h  -fo ${OBJECTDIR}/_ext/1386528437/rp.o   ../../../src/rp.c 
	@${DEP_GEN} -d ${OBJECTDIR}/_ext/1386528437/rp.o 
	@${FIXDEPS} "${OBJECTDIR}/_ext/1386528437/rp.o.d" $(SILENT) -rsi ${MP_CC_DIR}../ -c18 
	
${OBJECTDIR}/_ext/1386528437/whois.o: ../../../src/whois.c  nbproject/Makefile-${CND_CONF}.mk
	@${MKDIR} ${OBJECTDIR}/_ext/1386528437 
	@${RM} ${OBJECTDIR}/_ext/1386528437/whois.o.d 
	${MP_CC} $(MP_EXTRA_CC_PRE) -D__DEBUG -D__MPLAB_DEBUGGER_ICD3=1 -p$(MP_PROCESSOR_OPTION) -DPRINT_ENABLED=0 -DBACDL_MSTP=1 -DBIG_ENDIAN=0 -DMAX_APDU=50 -DMAX_TSM_TRANSACTIONS=0 -DBACAPP_MINIMAL -I"../" -I"../../../include" -I"../../../demo/object" -ms -oa-  -I ${MP_CC_DIR}/../h  -fo ${OBJECTDIR}/_ext/1386528437/whois.o   ../../../src/whois.c 
	@${DEP_GEN} -d ${OBJECTDIR}/_ext/1386528437/whois.o 
	@${FIXDEPS} "${OBJECTDIR}/_ext/1386528437/whois.o.d" $(SILENT) -rsi ${MP_CC_DIR}../ -c18 
	
${OBJECTDIR}/_ext/1394255507/h_dcc.o: ../../../demo/handler/h_dcc.c  nbproject/Makefile-${CND_CONF}.mk
	@${MKDIR} ${OBJECTDIR}/_ext/1394255507 
	@${RM} ${OBJECTDIR}/_ext/1394255507/h_dcc.o.d 
	${MP_CC} $(MP_EXTRA_CC_PRE) -D__DEBUG -D__MPLAB_DEBUGGER_ICD3=1 -p$(MP_PROCESSOR_OPTION) -DPRINT_ENABLED=0 -DBACDL_MSTP=1 -DBIG_ENDIAN=0 -DMAX_APDU=50 -DMAX_TSM_TRANSACTIONS=0 -DBACAPP_MINIMAL -I"../" -I"../../../include" -I"../../../demo/object" -ms -oa-  -I ${MP_CC_DIR}/../h  -fo ${OBJECTDIR}/_ext/1394255507/h_dcc.o   ../../../demo/handler/h_dcc.c 
	@${DEP_GEN} -d ${OBJECTDIR}/_ext/1394255507/h_dcc.o 
	@${FIXDEPS} "${OBJECTDIR}/_ext/1394255507/h_dcc.o.d" $(SILENT) -rsi ${MP_CC_DIR}../ -c18 
	
${OBJECTDIR}/_ext/1394255507/h_rd.o: ../../../demo/handler/h_rd.c  nbproject/Makefile-${CND_CONF}.mk
	@${MKDIR} ${OBJECTDIR}/_ext/1394255507 
	@${RM} ${OBJECTDIR}/_ext/1394255507/h_rd.o.d 
	${MP_CC} $(MP_EXTRA_CC_PRE) -D__DEBUG -D__MPLAB_DEBUGGER_ICD3=1 -p$(MP_PROCESSOR_OPTION) -DPRINT_ENABLED=0 -DBACDL_MSTP=1 -DBIG_ENDIAN=0 -DMAX_APDU=50 -DMAX_TSM_TRANSACTIONS=0 -DBACAPP_MINIMAL -I"../" -I"../../../include" -I"../../../demo/object" -ms -oa-  -I ${MP_CC_DIR}/../h  -fo ${OBJECTDIR}/_ext/1394255507/h_rd.o   ../../../demo/handler/h_rd.c 
	@${DEP_GEN} -d ${OBJECTDIR}/_ext/1394255507/h_rd.o 
	@${FIXDEPS} "${OBJECTDIR}/_ext/1394255507/h_rd.o.d" $(SILENT) -rsi ${MP_CC_DIR}../ -c18 
	
${OBJECTDIR}/_ext/1472/main.o: ../main.c  nbproject/Makefile-${CND_CONF}.mk
	@${MKDIR} ${OBJECTDIR}/_ext/1472 
	@${RM} ${OBJECTDIR}/_ext/1472/main.o.d 
	${MP_CC} $(MP_EXTRA_CC_PRE) -D__DEBUG -D__MPLAB_DEBUGGER_ICD3=1 -p$(MP_PROCESSOR_OPTION) -DPRINT_ENABLED=0 -DBACDL_MSTP=1 -DBIG_ENDIAN=0 -DMAX_APDU=50 -DMAX_TSM_TRANSACTIONS=0 -DBACAPP_MINIMAL -I"../" -I"../../../include" -I"../../../demo/object" -ms -oa-  -I ${MP_CC_DIR}/../h  -fo ${OBJECTDIR}/_ext/1472/main.o   ../main.c 
	@${DEP_GEN} -d ${OBJECTDIR}/_ext/1472/main.o 
	@${FIXDEPS} "${OBJECTDIR}/_ext/1472/main.o.d" $(SILENT) -rsi ${MP_CC_DIR}../ -c18 
	
${OBJECTDIR}/_ext/1472/dlmstp.o: ../dlmstp.c  nbproject/Makefile-${CND_CONF}.mk
	@${MKDIR} ${OBJECTDIR}/_ext/1472 
	@${RM} ${OBJECTDIR}/_ext/1472/dlmstp.o.d 
	${MP_CC} $(MP_EXTRA_CC_PRE) -D__DEBUG -D__MPLAB_DEBUGGER_ICD3=1 -p$(MP_PROCESSOR_OPTION) -DPRINT_ENABLED=0 -DBACDL_MSTP=1 -DBIG_ENDIAN=0 -DMAX_APDU=50 -DMAX_TSM_TRANSACTIONS=0 -DBACAPP_MINIMAL -I"../" -I"../../../include" -I"../../../demo/object" -ms -oa-  -I ${MP_CC_DIR}/../h  -fo ${OBJECTDIR}/_ext/1472/dlmstp.o   ../dlmstp.c 
	@${DEP_GEN} -d ${OBJECTDIR}/_ext/1472/dlmstp.o 
	@${FIXDEPS} "${OBJECTDIR}/_ext/1472/dlmstp.o.d" $(SILENT) -rsi ${MP_CC_DIR}../ -c18 
	
${OBJECTDIR}/_ext/1472/device.o: ../device.c  nbproject/Makefile-${CND_CONF}.mk
	@${MKDIR} ${OBJECTDIR}/_ext/1472 
	@${RM} ${OBJECTDIR}/_ext/1472/device.o.d 
	${MP_CC} $(MP_EXTRA_CC_PRE) -D__DEBUG -D__MPLAB_DEBUGGER_ICD3=1 -p$(MP_PROCESSOR_OPTION) -DPRINT_ENABLED=0 -DBACDL_MSTP=1 -DBIG_ENDIAN=0 -DMAX_APDU=50 -DMAX_TSM_TRANSACTIONS=0 -DBACAPP_MINIMAL -I"../" -I"../../../include" -I"../../../demo/object" -ms -oa-  -I ${MP_CC_DIR}/../h  -fo ${OBJECTDIR}/_ext/1472/device.o   ../device.c 
	@${DEP_GEN} -d ${OBJECTDIR}/_ext/1472/device.o 
	@${FIXDEPS} "${OBJECTDIR}/_ext/1472/device.o.d" $(SILENT) -rsi ${MP_CC_DIR}../ -c18 
	
${OBJECTDIR}/_ext/1472/rs485.o: ../rs485.c  nbproject/Makefile-${CND_CONF}.mk
	@${MKDIR} ${OBJECTDIR}/_ext/1472 
	@${RM} ${OBJECTDIR}/_ext/1472/rs485.o.d 
	${MP_CC} $(MP_EXTRA_CC_PRE) -D__DEBUG -D__MPLAB_DEBUGGER_ICD3=1 -p$(MP_PROCESSOR_OPTION) -DPRINT_ENABLED=0 -DBACDL_MSTP=1 -DBIG_ENDIAN=0 -DMAX_APDU=50 -DMAX_TSM_TRANSACTIONS=0 -DBACAPP_MINIMAL -I"../" -I"../../../include" -I"../../../demo/object" -ms -oa-  -I ${MP_CC_DIR}/../h  -fo ${OBJECTDIR}/_ext/1472/rs485.o   ../rs485.c 
	@${DEP_GEN} -d ${OBJECTDIR}/_ext/1472/rs485.o 
	@${FIXDEPS} "${OBJECTDIR}/_ext/1472/rs485.o.d" $(SILENT) -rsi ${MP_CC_DIR}../ -c18 
	
${OBJECTDIR}/_ext/1472/isr.o: ../isr.c  nbproject/Makefile-${CND_CONF}.mk
	@${MKDIR} ${OBJECTDIR}/_ext/1472 
	@${RM} ${OBJECTDIR}/_ext/1472/isr.o.d 
	${MP_CC} $(MP_EXTRA_CC_PRE) -D__DEBUG -D__MPLAB_DEBUGGER_ICD3=1 -p$(MP_PROCESSOR_OPTION) -DPRINT_ENABLED=0 -DBACDL_MSTP=1 -DBIG_ENDIAN=0 -DMAX_APDU=50 -DMAX_TSM_TRANSACTIONS=0 -DBACAPP_MINIMAL -I"../" -I"../../../include" -I"../../../demo/object" -ms -oa-  -I ${MP_CC_DIR}/../h  -fo ${OBJECTDIR}/_ext/1472/isr.o   ../isr.c 
	@${DEP_GEN} -d ${OBJECTDIR}/_ext/1472/isr.o 
	@${FIXDEPS} "${OBJECTDIR}/_ext/1472/isr.o.d" $(SILENT) -rsi ${MP_CC_DIR}../ -c18 
	
${OBJECTDIR}/_ext/1386528437/datetime.o: ../../../src/datetime.c  nbproject/Makefile-${CND_CONF}.mk
	@${MKDIR} ${OBJECTDIR}/_ext/1386528437 
	@${RM} ${OBJECTDIR}/_ext/1386528437/datetime.o.d 
	${MP_CC} $(MP_EXTRA_CC_PRE) -D__DEBUG -D__MPLAB_DEBUGGER_ICD3=1 -p$(MP_PROCESSOR_OPTION) -DPRINT_ENABLED=0 -DBACDL_MSTP=1 -DBIG_ENDIAN=0 -DMAX_APDU=50 -DMAX_TSM_TRANSACTIONS=0 -DBACAPP_MINIMAL -I"../" -I"../../../include" -I"../../../demo/object" -ms -oa-  -I ${MP_CC_DIR}/../h  -fo ${OBJECTDIR}/_ext/1386528437/datetime.o   ../../../src/datetime.c 
	@${DEP_GEN} -d ${OBJECTDIR}/_ext/1386528437/datetime.o 
	@${FIXDEPS} "${OBJECTDIR}/_ext/1386528437/datetime.o.d" $(SILENT) -rsi ${MP_CC_DIR}../ -c18 
	
${OBJECTDIR}/_ext/1394255507/txbuf.o: ../../../demo/handler/txbuf.c  nbproject/Makefile-${CND_CONF}.mk
	@${MKDIR} ${OBJECTDIR}/_ext/1394255507 
	@${RM} ${OBJECTDIR}/_ext/1394255507/txbuf.o.d 
	${MP_CC} $(MP_EXTRA_CC_PRE) -D__DEBUG -D__MPLAB_DEBUGGER_ICD3=1 -p$(MP_PROCESSOR_OPTION) -DPRINT_ENABLED=0 -DBACDL_MSTP=1 -DBIG_ENDIAN=0 -DMAX_APDU=50 -DMAX_TSM_TRANSACTIONS=0 -DBACAPP_MINIMAL -I"../" -I"../../../include" -I"../../../demo/object" -ms -oa-  -I ${MP_CC_DIR}/../h  -fo ${OBJECTDIR}/_ext/1394255507/txbuf.o   ../../../demo/handler/txbuf.c 
	@${DEP_GEN} -d ${OBJECTDIR}/_ext/1394255507/txbuf.o 
	@${FIXDEPS} "${OBJECTDIR}/_ext/1394255507/txbuf.o.d" $(SILENT) -rsi ${MP_CC_DIR}../ -c18 
	
${OBJECTDIR}/_ext/1394255507/h_whois.o: ../../../demo/handler/h_whois.c  nbproject/Makefile-${CND_CONF}.mk
	@${MKDIR} ${OBJECTDIR}/_ext/1394255507 
	@${RM} ${OBJECTDIR}/_ext/1394255507/h_whois.o.d 
	${MP_CC} $(MP_EXTRA_CC_PRE) -D__DEBUG -D__MPLAB_DEBUGGER_ICD3=1 -p$(MP_PROCESSOR_OPTION) -DPRINT_ENABLED=0 -DBACDL_MSTP=1 -DBIG_ENDIAN=0 -DMAX_APDU=50 -DMAX_TSM_TRANSACTIONS=0 -DBACAPP_MINIMAL -I"../" -I"../../../include" -I"../../../demo/object" -ms -oa-  -I ${MP_CC_DIR}/../h  -fo ${OBJECTDIR}/_ext/1394255507/h_whois.o   ../../../demo/handler/h_whois.c 
	@${DEP_GEN} -d ${OBJECTDIR}/_ext/1394255507/h_whois.o 
	@${FIXDEPS} "${OBJECTDIR}/_ext/1394255507/h_whois.o.d" $(SILENT) -rsi ${MP_CC_DIR}../ -c18 
	
${OBJECTDIR}/_ext/1472/mstp.o: ../mstp.c  nbproject/Makefile-${CND_CONF}.mk
	@${MKDIR} ${OBJECTDIR}/_ext/1472 
	@${RM} ${OBJECTDIR}/_ext/1472/mstp.o.d 
	${MP_CC} $(MP_EXTRA_CC_PRE) -D__DEBUG -D__MPLAB_DEBUGGER_ICD3=1 -p$(MP_PROCESSOR_OPTION) -DPRINT_ENABLED=0 -DBACDL_MSTP=1 -DBIG_ENDIAN=0 -DMAX_APDU=50 -DMAX_TSM_TRANSACTIONS=0 -DBACAPP_MINIMAL -I"../" -I"../../../include" -I"../../../demo/object" -ms -oa-  -I ${MP_CC_DIR}/../h  -fo ${OBJECTDIR}/_ext/1472/mstp.o   ../mstp.c 
	@${DEP_GEN} -d ${OBJECTDIR}/_ext/1472/mstp.o 
	@${FIXDEPS} "${OBJECTDIR}/_ext/1472/mstp.o.d" $(SILENT) -rsi ${MP_CC_DIR}../ -c18 
	
${OBJECTDIR}/_ext/1472/bv.o: ../bv.c  nbproject/Makefile-${CND_CONF}.mk
	@${MKDIR} ${OBJECTDIR}/_ext/1472 
	@${RM} ${OBJECTDIR}/_ext/1472/bv.o.d 
	${MP_CC} $(MP_EXTRA_CC_PRE) -D__DEBUG -D__MPLAB_DEBUGGER_ICD3=1 -p$(MP_PROCESSOR_OPTION) -DPRINT_ENABLED=0 -DBACDL_MSTP=1 -DBIG_ENDIAN=0 -DMAX_APDU=50 -DMAX_TSM_TRANSACTIONS=0 -DBACAPP_MINIMAL -I"../" -I"../../../include" -I"../../../demo/object" -ms -oa-  -I ${MP_CC_DIR}/../h  -fo ${OBJECTDIR}/_ext/1472/bv.o   ../bv.c 
	@${DEP_GEN} -d ${OBJECTDIR}/_ext/1472/bv.o 
	@${FIXDEPS} "${OBJECTDIR}/_ext/1472/bv.o.d" $(SILENT) -rsi ${MP_CC_DIR}../ -c18 
	
${OBJECTDIR}/_ext/1472/ai.o: ../ai.c  nbproject/Makefile-${CND_CONF}.mk
	@${MKDIR} ${OBJECTDIR}/_ext/1472 
	@${RM} ${OBJECTDIR}/_ext/1472/ai.o.d 
	${MP_CC} $(MP_EXTRA_CC_PRE) -D__DEBUG -D__MPLAB_DEBUGGER_ICD3=1 -p$(MP_PROCESSOR_OPTION) -DPRINT_ENABLED=0 -DBACDL_MSTP=1 -DBIG_ENDIAN=0 -DMAX_APDU=50 -DMAX_TSM_TRANSACTIONS=0 -DBACAPP_MINIMAL -I"../" -I"../../../include" -I"../../../demo/object" -ms -oa-  -I ${MP_CC_DIR}/../h  -fo ${OBJECTDIR}/_ext/1472/ai.o   ../ai.c 
	@${DEP_GEN} -d ${OBJECTDIR}/_ext/1472/ai.o 
	@${FIXDEPS} "${OBJECTDIR}/_ext/1472/ai.o.d" $(SILENT) -rsi ${MP_CC_DIR}../ -c18 
	
${OBJECTDIR}/_ext/1472/bi.o: ../bi.c  nbproject/Makefile-${CND_CONF}.mk
	@${MKDIR} ${OBJECTDIR}/_ext/1472 
	@${RM} ${OBJECTDIR}/_ext/1472/bi.o.d 
	${MP_CC} $(MP_EXTRA_CC_PRE) -D__DEBUG -D__MPLAB_DEBUGGER_ICD3=1 -p$(MP_PROCESSOR_OPTION) -DPRINT_ENABLED=0 -DBACDL_MSTP=1 -DBIG_ENDIAN=0 -DMAX_APDU=50 -DMAX_TSM_TRANSACTIONS=0 -DBACAPP_MINIMAL -I"../" -I"../../../include" -I"../../../demo/object" -ms -oa-  -I ${MP_CC_DIR}/../h  -fo ${OBJECTDIR}/_ext/1472/bi.o   ../bi.c 
	@${DEP_GEN} -d ${OBJECTDIR}/_ext/1472/bi.o 
	@${FIXDEPS} "${OBJECTDIR}/_ext/1472/bi.o.d" $(SILENT) -rsi ${MP_CC_DIR}../ -c18 
	
${OBJECTDIR}/_ext/1472/av.o: ../av.c  nbproject/Makefile-${CND_CONF}.mk
	@${MKDIR} ${OBJECTDIR}/_ext/1472 
	@${RM} ${OBJECTDIR}/_ext/1472/av.o.d 
	${MP_CC} $(MP_EXTRA_CC_PRE) -D__DEBUG -D__MPLAB_DEBUGGER_ICD3=1 -p$(MP_PROCESSOR_OPTION) -DPRINT_ENABLED=0 -DBACDL_MSTP=1 -DBIG_ENDIAN=0 -DMAX_APDU=50 -DMAX_TSM_TRANSACTIONS=0 -DBACAPP_MINIMAL -I"../" -I"../../../include" -I"../../../demo/object" -ms -oa-  -I ${MP_CC_DIR}/../h  -fo ${OBJECTDIR}/_ext/1472/av.o   ../av.c 
	@${DEP_GEN} -d ${OBJECTDIR}/_ext/1472/av.o 
	@${FIXDEPS} "${OBJECTDIR}/_ext/1472/av.o.d" $(SILENT) -rsi ${MP_CC_DIR}../ -c18 
	
${OBJECTDIR}/_ext/1386528437/wp.o: ../../../src/wp.c  nbproject/Makefile-${CND_CONF}.mk
	@${MKDIR} ${OBJECTDIR}/_ext/1386528437 
	@${RM} ${OBJECTDIR}/_ext/1386528437/wp.o.d 
	${MP_CC} $(MP_EXTRA_CC_PRE) -D__DEBUG -D__MPLAB_DEBUGGER_ICD3=1 -p$(MP_PROCESSOR_OPTION) -DPRINT_ENABLED=0 -DBACDL_MSTP=1 -DBIG_ENDIAN=0 -DMAX_APDU=50 -DMAX_TSM_TRANSACTIONS=0 -DBACAPP_MINIMAL -I"../" -I"../../../include" -I"../../../demo/object" -ms -oa-  -I ${MP_CC_DIR}/../h  -fo ${OBJECTDIR}/_ext/1386528437/wp.o   ../../../src/wp.c 
	@${DEP_GEN} -d ${OBJECTDIR}/_ext/1386528437/wp.o 
	@${FIXDEPS} "${OBJECTDIR}/_ext/1386528437/wp.o.d" $(SILENT) -rsi ${MP_CC_DIR}../ -c18 
	
${OBJECTDIR}/_ext/1394255507/h_npdu.o: ../../../demo/handler/h_npdu.c  nbproject/Makefile-${CND_CONF}.mk
	@${MKDIR} ${OBJECTDIR}/_ext/1394255507 
	@${RM} ${OBJECTDIR}/_ext/1394255507/h_npdu.o.d 
	${MP_CC} $(MP_EXTRA_CC_PRE) -D__DEBUG -D__MPLAB_DEBUGGER_ICD3=1 -p$(MP_PROCESSOR_OPTION) -DPRINT_ENABLED=0 -DBACDL_MSTP=1 -DBIG_ENDIAN=0 -DMAX_APDU=50 -DMAX_TSM_TRANSACTIONS=0 -DBACAPP_MINIMAL -I"../" -I"../../../include" -I"../../../demo/object" -ms -oa-  -I ${MP_CC_DIR}/../h  -fo ${OBJECTDIR}/_ext/1394255507/h_npdu.o   ../../../demo/handler/h_npdu.c 
	@${DEP_GEN} -d ${OBJECTDIR}/_ext/1394255507/h_npdu.o 
	@${FIXDEPS} "${OBJECTDIR}/_ext/1394255507/h_npdu.o.d" $(SILENT) -rsi ${MP_CC_DIR}../ -c18 
	
${OBJECTDIR}/_ext/1394255507/s_iam.o: ../../../demo/handler/s_iam.c  nbproject/Makefile-${CND_CONF}.mk
	@${MKDIR} ${OBJECTDIR}/_ext/1394255507 
	@${RM} ${OBJECTDIR}/_ext/1394255507/s_iam.o.d 
	${MP_CC} $(MP_EXTRA_CC_PRE) -D__DEBUG -D__MPLAB_DEBUGGER_ICD3=1 -p$(MP_PROCESSOR_OPTION) -DPRINT_ENABLED=0 -DBACDL_MSTP=1 -DBIG_ENDIAN=0 -DMAX_APDU=50 -DMAX_TSM_TRANSACTIONS=0 -DBACAPP_MINIMAL -I"../" -I"../../../include" -I"../../../demo/object" -ms -oa-  -I ${MP_CC_DIR}/../h  -fo ${OBJECTDIR}/_ext/1394255507/s_iam.o   ../../../demo/handler/s_iam.c 
	@${DEP_GEN} -d ${OBJECTDIR}/_ext/1394255507/s_iam.o 
	@${FIXDEPS} "${OBJECTDIR}/_ext/1394255507/s_iam.o.d" $(SILENT) -rsi ${MP_CC_DIR}../ -c18 
	
${OBJECTDIR}/_ext/1386528437/bacreal.o: ../../../src/bacreal.c  nbproject/Makefile-${CND_CONF}.mk
	@${MKDIR} ${OBJECTDIR}/_ext/1386528437 
	@${RM} ${OBJECTDIR}/_ext/1386528437/bacreal.o.d 
	${MP_CC} $(MP_EXTRA_CC_PRE) -D__DEBUG -D__MPLAB_DEBUGGER_ICD3=1 -p$(MP_PROCESSOR_OPTION) -DPRINT_ENABLED=0 -DBACDL_MSTP=1 -DBIG_ENDIAN=0 -DMAX_APDU=50 -DMAX_TSM_TRANSACTIONS=0 -DBACAPP_MINIMAL -I"../" -I"../../../include" -I"../../../demo/object" -ms -oa-  -I ${MP_CC_DIR}/../h  -fo ${OBJECTDIR}/_ext/1386528437/bacreal.o   ../../../src/bacreal.c 
	@${DEP_GEN} -d ${OBJECTDIR}/_ext/1386528437/bacreal.o 
	@${FIXDEPS} "${OBJECTDIR}/_ext/1386528437/bacreal.o.d" $(SILENT) -rsi ${MP_CC_DIR}../ -c18 
	
${OBJECTDIR}/_ext/1386528437/bacint.o: ../../../src/bacint.c  nbproject/Makefile-${CND_CONF}.mk
	@${MKDIR} ${OBJECTDIR}/_ext/1386528437 
	@${RM} ${OBJECTDIR}/_ext/1386528437/bacint.o.d 
	${MP_CC} $(MP_EXTRA_CC_PRE) -D__DEBUG -D__MPLAB_DEBUGGER_ICD3=1 -p$(MP_PROCESSOR_OPTION) -DPRINT_ENABLED=0 -DBACDL_MSTP=1 -DBIG_ENDIAN=0 -DMAX_APDU=50 -DMAX_TSM_TRANSACTIONS=0 -DBACAPP_MINIMAL -I"../" -I"../../../include" -I"../../../demo/object" -ms -oa-  -I ${MP_CC_DIR}/../h  -fo ${OBJECTDIR}/_ext/1386528437/bacint.o   ../../../src/bacint.c 
	@${DEP_GEN} -d ${OBJECTDIR}/_ext/1386528437/bacint.o 
	@${FIXDEPS} "${OBJECTDIR}/_ext/1386528437/bacint.o.d" $(SILENT) -rsi ${MP_CC_DIR}../ -c18 
	
${OBJECTDIR}/_ext/1386528437/npdu.o: ../../../src/npdu.c  nbproject/Makefile-${CND_CONF}.mk
	@${MKDIR} ${OBJECTDIR}/_ext/1386528437 
	@${RM} ${OBJECTDIR}/_ext/1386528437/npdu.o.d 
	${MP_CC} $(MP_EXTRA_CC_PRE) -D__DEBUG -D__MPLAB_DEBUGGER_ICD3=1 -p$(MP_PROCESSOR_OPTION) -DPRINT_ENABLED=0 -DBACDL_MSTP=1 -DBIG_ENDIAN=0 -DMAX_APDU=50 -DMAX_TSM_TRANSACTIONS=0 -DBACAPP_MINIMAL -I"../" -I"../../../include" -I"../../../demo/object" -ms -oa-  -I ${MP_CC_DIR}/../h  -fo ${OBJECTDIR}/_ext/1386528437/npdu.o   ../../../src/npdu.c 
	@${DEP_GEN} -d ${OBJECTDIR}/_ext/1386528437/npdu.o 
	@${FIXDEPS} "${OBJECTDIR}/_ext/1386528437/npdu.o.d" $(SILENT) -rsi ${MP_CC_DIR}../ -c18 
	
${OBJECTDIR}/_ext/1472/apdu.o: ../apdu.c  nbproject/Makefile-${CND_CONF}.mk
	@${MKDIR} ${OBJECTDIR}/_ext/1472 
	@${RM} ${OBJECTDIR}/_ext/1472/apdu.o.d 
	${MP_CC} $(MP_EXTRA_CC_PRE) -D__DEBUG -D__MPLAB_DEBUGGER_ICD3=1 -p$(MP_PROCESSOR_OPTION) -DPRINT_ENABLED=0 -DBACDL_MSTP=1 -DBIG_ENDIAN=0 -DMAX_APDU=50 -DMAX_TSM_TRANSACTIONS=0 -DBACAPP_MINIMAL -I"../" -I"../../../include" -I"../../../demo/object" -ms -oa-  -I ${MP_CC_DIR}/../h  -fo ${OBJECTDIR}/_ext/1472/apdu.o   ../apdu.c 
	@${DEP_GEN} -d ${OBJECTDIR}/_ext/1472/apdu.o 
	@${FIXDEPS} "${OBJECTDIR}/_ext/1472/apdu.o.d" $(SILENT) -rsi ${MP_CC_DIR}../ -c18 
	
${OBJECTDIR}/_ext/1394255507/noserv.o: ../../../demo/handler/noserv.c  nbproject/Makefile-${CND_CONF}.mk
	@${MKDIR} ${OBJECTDIR}/_ext/1394255507 
	@${RM} ${OBJECTDIR}/_ext/1394255507/noserv.o.d 
	${MP_CC} $(MP_EXTRA_CC_PRE) -D__DEBUG -D__MPLAB_DEBUGGER_ICD3=1 -p$(MP_PROCESSOR_OPTION) -DPRINT_ENABLED=0 -DBACDL_MSTP=1 -DBIG_ENDIAN=0 -DMAX_APDU=50 -DMAX_TSM_TRANSACTIONS=0 -DBACAPP_MINIMAL -I"../" -I"../../../include" -I"../../../demo/object" -ms -oa-  -I ${MP_CC_DIR}/../h  -fo ${OBJECTDIR}/_ext/1394255507/noserv.o   ../../../demo/handler/noserv.c 
	@${DEP_GEN} -d ${OBJECTDIR}/_ext/1394255507/noserv.o 
	@${FIXDEPS} "${OBJECTDIR}/_ext/1394255507/noserv.o.d" $(SILENT) -rsi ${MP_CC_DIR}../ -c18 
	
${OBJECTDIR}/_ext/1386528437/fifo.o: ../../../src/fifo.c  nbproject/Makefile-${CND_CONF}.mk
	@${MKDIR} ${OBJECTDIR}/_ext/1386528437 
	@${RM} ${OBJECTDIR}/_ext/1386528437/fifo.o.d 
	${MP_CC} $(MP_EXTRA_CC_PRE) -D__DEBUG -D__MPLAB_DEBUGGER_ICD3=1 -p$(MP_PROCESSOR_OPTION) -DPRINT_ENABLED=0 -DBACDL_MSTP=1 -DBIG_ENDIAN=0 -DMAX_APDU=50 -DMAX_TSM_TRANSACTIONS=0 -DBACAPP_MINIMAL -I"../" -I"../../../include" -I"../../../demo/object" -ms -oa-  -I ${MP_CC_DIR}/../h  -fo ${OBJECTDIR}/_ext/1386528437/fifo.o   ../../../src/fifo.c 
	@${DEP_GEN} -d ${OBJECTDIR}/_ext/1386528437/fifo.o 
	@${FIXDEPS} "${OBJECTDIR}/_ext/1386528437/fifo.o.d" $(SILENT) -rsi ${MP_CC_DIR}../ -c18 
	
${OBJECTDIR}/_ext/1394255507/h_rp.o: ../../../demo/handler/h_rp.c  nbproject/Makefile-${CND_CONF}.mk
	@${MKDIR} ${OBJECTDIR}/_ext/1394255507 
	@${RM} ${OBJECTDIR}/_ext/1394255507/h_rp.o.d 
	${MP_CC} $(MP_EXTRA_CC_PRE) -D__DEBUG -D__MPLAB_DEBUGGER_ICD3=1 -p$(MP_PROCESSOR_OPTION) -DPRINT_ENABLED=0 -DBACDL_MSTP=1 -DBIG_ENDIAN=0 -DMAX_APDU=50 -DMAX_TSM_TRANSACTIONS=0 -DBACAPP_MINIMAL -I"../" -I"../../../include" -I"../../../demo/object" -ms -oa-  -I ${MP_CC_DIR}/../h  -fo ${OBJECTDIR}/_ext/1394255507/h_rp.o   ../../../demo/handler/h_rp.c 
	@${DEP_GEN} -d ${OBJECTDIR}/_ext/1394255507/h_rp.o 
	@${FIXDEPS} "${OBJECTDIR}/_ext/1394255507/h_rp.o.d" $(SILENT) -rsi ${MP_CC_DIR}../ -c18 
	
${OBJECTDIR}/_ext/1394255507/h_wp.o: ../../../demo/handler/h_wp.c  nbproject/Makefile-${CND_CONF}.mk
	@${MKDIR} ${OBJECTDIR}/_ext/1394255507 
	@${RM} ${OBJECTDIR}/_ext/1394255507/h_wp.o.d 
	${MP_CC} $(MP_EXTRA_CC_PRE) -D__DEBUG -D__MPLAB_DEBUGGER_ICD3=1 -p$(MP_PROCESSOR_OPTION) -DPRINT_ENABLED=0 -DBACDL_MSTP=1 -DBIG_ENDIAN=0 -DMAX_APDU=50 -DMAX_TSM_TRANSACTIONS=0 -DBACAPP_MINIMAL -I"../" -I"../../../include" -I"../../../demo/object" -ms -oa-  -I ${MP_CC_DIR}/../h  -fo ${OBJECTDIR}/_ext/1394255507/h_wp.o   ../../../demo/handler/h_wp.c 
	@${DEP_GEN} -d ${OBJECTDIR}/_ext/1394255507/h_wp.o 
	@${FIXDEPS} "${OBJECTDIR}/_ext/1394255507/h_wp.o.d" $(SILENT) -rsi ${MP_CC_DIR}../ -c18 
	
${OBJECTDIR}/_ext/1386528437/bacaddr.o: ../../../src/bacaddr.c  nbproject/Makefile-${CND_CONF}.mk
	@${MKDIR} ${OBJECTDIR}/_ext/1386528437 
	@${RM} ${OBJECTDIR}/_ext/1386528437/bacaddr.o.d 
	${MP_CC} $(MP_EXTRA_CC_PRE) -D__DEBUG -D__MPLAB_DEBUGGER_ICD3=1 -p$(MP_PROCESSOR_OPTION) -DPRINT_ENABLED=0 -DBACDL_MSTP=1 -DBIG_ENDIAN=0 -DMAX_APDU=50 -DMAX_TSM_TRANSACTIONS=0 -DBACAPP_MINIMAL -I"../" -I"../../../include" -I"../../../demo/object" -ms -oa-  -I ${MP_CC_DIR}/../h  -fo ${OBJECTDIR}/_ext/1386528437/bacaddr.o   ../../../src/bacaddr.c 
	@${DEP_GEN} -d ${OBJECTDIR}/_ext/1386528437/bacaddr.o 
	@${FIXDEPS} "${OBJECTDIR}/_ext/1386528437/bacaddr.o.d" $(SILENT) -rsi ${MP_CC_DIR}../ -c18 
	
else
${OBJECTDIR}/_ext/1386528437/abort.o: ../../../src/abort.c  nbproject/Makefile-${CND_CONF}.mk
	@${MKDIR} ${OBJECTDIR}/_ext/1386528437 
	@${RM} ${OBJECTDIR}/_ext/1386528437/abort.o.d 
	${MP_CC} $(MP_EXTRA_CC_PRE) -p$(MP_PROCESSOR_OPTION) -DPRINT_ENABLED=0 -DBACDL_MSTP=1 -DBIG_ENDIAN=0 -DMAX_APDU=50 -DMAX_TSM_TRANSACTIONS=0 -DBACAPP_MINIMAL -I"../" -I"../../../include" -I"../../../demo/object" -ms -oa-  -I ${MP_CC_DIR}/../h  -fo ${OBJECTDIR}/_ext/1386528437/abort.o   ../../../src/abort.c 
	@${DEP_GEN} -d ${OBJECTDIR}/_ext/1386528437/abort.o 
	@${FIXDEPS} "${OBJECTDIR}/_ext/1386528437/abort.o.d" $(SILENT) -rsi ${MP_CC_DIR}../ -c18 
	
${OBJECTDIR}/_ext/1386528437/bacapp.o: ../../../src/bacapp.c  nbproject/Makefile-${CND_CONF}.mk
	@${MKDIR} ${OBJECTDIR}/_ext/1386528437 
	@${RM} ${OBJECTDIR}/_ext/1386528437/bacapp.o.d 
	${MP_CC} $(MP_EXTRA_CC_PRE) -p$(MP_PROCESSOR_OPTION) -DPRINT_ENABLED=0 -DBACDL_MSTP=1 -DBIG_ENDIAN=0 -DMAX_APDU=50 -DMAX_TSM_TRANSACTIONS=0 -DBACAPP_MINIMAL -I"../" -I"../../../include" -I"../../../demo/object" -ms -oa-  -I ${MP_CC_DIR}/../h  -fo ${OBJECTDIR}/_ext/1386528437/bacapp.o   ../../../src/bacapp.c 
	@${DEP_GEN} -d ${OBJECTDIR}/_ext/1386528437/bacapp.o 
	@${FIXDEPS} "${OBJECTDIR}/_ext/1386528437/bacapp.o.d" $(SILENT) -rsi ${MP_CC_DIR}../ -c18 
	
${OBJECTDIR}/_ext/1386528437/bacdcode.o: ../../../src/bacdcode.c  nbproject/Makefile-${CND_CONF}.mk
	@${MKDIR} ${OBJECTDIR}/_ext/1386528437 
	@${RM} ${OBJECTDIR}/_ext/1386528437/bacdcode.o.d 
	${MP_CC} $(MP_EXTRA_CC_PRE) -p$(MP_PROCESSOR_OPTION) -DPRINT_ENABLED=0 -DBACDL_MSTP=1 -DBIG_ENDIAN=0 -DMAX_APDU=50 -DMAX_TSM_TRANSACTIONS=0 -DBACAPP_MINIMAL -I"../" -I"../../../include" -I"../../../demo/object" -ms -oa-  -I ${MP_CC_DIR}/../h  -fo ${OBJECTDIR}/_ext/1386528437/bacdcode.o   ../../../src/bacdcode.c 
	@${DEP_GEN} -d ${OBJECTDIR}/_ext/1386528437/bacdcode.o 
	@${FIXDEPS} "${OBJECTDIR}/_ext/1386528437/bacdcode.o.d" $(SILENT) -rsi ${MP_CC_DIR}../ -c18 
	
${OBJECTDIR}/_ext/1386528437/bacerror.o: ../../../src/bacerror.c  nbproject/Makefile-${CND_CONF}.mk
	@${MKDIR} ${OBJECTDIR}/_ext/1386528437 
	@${RM} ${OBJECTDIR}/_ext/1386528437/bacerror.o.d 
	${MP_CC} $(MP_EXTRA_CC_PRE) -p$(MP_PROCESSOR_OPTION) -DPRINT_ENABLED=0 -DBACDL_MSTP=1 -DBIG_ENDIAN=0 -DMAX_APDU=50 -DMAX_TSM_TRANSACTIONS=0 -DBACAPP_MINIMAL -I"../" -I"../../../include" -I"../../../demo/object" -ms -oa-  -I ${MP_CC_DIR}/../h  -fo ${OBJECTDIR}/_ext/1386528437/bacerror.o   ../../../src/bacerror.c 
	@${DEP_GEN} -d ${OBJECTDIR}/_ext/1386528437/bacerror.o 
	@${FIXDEPS} "${OBJECTDIR}/_ext/1386528437/bacerror.o.d" $(SILENT) -rsi ${MP_CC_DIR}../ -c18 
	
${OBJECTDIR}/_ext/1386528437/bacstr.o: ../../../src/bacstr.c  nbproject/Makefile-${CND_CONF}.mk
	@${MKDIR} ${OBJECTDIR}/_ext/1386528437 
	@${RM} ${OBJECTDIR}/_ext/1386528437/bacstr.o.d 
	${MP_CC} $(MP_EXTRA_CC_PRE) -p$(MP_PROCESSOR_OPTION) -DPRINT_ENABLED=0 -DBACDL_MSTP=1 -DBIG_ENDIAN=0 -DMAX_APDU=50 -DMAX_TSM_TRANSACTIONS=0 -DBACAPP_MINIMAL -I"../" -I"../../../include" -I"../../../demo/object" -ms -oa-  -I ${MP_CC_DIR}/../h  -fo ${OBJECTDIR}/_ext/1386528437/bacstr.o   ../../../src/bacstr.c 
	@${DEP_GEN} -d ${OBJECTDIR}/_ext/1386528437/bacstr.o 
	@${FIXDEPS} "${OBJECTDIR}/_ext/1386528437/bacstr.o.d" $(SILENT) -rsi ${MP_CC_DIR}../ -c18 
	
${OBJECTDIR}/_ext/1386528437/crc.o: ../../../src/crc.c  nbproject/Makefile-${CND_CONF}.mk
	@${MKDIR} ${OBJECTDIR}/_ext/1386528437 
	@${RM} ${OBJECTDIR}/_ext/1386528437/crc.o.d 
	${MP_CC} $(MP_EXTRA_CC_PRE) -p$(MP_PROCESSOR_OPTION) -DPRINT_ENABLED=0 -DBACDL_MSTP=1 -DBIG_ENDIAN=0 -DMAX_APDU=50 -DMAX_TSM_TRANSACTIONS=0 -DBACAPP_MINIMAL -I"../" -I"../../../include" -I"../../../demo/object" -ms -oa-  -I ${MP_CC_DIR}/../h  -fo ${OBJECTDIR}/_ext/1386528437/crc.o   ../../../src/crc.c 
	@${DEP_GEN} -d ${OBJECTDIR}/_ext/1386528437/crc.o 
	@${FIXDEPS} "${OBJECTDIR}/_ext/1386528437/crc.o.d" $(SILENT) -rsi ${MP_CC_DIR}../ -c18 
	
${OBJECTDIR}/_ext/1386528437/dcc.o: ../../../src/dcc.c  nbproject/Makefile-${CND_CONF}.mk
	@${MKDIR} ${OBJECTDIR}/_ext/1386528437 
	@${RM} ${OBJECTDIR}/_ext/1386528437/dcc.o.d 
	${MP_CC} $(MP_EXTRA_CC_PRE) -p$(MP_PROCESSOR_OPTION) -DPRINT_ENABLED=0 -DBACDL_MSTP=1 -DBIG_ENDIAN=0 -DMAX_APDU=50 -DMAX_TSM_TRANSACTIONS=0 -DBACAPP_MINIMAL -I"../" -I"../../../include" -I"../../../demo/object" -ms -oa-  -I ${MP_CC_DIR}/../h  -fo ${OBJECTDIR}/_ext/1386528437/dcc.o   ../../../src/dcc.c 
	@${DEP_GEN} -d ${OBJECTDIR}/_ext/1386528437/dcc.o 
	@${FIXDEPS} "${OBJECTDIR}/_ext/1386528437/dcc.o.d" $(SILENT) -rsi ${MP_CC_DIR}../ -c18 
	
${OBJECTDIR}/_ext/1386528437/iam.o: ../../../src/iam.c  nbproject/Makefile-${CND_CONF}.mk
	@${MKDIR} ${OBJECTDIR}/_ext/1386528437 
	@${RM} ${OBJECTDIR}/_ext/1386528437/iam.o.d 
	${MP_CC} $(MP_EXTRA_CC_PRE) -p$(MP_PROCESSOR_OPTION) -DPRINT_ENABLED=0 -DBACDL_MSTP=1 -DBIG_ENDIAN=0 -DMAX_APDU=50 -DMAX_TSM_TRANSACTIONS=0 -DBACAPP_MINIMAL -I"../" -I"../../../include" -I"../../../demo/object" -ms -oa-  -I ${MP_CC_DIR}/../h  -fo ${OBJECTDIR}/_ext/1386528437/iam.o   ../../../src/iam.c 
	@${DEP_GEN} -d ${OBJECTDIR}/_ext/1386528437/iam.o 
	@${FIXDEPS} "${OBJECTDIR}/_ext/1386528437/iam.o.d" $(SILENT) -rsi ${MP_CC_DIR}../ -c18 
	
${OBJECTDIR}/_ext/1386528437/rd.o: ../../../src/rd.c  nbproject/Makefile-${CND_CONF}.mk
	@${MKDIR} ${OBJECTDIR}/_ext/1386528437 
	@${RM} ${OBJECTDIR}/_ext/1386528437/rd.o.d 
	${MP_CC} $(MP_EXTRA_CC_PRE) -p$(MP_PROCESSOR_OPTION) -DPRINT_ENABLED=0 -DBACDL_MSTP=1 -DBIG_ENDIAN=0 -DMAX_APDU=50 -DMAX_TSM_TRANSACTIONS=0 -DBACAPP_MINIMAL -I"../" -I"../../../include" -I"../../../demo/object" -ms -oa-  -I ${MP_CC_DIR}/../h  -fo ${OBJECTDIR}/_ext/1386528437/rd.o   ../../../src/rd.c 
	@${DEP_GEN} -d ${OBJECTDIR}/_ext/1386528437/rd.o 
	@${FIXDEPS} "${OBJECTDIR}/_ext/1386528437/rd.o.d" $(SILENT) -rsi ${MP_CC_DIR}../ -c18 
	
${OBJECTDIR}/_ext/1386528437/reject.o: ../../../src/reject.c  nbproject/Makefile-${CND_CONF}.mk
	@${MKDIR} ${OBJECTDIR}/_ext/1386528437 
	@${RM} ${OBJECTDIR}/_ext/1386528437/reject.o.d 
	${MP_CC} $(MP_EXTRA_CC_PRE) -p$(MP_PROCESSOR_OPTION) -DPRINT_ENABLED=0 -DBACDL_MSTP=1 -DBIG_ENDIAN=0 -DMAX_APDU=50 -DMAX_TSM_TRANSACTIONS=0 -DBACAPP_MINIMAL -I"../" -I"../../../include" -I"../../../demo/object" -ms -oa-  -I ${MP_CC_DIR}/../h  -fo ${OBJECTDIR}/_ext/1386528437/reject.o   ../../../src/reject.c 
	@${DEP_GEN} -d ${OBJECTDIR}/_ext/1386528437/reject.o 
	@${FIXDEPS} "${OBJECTDIR}/_ext/1386528437/reject.o.d" $(SILENT) -rsi ${MP_CC_DIR}../ -c18 
	
${OBJECTDIR}/_ext/1386528437/rp.o: ../../../src/rp.c  nbproject/Makefile-${CND_CONF}.mk
	@${MKDIR} ${OBJECTDIR}/_ext/1386528437 
	@${RM} ${OBJECTDIR}/_ext/1386528437/rp.o.d 
	${MP_CC} $(MP_EXTRA_CC_PRE) -p$(MP_PROCESSOR_OPTION) -DPRINT_ENABLED=0 -DBACDL_MSTP=1 -DBIG_ENDIAN=0 -DMAX_APDU=50 -DMAX_TSM_TRANSACTIONS=0 -DBACAPP_MINIMAL -I"../" -I"../../../include" -I"../../../demo/object" -ms -oa-  -I ${MP_CC_DIR}/../h  -fo ${OBJECTDIR}/_ext/1386528437/rp.o   ../../../src/rp.c 
	@${DEP_GEN} -d ${OBJECTDIR}/_ext/1386528437/rp.o 
	@${FIXDEPS} "${OBJECTDIR}/_ext/1386528437/rp.o.d" $(SILENT) -rsi ${MP_CC_DIR}../ -c18 
	
${OBJECTDIR}/_ext/1386528437/whois.o: ../../../src/whois.c  nbproject/Makefile-${CND_CONF}.mk
	@${MKDIR} ${OBJECTDIR}/_ext/1386528437 
	@${RM} ${OBJECTDIR}/_ext/1386528437/whois.o.d 
	${MP_CC} $(MP_EXTRA_CC_PRE) -p$(MP_PROCESSOR_OPTION) -DPRINT_ENABLED=0 -DBACDL_MSTP=1 -DBIG_ENDIAN=0 -DMAX_APDU=50 -DMAX_TSM_TRANSACTIONS=0 -DBACAPP_MINIMAL -I"../" -I"../../../include" -I"../../../demo/object" -ms -oa-  -I ${MP_CC_DIR}/../h  -fo ${OBJECTDIR}/_ext/1386528437/whois.o   ../../../src/whois.c 
	@${DEP_GEN} -d ${OBJECTDIR}/_ext/1386528437/whois.o 
	@${FIXDEPS} "${OBJECTDIR}/_ext/1386528437/whois.o.d" $(SILENT) -rsi ${MP_CC_DIR}../ -c18 
	
${OBJECTDIR}/_ext/1394255507/h_dcc.o: ../../../demo/handler/h_dcc.c  nbproject/Makefile-${CND_CONF}.mk
	@${MKDIR} ${OBJECTDIR}/_ext/1394255507 
	@${RM} ${OBJECTDIR}/_ext/1394255507/h_dcc.o.d 
	${MP_CC} $(MP_EXTRA_CC_PRE) -p$(MP_PROCESSOR_OPTION) -DPRINT_ENABLED=0 -DBACDL_MSTP=1 -DBIG_ENDIAN=0 -DMAX_APDU=50 -DMAX_TSM_TRANSACTIONS=0 -DBACAPP_MINIMAL -I"../" -I"../../../include" -I"../../../demo/object" -ms -oa-  -I ${MP_CC_DIR}/../h  -fo ${OBJECTDIR}/_ext/1394255507/h_dcc.o   ../../../demo/handler/h_dcc.c 
	@${DEP_GEN} -d ${OBJECTDIR}/_ext/1394255507/h_dcc.o 
	@${FIXDEPS} "${OBJECTDIR}/_ext/1394255507/h_dcc.o.d" $(SILENT) -rsi ${MP_CC_DIR}../ -c18 
	
${OBJECTDIR}/_ext/1394255507/h_rd.o: ../../../demo/handler/h_rd.c  nbproject/Makefile-${CND_CONF}.mk
	@${MKDIR} ${OBJECTDIR}/_ext/1394255507 
	@${RM} ${OBJECTDIR}/_ext/1394255507/h_rd.o.d 
	${MP_CC} $(MP_EXTRA_CC_PRE) -p$(MP_PROCESSOR_OPTION) -DPRINT_ENABLED=0 -DBACDL_MSTP=1 -DBIG_ENDIAN=0 -DMAX_APDU=50 -DMAX_TSM_TRANSACTIONS=0 -DBACAPP_MINIMAL -I"../" -I"../../../include" -I"../../../demo/object" -ms -oa-  -I ${MP_CC_DIR}/../h  -fo ${OBJECTDIR}/_ext/1394255507/h_rd.o   ../../../demo/handler/h_rd.c 
	@${DEP_GEN} -d ${OBJECTDIR}/_ext/1394255507/h_rd.o 
	@${FIXDEPS} "${OBJECTDIR}/_ext/1394255507/h_rd.o.d" $(SILENT) -rsi ${MP_CC_DIR}../ -c18 
	
${OBJECTDIR}/_ext/1472/main.o: ../main.c  nbproject/Makefile-${CND_CONF}.mk
	@${MKDIR} ${OBJECTDIR}/_ext/1472 
	@${RM} ${OBJECTDIR}/_ext/1472/main.o.d 
	${MP_CC} $(MP_EXTRA_CC_PRE) -p$(MP_PROCESSOR_OPTION) -DPRINT_ENABLED=0 -DBACDL_MSTP=1 -DBIG_ENDIAN=0 -DMAX_APDU=50 -DMAX_TSM_TRANSACTIONS=0 -DBACAPP_MINIMAL -I"../" -I"../../../include" -I"../../../demo/object" -ms -oa-  -I ${MP_CC_DIR}/../h  -fo ${OBJECTDIR}/_ext/1472/main.o   ../main.c 
	@${DEP_GEN} -d ${OBJECTDIR}/_ext/1472/main.o 
	@${FIXDEPS} "${OBJECTDIR}/_ext/1472/main.o.d" $(SILENT) -rsi ${MP_CC_DIR}../ -c18 
	
${OBJECTDIR}/_ext/1472/dlmstp.o: ../dlmstp.c  nbproject/Makefile-${CND_CONF}.mk
	@${MKDIR} ${OBJECTDIR}/_ext/1472 
	@${RM} ${OBJECTDIR}/_ext/1472/dlmstp.o.d 
	${MP_CC} $(MP_EXTRA_CC_PRE) -p$(MP_PROCESSOR_OPTION) -DPRINT_ENABLED=0 -DBACDL_MSTP=1 -DBIG_ENDIAN=0 -DMAX_APDU=50 -DMAX_TSM_TRANSACTIONS=0 -DBACAPP_MINIMAL -I"../" -I"../../../include" -I"../../../demo/object" -ms -oa-  -I ${MP_CC_DIR}/../h  -fo ${OBJECTDIR}/_ext/1472/dlmstp.o   ../dlmstp.c 
	@${DEP_GEN} -d ${OBJECTDIR}/_ext/1472/dlmstp.o 
	@${FIXDEPS} "${OBJECTDIR}/_ext/1472/dlmstp.o.d" $(SILENT) -rsi ${MP_CC_DIR}../ -c18 
	
${OBJECTDIR}/_ext/1472/device.o: ../device.c  nbproject/Makefile-${CND_CONF}.mk
	@${MKDIR} ${OBJECTDIR}/_ext/1472 
	@${RM} ${OBJECTDIR}/_ext/1472/device.o.d 
	${MP_CC} $(MP_EXTRA_CC_PRE) -p$(MP_PROCESSOR_OPTION) -DPRINT_ENABLED=0 -DBACDL_MSTP=1 -DBIG_ENDIAN=0 -DMAX_APDU=50 -DMAX_TSM_TRANSACTIONS=0 -DBACAPP_MINIMAL -I"../" -I"../../../include" -I"../../../demo/object" -ms -oa-  -I ${MP_CC_DIR}/../h  -fo ${OBJECTDIR}/_ext/1472/device.o   ../device.c 
	@${DEP_GEN} -d ${OBJECTDIR}/_ext/1472/device.o 
	@${FIXDEPS} "${OBJECTDIR}/_ext/1472/device.o.d" $(SILENT) -rsi ${MP_CC_DIR}../ -c18 
	
${OBJECTDIR}/_ext/1472/rs485.o: ../rs485.c  nbproject/Makefile-${CND_CONF}.mk
	@${MKDIR} ${OBJECTDIR}/_ext/1472 
	@${RM} ${OBJECTDIR}/_ext/1472/rs485.o.d 
	${MP_CC} $(MP_EXTRA_CC_PRE) -p$(MP_PROCESSOR_OPTION) -DPRINT_ENABLED=0 -DBACDL_MSTP=1 -DBIG_ENDIAN=0 -DMAX_APDU=50 -DMAX_TSM_TRANSACTIONS=0 -DBACAPP_MINIMAL -I"../" -I"../../../include" -I"../../../demo/object" -ms -oa-  -I ${MP_CC_DIR}/../h  -fo ${OBJECTDIR}/_ext/1472/rs485.o   ../rs485.c 
	@${DEP_GEN} -d ${OBJECTDIR}/_ext/1472/rs485.o 
	@${FIXDEPS} "${OBJECTDIR}/_ext/1472/rs485.o.d" $(SILENT) -rsi ${MP_CC_DIR}../ -c18 
	
${OBJECTDIR}/_ext/1472/isr.o: ../isr.c  nbproject/Makefile-${CND_CONF}.mk
	@${MKDIR} ${OBJECTDIR}/_ext/1472 
	@${RM} ${OBJECTDIR}/_ext/1472/isr.o.d 
	${MP_CC} $(MP_EXTRA_CC_PRE) -p$(MP_PROCESSOR_OPTION) -DPRINT_ENABLED=0 -DBACDL_MSTP=1 -DBIG_ENDIAN=0 -DMAX_APDU=50 -DMAX_TSM_TRANSACTIONS=0 -DBACAPP_MINIMAL -I"../" -I"../../../include" -I"../../../demo/object" -ms -oa-  -I ${MP_CC_DIR}/../h  -fo ${OBJECTDIR}/_ext/1472/isr.o   ../isr.c 
	@${DEP_GEN} -d ${OBJECTDIR}/_ext/1472/isr.o 
	@${FIXDEPS} "${OBJECTDIR}/_ext/1472/isr.o.d" $(SILENT) -rsi ${MP_CC_DIR}../ -c18 
	
${OBJECTDIR}/_ext/1386528437/datetime.o: ../../../src/datetime.c  nbproject/Makefile-${CND_CONF}.mk
	@${MKDIR} ${OBJECTDIR}/_ext/1386528437 
	@${RM} ${OBJECTDIR}/_ext/1386528437/datetime.o.d 
	${MP_CC} $(MP_EXTRA_CC_PRE) -p$(MP_PROCESSOR_OPTION) -DPRINT_ENABLED=0 -DBACDL_MSTP=1 -DBIG_ENDIAN=0 -DMAX_APDU=50 -DMAX_TSM_TRANSACTIONS=0 -DBACAPP_MINIMAL -I"../" -I"../../../include" -I"../../../demo/object" -ms -oa-  -I ${MP_CC_DIR}/../h  -fo ${OBJECTDIR}/_ext/1386528437/datetime.o   ../../../src/datetime.c 
	@${DEP_GEN} -d ${OBJECTDIR}/_ext/1386528437/datetime.o 
	@${FIXDEPS} "${OBJECTDIR}/_ext/1386528437/datetime.o.d" $(SILENT) -rsi ${MP_CC_DIR}../ -c18 
	
${OBJECTDIR}/_ext/1394255507/txbuf.o: ../../../demo/handler/txbuf.c  nbproject/Makefile-${CND_CONF}.mk
	@${MKDIR} ${OBJECTDIR}/_ext/1394255507 
	@${RM} ${OBJECTDIR}/_ext/1394255507/txbuf.o.d 
	${MP_CC} $(MP_EXTRA_CC_PRE) -p$(MP_PROCESSOR_OPTION) -DPRINT_ENABLED=0 -DBACDL_MSTP=1 -DBIG_ENDIAN=0 -DMAX_APDU=50 -DMAX_TSM_TRANSACTIONS=0 -DBACAPP_MINIMAL -I"../" -I"../../../include" -I"../../../demo/object" -ms -oa-  -I ${MP_CC_DIR}/../h  -fo ${OBJECTDIR}/_ext/1394255507/txbuf.o   ../../../demo/handler/txbuf.c 
	@${DEP_GEN} -d ${OBJECTDIR}/_ext/1394255507/txbuf.o 
	@${FIXDEPS} "${OBJECTDIR}/_ext/1394255507/txbuf.o.d" $(SILENT) -rsi ${MP_CC_DIR}../ -c18 
	
${OBJECTDIR}/_ext/1394255507/h_whois.o: ../../../demo/handler/h_whois.c  nbproject/Makefile-${CND_CONF}.mk
	@${MKDIR} ${OBJECTDIR}/_ext/1394255507 
	@${RM} ${OBJECTDIR}/_ext/1394255507/h_whois.o.d 
	${MP_CC} $(MP_EXTRA_CC_PRE) -p$(MP_PROCESSOR_OPTION) -DPRINT_ENABLED=0 -DBACDL_MSTP=1 -DBIG_ENDIAN=0 -DMAX_APDU=50 -DMAX_TSM_TRANSACTIONS=0 -DBACAPP_MINIMAL -I"../" -I"../../../include" -I"../../../demo/object" -ms -oa-  -I ${MP_CC_DIR}/../h  -fo ${OBJECTDIR}/_ext/1394255507/h_whois.o   ../../../demo/handler/h_whois.c 
	@${DEP_GEN} -d ${OBJECTDIR}/_ext/1394255507/h_whois.o 
	@${FIXDEPS} "${OBJECTDIR}/_ext/1394255507/h_whois.o.d" $(SILENT) -rsi ${MP_CC_DIR}../ -c18 
	
${OBJECTDIR}/_ext/1472/mstp.o: ../mstp.c  nbproject/Makefile-${CND_CONF}.mk
	@${MKDIR} ${OBJECTDIR}/_ext/1472 
	@${RM} ${OBJECTDIR}/_ext/1472/mstp.o.d 
	${MP_CC} $(MP_EXTRA_CC_PRE) -p$(MP_PROCESSOR_OPTION) -DPRINT_ENABLED=0 -DBACDL_MSTP=1 -DBIG_ENDIAN=0 -DMAX_APDU=50 -DMAX_TSM_TRANSACTIONS=0 -DBACAPP_MINIMAL -I"../" -I"../../../include" -I"../../../demo/object" -ms -oa-  -I ${MP_CC_DIR}/../h  -fo ${OBJECTDIR}/_ext/1472/mstp.o   ../mstp.c 
	@${DEP_GEN} -d ${OBJECTDIR}/_ext/1472/mstp.o 
	@${FIXDEPS} "${OBJECTDIR}/_ext/1472/mstp.o.d" $(SILENT) -rsi ${MP_CC_DIR}../ -c18 
	
${OBJECTDIR}/_ext/1472/bv.o: ../bv.c  nbproject/Makefile-${CND_CONF}.mk
	@${MKDIR} ${OBJECTDIR}/_ext/1472 
	@${RM} ${OBJECTDIR}/_ext/1472/bv.o.d 
	${MP_CC} $(MP_EXTRA_CC_PRE) -p$(MP_PROCESSOR_OPTION) -DPRINT_ENABLED=0 -DBACDL_MSTP=1 -DBIG_ENDIAN=0 -DMAX_APDU=50 -DMAX_TSM_TRANSACTIONS=0 -DBACAPP_MINIMAL -I"../" -I"../../../include" -I"../../../demo/object" -ms -oa-  -I ${MP_CC_DIR}/../h  -fo ${OBJECTDIR}/_ext/1472/bv.o   ../bv.c 
	@${DEP_GEN} -d ${OBJECTDIR}/_ext/1472/bv.o 
	@${FIXDEPS} "${OBJECTDIR}/_ext/1472/bv.o.d" $(SILENT) -rsi ${MP_CC_DIR}../ -c18 
	
${OBJECTDIR}/_ext/1472/ai.o: ../ai.c  nbproject/Makefile-${CND_CONF}.mk
	@${MKDIR} ${OBJECTDIR}/_ext/1472 
	@${RM} ${OBJECTDIR}/_ext/1472/ai.o.d 
	${MP_CC} $(MP_EXTRA_CC_PRE) -p$(MP_PROCESSOR_OPTION) -DPRINT_ENABLED=0 -DBACDL_MSTP=1 -DBIG_ENDIAN=0 -DMAX_APDU=50 -DMAX_TSM_TRANSACTIONS=0 -DBACAPP_MINIMAL -I"../" -I"../../../include" -I"../../../demo/object" -ms -oa-  -I ${MP_CC_DIR}/../h  -fo ${OBJECTDIR}/_ext/1472/ai.o   ../ai.c 
	@${DEP_GEN} -d ${OBJECTDIR}/_ext/1472/ai.o 
	@${FIXDEPS} "${OBJECTDIR}/_ext/1472/ai.o.d" $(SILENT) -rsi ${MP_CC_DIR}../ -c18 
	
${OBJECTDIR}/_ext/1472/bi.o: ../bi.c  nbproject/Makefile-${CND_CONF}.mk
	@${MKDIR} ${OBJECTDIR}/_ext/1472 
	@${RM} ${OBJECTDIR}/_ext/1472/bi.o.d 
	${MP_CC} $(MP_EXTRA_CC_PRE) -p$(MP_PROCESSOR_OPTION) -DPRINT_ENABLED=0 -DBACDL_MSTP=1 -DBIG_ENDIAN=0 -DMAX_APDU=50 -DMAX_TSM_TRANSACTIONS=0 -DBACAPP_MINIMAL -I"../" -I"../../../include" -I"../../../demo/object" -ms -oa-  -I ${MP_CC_DIR}/../h  -fo ${OBJECTDIR}/_ext/1472/bi.o   ../bi.c 
	@${DEP_GEN} -d ${OBJECTDIR}/_ext/1472/bi.o 
	@${FIXDEPS} "${OBJECTDIR}/_ext/1472/bi.o.d" $(SILENT) -rsi ${MP_CC_DIR}../ -c18 
	
${OBJECTDIR}/_ext/1472/av.o: ../av.c  nbproject/Makefile-${CND_CONF}.mk
	@${MKDIR} ${OBJECTDIR}/_ext/1472 
	@${RM} ${OBJECTDIR}/_ext/1472/av.o.d 
	${MP_CC} $(MP_EXTRA_CC_PRE) -p$(MP_PROCESSOR_OPTION) -DPRINT_ENABLED=0 -DBACDL_MSTP=1 -DBIG_ENDIAN=0 -DMAX_APDU=50 -DMAX_TSM_TRANSACTIONS=0 -DBACAPP_MINIMAL -I"../" -I"../../../include" -I"../../../demo/object" -ms -oa-  -I ${MP_CC_DIR}/../h  -fo ${OBJECTDIR}/_ext/1472/av.o   ../av.c 
	@${DEP_GEN} -d ${OBJECTDIR}/_ext/1472/av.o 
	@${FIXDEPS} "${OBJECTDIR}/_ext/1472/av.o.d" $(SILENT) -rsi ${MP_CC_DIR}../ -c18 
	
${OBJECTDIR}/_ext/1386528437/wp.o: ../../../src/wp.c  nbproject/Makefile-${CND_CONF}.mk
	@${MKDIR} ${OBJECTDIR}/_ext/1386528437 
	@${RM} ${OBJECTDIR}/_ext/1386528437/wp.o.d 
	${MP_CC} $(MP_EXTRA_CC_PRE) -p$(MP_PROCESSOR_OPTION) -DPRINT_ENABLED=0 -DBACDL_MSTP=1 -DBIG_ENDIAN=0 -DMAX_APDU=50 -DMAX_TSM_TRANSACTIONS=0 -DBACAPP_MINIMAL -I"../" -I"../../../include" -I"../../../demo/object" -ms -oa-  -I ${MP_CC_DIR}/../h  -fo ${OBJECTDIR}/_ext/1386528437/wp.o   ../../../src/wp.c 
	@${DEP_GEN} -d ${OBJECTDIR}/_ext/1386528437/wp.o 
	@${FIXDEPS} "${OBJECTDIR}/_ext/1386528437/wp.o.d" $(SILENT) -rsi ${MP_CC_DIR}../ -c18 
	
${OBJECTDIR}/_ext/1394255507/h_npdu.o: ../../../demo/handler/h_npdu.c  nbproject/Makefile-${CND_CONF}.mk
	@${MKDIR} ${OBJECTDIR}/_ext/1394255507 
	@${RM} ${OBJECTDIR}/_ext/1394255507/h_npdu.o.d 
	${MP_CC} $(MP_EXTRA_CC_PRE) -p$(MP_PROCESSOR_OPTION) -DPRINT_ENABLED=0 -DBACDL_MSTP=1 -DBIG_ENDIAN=0 -DMAX_APDU=50 -DMAX_TSM_TRANSACTIONS=0 -DBACAPP_MINIMAL -I"../" -I"../../../include" -I"../../../demo/object" -ms -oa-  -I ${MP_CC_DIR}/../h  -fo ${OBJECTDIR}/_ext/1394255507/h_npdu.o   ../../../demo/handler/h_npdu.c 
	@${DEP_GEN} -d ${OBJECTDIR}/_ext/1394255507/h_npdu.o 
	@${FIXDEPS} "${OBJECTDIR}/_ext/1394255507/h_npdu.o.d" $(SILENT) -rsi ${MP_CC_DIR}../ -c18 
	
${OBJECTDIR}/_ext/1394255507/s_iam.o: ../../../demo/handler/s_iam.c  nbproject/Makefile-${CND_CONF}.mk
	@${MKDIR} ${OBJECTDIR}/_ext/1394255507 
	@${RM} ${OBJECTDIR}/_ext/1394255507/s_iam.o.d 
	${MP_CC} $(MP_EXTRA_CC_PRE) -p$(MP_PROCESSOR_OPTION) -DPRINT_ENABLED=0 -DBACDL_MSTP=1 -DBIG_ENDIAN=0 -DMAX_APDU=50 -DMAX_TSM_TRANSACTIONS=0 -DBACAPP_MINIMAL -I"../" -I"../../../include" -I"../../../demo/object" -ms -oa-  -I ${MP_CC_DIR}/../h  -fo ${OBJECTDIR}/_ext/1394255507/s_iam.o   ../../../demo/handler/s_iam.c 
	@${DEP_GEN} -d ${OBJECTDIR}/_ext/1394255507/s_iam.o 
	@${FIXDEPS} "${OBJECTDIR}/_ext/1394255507/s_iam.o.d" $(SILENT) -rsi ${MP_CC_DIR}../ -c18 
	
${OBJECTDIR}/_ext/1386528437/bacreal.o: ../../../src/bacreal.c  nbproject/Makefile-${CND_CONF}.mk
	@${MKDIR} ${OBJECTDIR}/_ext/1386528437 
	@${RM} ${OBJECTDIR}/_ext/1386528437/bacreal.o.d 
	${MP_CC} $(MP_EXTRA_CC_PRE) -p$(MP_PROCESSOR_OPTION) -DPRINT_ENABLED=0 -DBACDL_MSTP=1 -DBIG_ENDIAN=0 -DMAX_APDU=50 -DMAX_TSM_TRANSACTIONS=0 -DBACAPP_MINIMAL -I"../" -I"../../../include" -I"../../../demo/object" -ms -oa-  -I ${MP_CC_DIR}/../h  -fo ${OBJECTDIR}/_ext/1386528437/bacreal.o   ../../../src/bacreal.c 
	@${DEP_GEN} -d ${OBJECTDIR}/_ext/1386528437/bacreal.o 
	@${FIXDEPS} "${OBJECTDIR}/_ext/1386528437/bacreal.o.d" $(SILENT) -rsi ${MP_CC_DIR}../ -c18 
	
${OBJECTDIR}/_ext/1386528437/bacint.o: ../../../src/bacint.c  nbproject/Makefile-${CND_CONF}.mk
	@${MKDIR} ${OBJECTDIR}/_ext/1386528437 
	@${RM} ${OBJECTDIR}/_ext/1386528437/bacint.o.d 
	${MP_CC} $(MP_EXTRA_CC_PRE) -p$(MP_PROCESSOR_OPTION) -DPRINT_ENABLED=0 -DBACDL_MSTP=1 -DBIG_ENDIAN=0 -DMAX_APDU=50 -DMAX_TSM_TRANSACTIONS=0 -DBACAPP_MINIMAL -I"../" -I"../../../include" -I"../../../demo/object" -ms -oa-  -I ${MP_CC_DIR}/../h  -fo ${OBJECTDIR}/_ext/1386528437/bacint.o   ../../../src/bacint.c 
	@${DEP_GEN} -d ${OBJECTDIR}/_ext/1386528437/bacint.o 
	@${FIXDEPS} "${OBJECTDIR}/_ext/1386528437/bacint.o.d" $(SILENT) -rsi ${MP_CC_DIR}../ -c18 
	
${OBJECTDIR}/_ext/1386528437/npdu.o: ../../../src/npdu.c  nbproject/Makefile-${CND_CONF}.mk
	@${MKDIR} ${OBJECTDIR}/_ext/1386528437 
	@${RM} ${OBJECTDIR}/_ext/1386528437/npdu.o.d 
	${MP_CC} $(MP_EXTRA_CC_PRE) -p$(MP_PROCESSOR_OPTION) -DPRINT_ENABLED=0 -DBACDL_MSTP=1 -DBIG_ENDIAN=0 -DMAX_APDU=50 -DMAX_TSM_TRANSACTIONS=0 -DBACAPP_MINIMAL -I"../" -I"../../../include" -I"../../../demo/object" -ms -oa-  -I ${MP_CC_DIR}/../h  -fo ${OBJECTDIR}/_ext/1386528437/npdu.o   ../../../src/npdu.c 
	@${DEP_GEN} -d ${OBJECTDIR}/_ext/1386528437/npdu.o 
	@${FIXDEPS} "${OBJECTDIR}/_ext/1386528437/npdu.o.d" $(SILENT) -rsi ${MP_CC_DIR}../ -c18 
	
${OBJECTDIR}/_ext/1472/apdu.o: ../apdu.c  nbproject/Makefile-${CND_CONF}.mk
	@${MKDIR} ${OBJECTDIR}/_ext/1472 
	@${RM} ${OBJECTDIR}/_ext/1472/apdu.o.d 
	${MP_CC} $(MP_EXTRA_CC_PRE) -p$(MP_PROCESSOR_OPTION) -DPRINT_ENABLED=0 -DBACDL_MSTP=1 -DBIG_ENDIAN=0 -DMAX_APDU=50 -DMAX_TSM_TRANSACTIONS=0 -DBACAPP_MINIMAL -I"../" -I"../../../include" -I"../../../demo/object" -ms -oa-  -I ${MP_CC_DIR}/../h  -fo ${OBJECTDIR}/_ext/1472/apdu.o   ../apdu.c 
	@${DEP_GEN} -d ${OBJECTDIR}/_ext/1472/apdu.o 
	@${FIXDEPS} "${OBJECTDIR}/_ext/1472/apdu.o.d" $(SILENT) -rsi ${MP_CC_DIR}../ -c18 
	
${OBJECTDIR}/_ext/1394255507/noserv.o: ../../../demo/handler/noserv.c  nbproject/Makefile-${CND_CONF}.mk
	@${MKDIR} ${OBJECTDIR}/_ext/1394255507 
	@${RM} ${OBJECTDIR}/_ext/1394255507/noserv.o.d 
	${MP_CC} $(MP_EXTRA_CC_PRE) -p$(MP_PROCESSOR_OPTION) -DPRINT_ENABLED=0 -DBACDL_MSTP=1 -DBIG_ENDIAN=0 -DMAX_APDU=50 -DMAX_TSM_TRANSACTIONS=0 -DBACAPP_MINIMAL -I"../" -I"../../../include" -I"../../../demo/object" -ms -oa-  -I ${MP_CC_DIR}/../h  -fo ${OBJECTDIR}/_ext/1394255507/noserv.o   ../../../demo/handler/noserv.c 
	@${DEP_GEN} -d ${OBJECTDIR}/_ext/1394255507/noserv.o 
	@${FIXDEPS} "${OBJECTDIR}/_ext/1394255507/noserv.o.d" $(SILENT) -rsi ${MP_CC_DIR}../ -c18 
	
${OBJECTDIR}/_ext/1386528437/fifo.o: ../../../src/fifo.c  nbproject/Makefile-${CND_CONF}.mk
	@${MKDIR} ${OBJECTDIR}/_ext/1386528437 
	@${RM} ${OBJECTDIR}/_ext/1386528437/fifo.o.d 
	${MP_CC} $(MP_EXTRA_CC_PRE) -p$(MP_PROCESSOR_OPTION) -DPRINT_ENABLED=0 -DBACDL_MSTP=1 -DBIG_ENDIAN=0 -DMAX_APDU=50 -DMAX_TSM_TRANSACTIONS=0 -DBACAPP_MINIMAL -I"../" -I"../../../include" -I"../../../demo/object" -ms -oa-  -I ${MP_CC_DIR}/../h  -fo ${OBJECTDIR}/_ext/1386528437/fifo.o   ../../../src/fifo.c 
	@${DEP_GEN} -d ${OBJECTDIR}/_ext/1386528437/fifo.o 
	@${FIXDEPS} "${OBJECTDIR}/_ext/1386528437/fifo.o.d" $(SILENT) -rsi ${MP_CC_DIR}../ -c18 
	
${OBJECTDIR}/_ext/1394255507/h_rp.o: ../../../demo/handler/h_rp.c  nbproject/Makefile-${CND_CONF}.mk
	@${MKDIR} ${OBJECTDIR}/_ext/1394255507 
	@${RM} ${OBJECTDIR}/_ext/1394255507/h_rp.o.d 
	${MP_CC} $(MP_EXTRA_CC_PRE) -p$(MP_PROCESSOR_OPTION) -DPRINT_ENABLED=0 -DBACDL_MSTP=1 -DBIG_ENDIAN=0 -DMAX_APDU=50 -DMAX_TSM_TRANSACTIONS=0 -DBACAPP_MINIMAL -I"../" -I"../../../include" -I"../../../demo/object" -ms -oa-  -I ${MP_CC_DIR}/../h  -fo ${OBJECTDIR}/_ext/1394255507/h_rp.o   ../../../demo/handler/h_rp.c 
	@${DEP_GEN} -d ${OBJECTDIR}/_ext/1394255507/h_rp.o 
	@${FIXDEPS} "${OBJECTDIR}/_ext/1394255507/h_rp.o.d" $(SILENT) -rsi ${MP_CC_DIR}../ -c18 
	
${OBJECTDIR}/_ext/1394255507/h_wp.o: ../../../demo/handler/h_wp.c  nbproject/Makefile-${CND_CONF}.mk
	@${MKDIR} ${OBJECTDIR}/_ext/1394255507 
	@${RM} ${OBJECTDIR}/_ext/1394255507/h_wp.o.d 
	${MP_CC} $(MP_EXTRA_CC_PRE) -p$(MP_PROCESSOR_OPTION) -DPRINT_ENABLED=0 -DBACDL_MSTP=1 -DBIG_ENDIAN=0 -DMAX_APDU=50 -DMAX_TSM_TRANSACTIONS=0 -DBACAPP_MINIMAL -I"../" -I"../../../include" -I"../../../demo/object" -ms -oa-  -I ${MP_CC_DIR}/../h  -fo ${OBJECTDIR}/_ext/1394255507/h_wp.o   ../../../demo/handler/h_wp.c 
	@${DEP_GEN} -d ${OBJECTDIR}/_ext/1394255507/h_wp.o 
	@${FIXDEPS} "${OBJECTDIR}/_ext/1394255507/h_wp.o.d" $(SILENT) -rsi ${MP_CC_DIR}../ -c18 
	
${OBJECTDIR}/_ext/1386528437/bacaddr.o: ../../../src/bacaddr.c  nbproject/Makefile-${CND_CONF}.mk
	@${MKDIR} ${OBJECTDIR}/_ext/1386528437 
	@${RM} ${OBJECTDIR}/_ext/1386528437/bacaddr.o.d 
	${MP_CC} $(MP_EXTRA_CC_PRE) -p$(MP_PROCESSOR_OPTION) -DPRINT_ENABLED=0 -DBACDL_MSTP=1 -DBIG_ENDIAN=0 -DMAX_APDU=50 -DMAX_TSM_TRANSACTIONS=0 -DBACAPP_MINIMAL -I"../" -I"../../../include" -I"../../../demo/object" -ms -oa-  -I ${MP_CC_DIR}/../h  -fo ${OBJECTDIR}/_ext/1386528437/bacaddr.o   ../../../src/bacaddr.c 
	@${DEP_GEN} -d ${OBJECTDIR}/_ext/1386528437/bacaddr.o 
	@${FIXDEPS} "${OBJECTDIR}/_ext/1386528437/bacaddr.o.d" $(SILENT) -rsi ${MP_CC_DIR}../ -c18 
	
endif

# ------------------------------------------------------------------------------------
# Rules for buildStep: link
ifeq ($(TYPE_IMAGE), DEBUG_RUN)
dist/${CND_CONF}/${IMAGE_TYPE}/BACnet-Server.X.${IMAGE_TYPE}.${OUTPUT_SUFFIX}: ${OBJECTFILES}  nbproject/Makefile-${CND_CONF}.mk    ../18F6720.lkr
	@${MKDIR} dist/${CND_CONF}/${IMAGE_TYPE} 
	${MP_LD} $(MP_EXTRA_LD_PRE) "../18F6720.lkr"  -p$(MP_PROCESSOR_OPTION_LD)  -w -x -u_DEBUG   -z__MPLAB_BUILD=1  -u_CRUNTIME -z__MPLAB_DEBUG=1 -z__MPLAB_DEBUGGER_ICD3=1 $(MP_LINKER_DEBUG_OPTION) -l ${MP_CC_DIR}/../lib  -o dist/${CND_CONF}/${IMAGE_TYPE}/BACnet-Server.X.${IMAGE_TYPE}.${OUTPUT_SUFFIX}  ${OBJECTFILES_QUOTED_IF_SPACED}   
else
dist/${CND_CONF}/${IMAGE_TYPE}/BACnet-Server.X.${IMAGE_TYPE}.${OUTPUT_SUFFIX}: ${OBJECTFILES}  nbproject/Makefile-${CND_CONF}.mk   ../18F6720.lkr
	@${MKDIR} dist/${CND_CONF}/${IMAGE_TYPE} 
	${MP_LD} $(MP_EXTRA_LD_PRE) "../18F6720.lkr"  -p$(MP_PROCESSOR_OPTION_LD)  -w    -z__MPLAB_BUILD=1  -u_CRUNTIME -l ${MP_CC_DIR}/../lib  -o dist/${CND_CONF}/${IMAGE_TYPE}/BACnet-Server.X.${IMAGE_TYPE}.${DEBUGGABLE_SUFFIX}  ${OBJECTFILES_QUOTED_IF_SPACED}   
endif


# Subprojects
.build-subprojects:


# Subprojects
.clean-subprojects:

# Clean Targets
.clean-conf: ${CLEAN_SUBPROJECTS}
	${RM} -r build/default
	${RM} -r dist/default

# Enable dependency checking
.dep.inc: .depcheck-impl

DEPFILES=$(shell "${PATH_TO_IDE_BIN}"mplabwildcard ${POSSIBLE_DEPFILES})
ifneq (${DEPFILES},)
include ${DEPFILES}
endif
