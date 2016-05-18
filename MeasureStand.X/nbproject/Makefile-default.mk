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
OUTPUT_SUFFIX=elf
DEBUGGABLE_SUFFIX=elf
FINAL_IMAGE=dist/${CND_CONF}/${IMAGE_TYPE}/MeasureStand.X.${IMAGE_TYPE}.${OUTPUT_SUFFIX}
else
IMAGE_TYPE=production
OUTPUT_SUFFIX=hex
DEBUGGABLE_SUFFIX=elf
FINAL_IMAGE=dist/${CND_CONF}/${IMAGE_TYPE}/MeasureStand.X.${IMAGE_TYPE}.${OUTPUT_SUFFIX}
endif

# Object Directory
OBJECTDIR=build/${CND_CONF}/${IMAGE_TYPE}

# Distribution Directory
DISTDIR=dist/${CND_CONF}/${IMAGE_TYPE}

# Source Files Quoted if spaced
SOURCEFILES_QUOTED_IF_SPACED=smain.c motor_control.c hx711.c tachometer.c input_signal.c ad7705.c

# Object Files Quoted if spaced
OBJECTFILES_QUOTED_IF_SPACED=${OBJECTDIR}/smain.o ${OBJECTDIR}/motor_control.o ${OBJECTDIR}/hx711.o ${OBJECTDIR}/tachometer.o ${OBJECTDIR}/input_signal.o ${OBJECTDIR}/ad7705.o
POSSIBLE_DEPFILES=${OBJECTDIR}/smain.o.d ${OBJECTDIR}/motor_control.o.d ${OBJECTDIR}/hx711.o.d ${OBJECTDIR}/tachometer.o.d ${OBJECTDIR}/input_signal.o.d ${OBJECTDIR}/ad7705.o.d

# Object Files
OBJECTFILES=${OBJECTDIR}/smain.o ${OBJECTDIR}/motor_control.o ${OBJECTDIR}/hx711.o ${OBJECTDIR}/tachometer.o ${OBJECTDIR}/input_signal.o ${OBJECTDIR}/ad7705.o

# Source Files
SOURCEFILES=smain.c motor_control.c hx711.c tachometer.c input_signal.c ad7705.c


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
ifneq ($(INFORMATION_MESSAGE), )
	@echo $(INFORMATION_MESSAGE)
endif
	${MAKE}  -f nbproject/Makefile-default.mk dist/${CND_CONF}/${IMAGE_TYPE}/MeasureStand.X.${IMAGE_TYPE}.${OUTPUT_SUFFIX}

MP_PROCESSOR_OPTION=33FJ256MC710
MP_LINKER_FILE_OPTION=,--script=p33FJ256MC710.gld
# ------------------------------------------------------------------------------------
# Rules for buildStep: compile
ifeq ($(TYPE_IMAGE), DEBUG_RUN)
${OBJECTDIR}/smain.o: smain.c  nbproject/Makefile-${CND_CONF}.mk
	@${MKDIR} "${OBJECTDIR}" 
	@${RM} ${OBJECTDIR}/smain.o.d 
	@${RM} ${OBJECTDIR}/smain.o 
	${MP_CC} $(MP_EXTRA_CC_PRE)  smain.c  -o ${OBJECTDIR}/smain.o  -c -mcpu=$(MP_PROCESSOR_OPTION)  -MMD -MF "${OBJECTDIR}/smain.o.d"      -g -D__DEBUG -D__MPLAB_DEBUGGER_PK3=1    -omf=elf   -O0 -msmart-io=1 -Wall -msfr-warn=off   -std=c99
	@${FIXDEPS} "${OBJECTDIR}/smain.o.d" $(SILENT)  -rsi ${MP_CC_DIR}../ 
	
${OBJECTDIR}/motor_control.o: motor_control.c  nbproject/Makefile-${CND_CONF}.mk
	@${MKDIR} "${OBJECTDIR}" 
	@${RM} ${OBJECTDIR}/motor_control.o.d 
	@${RM} ${OBJECTDIR}/motor_control.o 
	${MP_CC} $(MP_EXTRA_CC_PRE)  motor_control.c  -o ${OBJECTDIR}/motor_control.o  -c -mcpu=$(MP_PROCESSOR_OPTION)  -MMD -MF "${OBJECTDIR}/motor_control.o.d"      -g -D__DEBUG -D__MPLAB_DEBUGGER_PK3=1    -omf=elf   -O0 -msmart-io=1 -Wall -msfr-warn=off   -std=c99
	@${FIXDEPS} "${OBJECTDIR}/motor_control.o.d" $(SILENT)  -rsi ${MP_CC_DIR}../ 
	
${OBJECTDIR}/hx711.o: hx711.c  nbproject/Makefile-${CND_CONF}.mk
	@${MKDIR} "${OBJECTDIR}" 
	@${RM} ${OBJECTDIR}/hx711.o.d 
	@${RM} ${OBJECTDIR}/hx711.o 
	${MP_CC} $(MP_EXTRA_CC_PRE)  hx711.c  -o ${OBJECTDIR}/hx711.o  -c -mcpu=$(MP_PROCESSOR_OPTION)  -MMD -MF "${OBJECTDIR}/hx711.o.d"      -g -D__DEBUG -D__MPLAB_DEBUGGER_PK3=1    -omf=elf   -O0 -msmart-io=1 -Wall -msfr-warn=off   -std=c99
	@${FIXDEPS} "${OBJECTDIR}/hx711.o.d" $(SILENT)  -rsi ${MP_CC_DIR}../ 
	
${OBJECTDIR}/tachometer.o: tachometer.c  nbproject/Makefile-${CND_CONF}.mk
	@${MKDIR} "${OBJECTDIR}" 
	@${RM} ${OBJECTDIR}/tachometer.o.d 
	@${RM} ${OBJECTDIR}/tachometer.o 
	${MP_CC} $(MP_EXTRA_CC_PRE)  tachometer.c  -o ${OBJECTDIR}/tachometer.o  -c -mcpu=$(MP_PROCESSOR_OPTION)  -MMD -MF "${OBJECTDIR}/tachometer.o.d"      -g -D__DEBUG -D__MPLAB_DEBUGGER_PK3=1    -omf=elf   -O0 -msmart-io=1 -Wall -msfr-warn=off   -std=c99
	@${FIXDEPS} "${OBJECTDIR}/tachometer.o.d" $(SILENT)  -rsi ${MP_CC_DIR}../ 
	
${OBJECTDIR}/input_signal.o: input_signal.c  nbproject/Makefile-${CND_CONF}.mk
	@${MKDIR} "${OBJECTDIR}" 
	@${RM} ${OBJECTDIR}/input_signal.o.d 
	@${RM} ${OBJECTDIR}/input_signal.o 
	${MP_CC} $(MP_EXTRA_CC_PRE)  input_signal.c  -o ${OBJECTDIR}/input_signal.o  -c -mcpu=$(MP_PROCESSOR_OPTION)  -MMD -MF "${OBJECTDIR}/input_signal.o.d"      -g -D__DEBUG -D__MPLAB_DEBUGGER_PK3=1    -omf=elf   -O0 -msmart-io=1 -Wall -msfr-warn=off   -std=c99
	@${FIXDEPS} "${OBJECTDIR}/input_signal.o.d" $(SILENT)  -rsi ${MP_CC_DIR}../ 
	
${OBJECTDIR}/ad7705.o: ad7705.c  nbproject/Makefile-${CND_CONF}.mk
	@${MKDIR} "${OBJECTDIR}" 
	@${RM} ${OBJECTDIR}/ad7705.o.d 
	@${RM} ${OBJECTDIR}/ad7705.o 
	${MP_CC} $(MP_EXTRA_CC_PRE)  ad7705.c  -o ${OBJECTDIR}/ad7705.o  -c -mcpu=$(MP_PROCESSOR_OPTION)  -MMD -MF "${OBJECTDIR}/ad7705.o.d"      -g -D__DEBUG -D__MPLAB_DEBUGGER_PK3=1    -omf=elf   -O0 -msmart-io=1 -Wall -msfr-warn=off   -std=c99
	@${FIXDEPS} "${OBJECTDIR}/ad7705.o.d" $(SILENT)  -rsi ${MP_CC_DIR}../ 
	
else
${OBJECTDIR}/smain.o: smain.c  nbproject/Makefile-${CND_CONF}.mk
	@${MKDIR} "${OBJECTDIR}" 
	@${RM} ${OBJECTDIR}/smain.o.d 
	@${RM} ${OBJECTDIR}/smain.o 
	${MP_CC} $(MP_EXTRA_CC_PRE)  smain.c  -o ${OBJECTDIR}/smain.o  -c -mcpu=$(MP_PROCESSOR_OPTION)  -MMD -MF "${OBJECTDIR}/smain.o.d"        -g -omf=elf   -O0 -msmart-io=1 -Wall -msfr-warn=off   -std=c99
	@${FIXDEPS} "${OBJECTDIR}/smain.o.d" $(SILENT)  -rsi ${MP_CC_DIR}../ 
	
${OBJECTDIR}/motor_control.o: motor_control.c  nbproject/Makefile-${CND_CONF}.mk
	@${MKDIR} "${OBJECTDIR}" 
	@${RM} ${OBJECTDIR}/motor_control.o.d 
	@${RM} ${OBJECTDIR}/motor_control.o 
	${MP_CC} $(MP_EXTRA_CC_PRE)  motor_control.c  -o ${OBJECTDIR}/motor_control.o  -c -mcpu=$(MP_PROCESSOR_OPTION)  -MMD -MF "${OBJECTDIR}/motor_control.o.d"        -g -omf=elf   -O0 -msmart-io=1 -Wall -msfr-warn=off   -std=c99
	@${FIXDEPS} "${OBJECTDIR}/motor_control.o.d" $(SILENT)  -rsi ${MP_CC_DIR}../ 
	
${OBJECTDIR}/hx711.o: hx711.c  nbproject/Makefile-${CND_CONF}.mk
	@${MKDIR} "${OBJECTDIR}" 
	@${RM} ${OBJECTDIR}/hx711.o.d 
	@${RM} ${OBJECTDIR}/hx711.o 
	${MP_CC} $(MP_EXTRA_CC_PRE)  hx711.c  -o ${OBJECTDIR}/hx711.o  -c -mcpu=$(MP_PROCESSOR_OPTION)  -MMD -MF "${OBJECTDIR}/hx711.o.d"        -g -omf=elf   -O0 -msmart-io=1 -Wall -msfr-warn=off   -std=c99
	@${FIXDEPS} "${OBJECTDIR}/hx711.o.d" $(SILENT)  -rsi ${MP_CC_DIR}../ 
	
${OBJECTDIR}/tachometer.o: tachometer.c  nbproject/Makefile-${CND_CONF}.mk
	@${MKDIR} "${OBJECTDIR}" 
	@${RM} ${OBJECTDIR}/tachometer.o.d 
	@${RM} ${OBJECTDIR}/tachometer.o 
	${MP_CC} $(MP_EXTRA_CC_PRE)  tachometer.c  -o ${OBJECTDIR}/tachometer.o  -c -mcpu=$(MP_PROCESSOR_OPTION)  -MMD -MF "${OBJECTDIR}/tachometer.o.d"        -g -omf=elf   -O0 -msmart-io=1 -Wall -msfr-warn=off   -std=c99
	@${FIXDEPS} "${OBJECTDIR}/tachometer.o.d" $(SILENT)  -rsi ${MP_CC_DIR}../ 
	
${OBJECTDIR}/input_signal.o: input_signal.c  nbproject/Makefile-${CND_CONF}.mk
	@${MKDIR} "${OBJECTDIR}" 
	@${RM} ${OBJECTDIR}/input_signal.o.d 
	@${RM} ${OBJECTDIR}/input_signal.o 
	${MP_CC} $(MP_EXTRA_CC_PRE)  input_signal.c  -o ${OBJECTDIR}/input_signal.o  -c -mcpu=$(MP_PROCESSOR_OPTION)  -MMD -MF "${OBJECTDIR}/input_signal.o.d"        -g -omf=elf   -O0 -msmart-io=1 -Wall -msfr-warn=off   -std=c99
	@${FIXDEPS} "${OBJECTDIR}/input_signal.o.d" $(SILENT)  -rsi ${MP_CC_DIR}../ 
	
${OBJECTDIR}/ad7705.o: ad7705.c  nbproject/Makefile-${CND_CONF}.mk
	@${MKDIR} "${OBJECTDIR}" 
	@${RM} ${OBJECTDIR}/ad7705.o.d 
	@${RM} ${OBJECTDIR}/ad7705.o 
	${MP_CC} $(MP_EXTRA_CC_PRE)  ad7705.c  -o ${OBJECTDIR}/ad7705.o  -c -mcpu=$(MP_PROCESSOR_OPTION)  -MMD -MF "${OBJECTDIR}/ad7705.o.d"        -g -omf=elf   -O0 -msmart-io=1 -Wall -msfr-warn=off   -std=c99
	@${FIXDEPS} "${OBJECTDIR}/ad7705.o.d" $(SILENT)  -rsi ${MP_CC_DIR}../ 
	
endif

# ------------------------------------------------------------------------------------
# Rules for buildStep: assemble
ifeq ($(TYPE_IMAGE), DEBUG_RUN)
else
endif

# ------------------------------------------------------------------------------------
# Rules for buildStep: assemblePreproc
ifeq ($(TYPE_IMAGE), DEBUG_RUN)
else
endif

# ------------------------------------------------------------------------------------
# Rules for buildStep: link
ifeq ($(TYPE_IMAGE), DEBUG_RUN)
dist/${CND_CONF}/${IMAGE_TYPE}/MeasureStand.X.${IMAGE_TYPE}.${OUTPUT_SUFFIX}: ${OBJECTFILES}  nbproject/Makefile-${CND_CONF}.mk  ../Libraries/Periphery_lib.X/dist/default/debug/Periphery_lib.X.a  
	@${MKDIR} dist/${CND_CONF}/${IMAGE_TYPE} 
	${MP_CC} $(MP_EXTRA_LD_PRE)  -o dist/${CND_CONF}/${IMAGE_TYPE}/MeasureStand.X.${IMAGE_TYPE}.${OUTPUT_SUFFIX}  ${OBJECTFILES_QUOTED_IF_SPACED}    ../Libraries/Periphery_lib.X/dist/default/debug/Periphery_lib.X.a  -mcpu=$(MP_PROCESSOR_OPTION)        -D__DEBUG -D__MPLAB_DEBUGGER_PK3=1  -omf=elf    -mreserve=data@0x800:0x81F -mreserve=data@0x820:0x821 -mreserve=data@0x822:0x823 -mreserve=data@0x824:0x825 -mreserve=data@0x826:0x84F   -Wl,,--defsym=__MPLAB_BUILD=1,--defsym=__MPLAB_DEBUG=1,--defsym=__DEBUG=1,--defsym=__MPLAB_DEBUGGER_PK3=1,$(MP_LINKER_FILE_OPTION),--heap=4096,--stack=16,--check-sections,--data-init,--pack-data,--handles,--isr,--no-gc-sections,--fill-upper=0,--stackguard=16,--no-force-link,--smart-io,-Map="${DISTDIR}/${PROJECTNAME}.${IMAGE_TYPE}.map",--report-mem$(MP_EXTRA_LD_POST) 
	
else
dist/${CND_CONF}/${IMAGE_TYPE}/MeasureStand.X.${IMAGE_TYPE}.${OUTPUT_SUFFIX}: ${OBJECTFILES}  nbproject/Makefile-${CND_CONF}.mk  ../Libraries/Periphery_lib.X/dist/default/production/Periphery_lib.X.a 
	@${MKDIR} dist/${CND_CONF}/${IMAGE_TYPE} 
	${MP_CC} $(MP_EXTRA_LD_PRE)  -o dist/${CND_CONF}/${IMAGE_TYPE}/MeasureStand.X.${IMAGE_TYPE}.${DEBUGGABLE_SUFFIX}  ${OBJECTFILES_QUOTED_IF_SPACED}    ../Libraries/Periphery_lib.X/dist/default/production/Periphery_lib.X.a  -mcpu=$(MP_PROCESSOR_OPTION)        -omf=elf   -Wl,,--defsym=__MPLAB_BUILD=1,$(MP_LINKER_FILE_OPTION),--heap=4096,--stack=16,--check-sections,--data-init,--pack-data,--handles,--isr,--no-gc-sections,--fill-upper=0,--stackguard=16,--no-force-link,--smart-io,-Map="${DISTDIR}/${PROJECTNAME}.${IMAGE_TYPE}.map",--report-mem$(MP_EXTRA_LD_POST) 
	${MP_CC_DIR}/xc16-bin2hex dist/${CND_CONF}/${IMAGE_TYPE}/MeasureStand.X.${IMAGE_TYPE}.${DEBUGGABLE_SUFFIX} -a  -omf=elf  
	
endif


# Subprojects
.build-subprojects:
	cd ../Libraries/Periphery_lib.X && ${MAKE}  -f Makefile CONF=default


# Subprojects
.clean-subprojects:
	cd ../Libraries/Periphery_lib.X && rm -rf "build/default" "dist/default"

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
