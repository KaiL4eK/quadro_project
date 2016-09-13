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
FINAL_IMAGE=dist/${CND_CONF}/${IMAGE_TYPE}/Quadro.X.${IMAGE_TYPE}.${OUTPUT_SUFFIX}
else
IMAGE_TYPE=production
OUTPUT_SUFFIX=hex
DEBUGGABLE_SUFFIX=elf
FINAL_IMAGE=dist/${CND_CONF}/${IMAGE_TYPE}/Quadro.X.${IMAGE_TYPE}.${OUTPUT_SUFFIX}
endif

ifeq ($(COMPARE_BUILD), true)
COMPARISON_BUILD=-mafrlcsj
else
COMPARISON_BUILD=
endif

# Object Directory
OBJECTDIR=build/${CND_CONF}/${IMAGE_TYPE}

# Distribution Directory
DISTDIR=dist/${CND_CONF}/${IMAGE_TYPE}

# Source Files Quoted if spaced
SOURCEFILES_QUOTED_IF_SPACED=qmain.c motor_control.c input_control.c bmp180.c hmc5883l.c FAT32.c SDcard.c file_io.c error.c command_processor.c

# Object Files Quoted if spaced
OBJECTFILES_QUOTED_IF_SPACED=${OBJECTDIR}/qmain.o ${OBJECTDIR}/motor_control.o ${OBJECTDIR}/input_control.o ${OBJECTDIR}/bmp180.o ${OBJECTDIR}/hmc5883l.o ${OBJECTDIR}/FAT32.o ${OBJECTDIR}/SDcard.o ${OBJECTDIR}/file_io.o ${OBJECTDIR}/error.o ${OBJECTDIR}/command_processor.o
POSSIBLE_DEPFILES=${OBJECTDIR}/qmain.o.d ${OBJECTDIR}/motor_control.o.d ${OBJECTDIR}/input_control.o.d ${OBJECTDIR}/bmp180.o.d ${OBJECTDIR}/hmc5883l.o.d ${OBJECTDIR}/FAT32.o.d ${OBJECTDIR}/SDcard.o.d ${OBJECTDIR}/file_io.o.d ${OBJECTDIR}/error.o.d ${OBJECTDIR}/command_processor.o.d

# Object Files
OBJECTFILES=${OBJECTDIR}/qmain.o ${OBJECTDIR}/motor_control.o ${OBJECTDIR}/input_control.o ${OBJECTDIR}/bmp180.o ${OBJECTDIR}/hmc5883l.o ${OBJECTDIR}/FAT32.o ${OBJECTDIR}/SDcard.o ${OBJECTDIR}/file_io.o ${OBJECTDIR}/error.o ${OBJECTDIR}/command_processor.o

# Source Files
SOURCEFILES=qmain.c motor_control.c input_control.c bmp180.c hmc5883l.c FAT32.c SDcard.c file_io.c error.c command_processor.c


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
	${MAKE}  -f nbproject/Makefile-default.mk dist/${CND_CONF}/${IMAGE_TYPE}/Quadro.X.${IMAGE_TYPE}.${OUTPUT_SUFFIX}

MP_PROCESSOR_OPTION=33FJ256MC710
MP_LINKER_FILE_OPTION=,--script=p33FJ256MC710.gld
# ------------------------------------------------------------------------------------
# Rules for buildStep: compile
ifeq ($(TYPE_IMAGE), DEBUG_RUN)
${OBJECTDIR}/qmain.o: qmain.c  nbproject/Makefile-${CND_CONF}.mk
	@${MKDIR} "${OBJECTDIR}" 
	@${RM} ${OBJECTDIR}/qmain.o.d 
	@${RM} ${OBJECTDIR}/qmain.o 
	${MP_CC} $(MP_EXTRA_CC_PRE)  qmain.c  -o ${OBJECTDIR}/qmain.o  -c -mcpu=$(MP_PROCESSOR_OPTION)  -MMD -MF "${OBJECTDIR}/qmain.o.d"      -g -D__DEBUG -D__MPLAB_DEBUGGER_PK3=1    -omf=elf -fast-math -no-legacy-libc  $(COMPARISON_BUILD)  -O1 -I"../Libraries/Math_lib.X" -I"../Libraries/Periphery_lib.X" -I"../common" -I"../Libraries/SensorsLib.X" -msmart-io=1 -Wall -msfr-warn=off  
	@${FIXDEPS} "${OBJECTDIR}/qmain.o.d" $(SILENT)  -rsi ${MP_CC_DIR}../ 
	
${OBJECTDIR}/motor_control.o: motor_control.c  nbproject/Makefile-${CND_CONF}.mk
	@${MKDIR} "${OBJECTDIR}" 
	@${RM} ${OBJECTDIR}/motor_control.o.d 
	@${RM} ${OBJECTDIR}/motor_control.o 
	${MP_CC} $(MP_EXTRA_CC_PRE)  motor_control.c  -o ${OBJECTDIR}/motor_control.o  -c -mcpu=$(MP_PROCESSOR_OPTION)  -MMD -MF "${OBJECTDIR}/motor_control.o.d"      -g -D__DEBUG -D__MPLAB_DEBUGGER_PK3=1    -omf=elf -fast-math -no-legacy-libc  $(COMPARISON_BUILD)  -O1 -I"../Libraries/Math_lib.X" -I"../Libraries/Periphery_lib.X" -I"../common" -I"../Libraries/SensorsLib.X" -msmart-io=1 -Wall -msfr-warn=off  
	@${FIXDEPS} "${OBJECTDIR}/motor_control.o.d" $(SILENT)  -rsi ${MP_CC_DIR}../ 
	
${OBJECTDIR}/input_control.o: input_control.c  nbproject/Makefile-${CND_CONF}.mk
	@${MKDIR} "${OBJECTDIR}" 
	@${RM} ${OBJECTDIR}/input_control.o.d 
	@${RM} ${OBJECTDIR}/input_control.o 
	${MP_CC} $(MP_EXTRA_CC_PRE)  input_control.c  -o ${OBJECTDIR}/input_control.o  -c -mcpu=$(MP_PROCESSOR_OPTION)  -MMD -MF "${OBJECTDIR}/input_control.o.d"      -g -D__DEBUG -D__MPLAB_DEBUGGER_PK3=1    -omf=elf -fast-math -no-legacy-libc  $(COMPARISON_BUILD)  -O1 -I"../Libraries/Math_lib.X" -I"../Libraries/Periphery_lib.X" -I"../common" -I"../Libraries/SensorsLib.X" -msmart-io=1 -Wall -msfr-warn=off  
	@${FIXDEPS} "${OBJECTDIR}/input_control.o.d" $(SILENT)  -rsi ${MP_CC_DIR}../ 
	
${OBJECTDIR}/bmp180.o: bmp180.c  nbproject/Makefile-${CND_CONF}.mk
	@${MKDIR} "${OBJECTDIR}" 
	@${RM} ${OBJECTDIR}/bmp180.o.d 
	@${RM} ${OBJECTDIR}/bmp180.o 
	${MP_CC} $(MP_EXTRA_CC_PRE)  bmp180.c  -o ${OBJECTDIR}/bmp180.o  -c -mcpu=$(MP_PROCESSOR_OPTION)  -MMD -MF "${OBJECTDIR}/bmp180.o.d"      -g -D__DEBUG -D__MPLAB_DEBUGGER_PK3=1    -omf=elf -fast-math -no-legacy-libc  $(COMPARISON_BUILD)  -O1 -I"../Libraries/Math_lib.X" -I"../Libraries/Periphery_lib.X" -I"../common" -I"../Libraries/SensorsLib.X" -msmart-io=1 -Wall -msfr-warn=off  
	@${FIXDEPS} "${OBJECTDIR}/bmp180.o.d" $(SILENT)  -rsi ${MP_CC_DIR}../ 
	
${OBJECTDIR}/hmc5883l.o: hmc5883l.c  nbproject/Makefile-${CND_CONF}.mk
	@${MKDIR} "${OBJECTDIR}" 
	@${RM} ${OBJECTDIR}/hmc5883l.o.d 
	@${RM} ${OBJECTDIR}/hmc5883l.o 
	${MP_CC} $(MP_EXTRA_CC_PRE)  hmc5883l.c  -o ${OBJECTDIR}/hmc5883l.o  -c -mcpu=$(MP_PROCESSOR_OPTION)  -MMD -MF "${OBJECTDIR}/hmc5883l.o.d"      -g -D__DEBUG -D__MPLAB_DEBUGGER_PK3=1    -omf=elf -fast-math -no-legacy-libc  $(COMPARISON_BUILD)  -O1 -I"../Libraries/Math_lib.X" -I"../Libraries/Periphery_lib.X" -I"../common" -I"../Libraries/SensorsLib.X" -msmart-io=1 -Wall -msfr-warn=off  
	@${FIXDEPS} "${OBJECTDIR}/hmc5883l.o.d" $(SILENT)  -rsi ${MP_CC_DIR}../ 
	
${OBJECTDIR}/FAT32.o: FAT32.c  nbproject/Makefile-${CND_CONF}.mk
	@${MKDIR} "${OBJECTDIR}" 
	@${RM} ${OBJECTDIR}/FAT32.o.d 
	@${RM} ${OBJECTDIR}/FAT32.o 
	${MP_CC} $(MP_EXTRA_CC_PRE)  FAT32.c  -o ${OBJECTDIR}/FAT32.o  -c -mcpu=$(MP_PROCESSOR_OPTION)  -MMD -MF "${OBJECTDIR}/FAT32.o.d"      -g -D__DEBUG -D__MPLAB_DEBUGGER_PK3=1    -omf=elf -fast-math -no-legacy-libc  $(COMPARISON_BUILD)  -O1 -I"../Libraries/Math_lib.X" -I"../Libraries/Periphery_lib.X" -I"../common" -I"../Libraries/SensorsLib.X" -msmart-io=1 -Wall -msfr-warn=off  
	@${FIXDEPS} "${OBJECTDIR}/FAT32.o.d" $(SILENT)  -rsi ${MP_CC_DIR}../ 
	
${OBJECTDIR}/SDcard.o: SDcard.c  nbproject/Makefile-${CND_CONF}.mk
	@${MKDIR} "${OBJECTDIR}" 
	@${RM} ${OBJECTDIR}/SDcard.o.d 
	@${RM} ${OBJECTDIR}/SDcard.o 
	${MP_CC} $(MP_EXTRA_CC_PRE)  SDcard.c  -o ${OBJECTDIR}/SDcard.o  -c -mcpu=$(MP_PROCESSOR_OPTION)  -MMD -MF "${OBJECTDIR}/SDcard.o.d"      -g -D__DEBUG -D__MPLAB_DEBUGGER_PK3=1    -omf=elf -fast-math -no-legacy-libc  $(COMPARISON_BUILD)  -O1 -I"../Libraries/Math_lib.X" -I"../Libraries/Periphery_lib.X" -I"../common" -I"../Libraries/SensorsLib.X" -msmart-io=1 -Wall -msfr-warn=off  
	@${FIXDEPS} "${OBJECTDIR}/SDcard.o.d" $(SILENT)  -rsi ${MP_CC_DIR}../ 
	
${OBJECTDIR}/file_io.o: file_io.c  nbproject/Makefile-${CND_CONF}.mk
	@${MKDIR} "${OBJECTDIR}" 
	@${RM} ${OBJECTDIR}/file_io.o.d 
	@${RM} ${OBJECTDIR}/file_io.o 
	${MP_CC} $(MP_EXTRA_CC_PRE)  file_io.c  -o ${OBJECTDIR}/file_io.o  -c -mcpu=$(MP_PROCESSOR_OPTION)  -MMD -MF "${OBJECTDIR}/file_io.o.d"      -g -D__DEBUG -D__MPLAB_DEBUGGER_PK3=1    -omf=elf -fast-math -no-legacy-libc  $(COMPARISON_BUILD)  -O1 -I"../Libraries/Math_lib.X" -I"../Libraries/Periphery_lib.X" -I"../common" -I"../Libraries/SensorsLib.X" -msmart-io=1 -Wall -msfr-warn=off  
	@${FIXDEPS} "${OBJECTDIR}/file_io.o.d" $(SILENT)  -rsi ${MP_CC_DIR}../ 
	
${OBJECTDIR}/error.o: error.c  nbproject/Makefile-${CND_CONF}.mk
	@${MKDIR} "${OBJECTDIR}" 
	@${RM} ${OBJECTDIR}/error.o.d 
	@${RM} ${OBJECTDIR}/error.o 
	${MP_CC} $(MP_EXTRA_CC_PRE)  error.c  -o ${OBJECTDIR}/error.o  -c -mcpu=$(MP_PROCESSOR_OPTION)  -MMD -MF "${OBJECTDIR}/error.o.d"      -g -D__DEBUG -D__MPLAB_DEBUGGER_PK3=1    -omf=elf -fast-math -no-legacy-libc  $(COMPARISON_BUILD)  -O1 -I"../Libraries/Math_lib.X" -I"../Libraries/Periphery_lib.X" -I"../common" -I"../Libraries/SensorsLib.X" -msmart-io=1 -Wall -msfr-warn=off  
	@${FIXDEPS} "${OBJECTDIR}/error.o.d" $(SILENT)  -rsi ${MP_CC_DIR}../ 
	
${OBJECTDIR}/command_processor.o: command_processor.c  nbproject/Makefile-${CND_CONF}.mk
	@${MKDIR} "${OBJECTDIR}" 
	@${RM} ${OBJECTDIR}/command_processor.o.d 
	@${RM} ${OBJECTDIR}/command_processor.o 
	${MP_CC} $(MP_EXTRA_CC_PRE)  command_processor.c  -o ${OBJECTDIR}/command_processor.o  -c -mcpu=$(MP_PROCESSOR_OPTION)  -MMD -MF "${OBJECTDIR}/command_processor.o.d"      -g -D__DEBUG -D__MPLAB_DEBUGGER_PK3=1    -omf=elf -fast-math -no-legacy-libc  $(COMPARISON_BUILD)  -O1 -I"../Libraries/Math_lib.X" -I"../Libraries/Periphery_lib.X" -I"../common" -I"../Libraries/SensorsLib.X" -msmart-io=1 -Wall -msfr-warn=off  
	@${FIXDEPS} "${OBJECTDIR}/command_processor.o.d" $(SILENT)  -rsi ${MP_CC_DIR}../ 
	
else
${OBJECTDIR}/qmain.o: qmain.c  nbproject/Makefile-${CND_CONF}.mk
	@${MKDIR} "${OBJECTDIR}" 
	@${RM} ${OBJECTDIR}/qmain.o.d 
	@${RM} ${OBJECTDIR}/qmain.o 
	${MP_CC} $(MP_EXTRA_CC_PRE)  qmain.c  -o ${OBJECTDIR}/qmain.o  -c -mcpu=$(MP_PROCESSOR_OPTION)  -MMD -MF "${OBJECTDIR}/qmain.o.d"        -g -omf=elf -fast-math -no-legacy-libc  $(COMPARISON_BUILD)  -O1 -I"../Libraries/Math_lib.X" -I"../Libraries/Periphery_lib.X" -I"../common" -I"../Libraries/SensorsLib.X" -msmart-io=1 -Wall -msfr-warn=off  
	@${FIXDEPS} "${OBJECTDIR}/qmain.o.d" $(SILENT)  -rsi ${MP_CC_DIR}../ 
	
${OBJECTDIR}/motor_control.o: motor_control.c  nbproject/Makefile-${CND_CONF}.mk
	@${MKDIR} "${OBJECTDIR}" 
	@${RM} ${OBJECTDIR}/motor_control.o.d 
	@${RM} ${OBJECTDIR}/motor_control.o 
	${MP_CC} $(MP_EXTRA_CC_PRE)  motor_control.c  -o ${OBJECTDIR}/motor_control.o  -c -mcpu=$(MP_PROCESSOR_OPTION)  -MMD -MF "${OBJECTDIR}/motor_control.o.d"        -g -omf=elf -fast-math -no-legacy-libc  $(COMPARISON_BUILD)  -O1 -I"../Libraries/Math_lib.X" -I"../Libraries/Periphery_lib.X" -I"../common" -I"../Libraries/SensorsLib.X" -msmart-io=1 -Wall -msfr-warn=off  
	@${FIXDEPS} "${OBJECTDIR}/motor_control.o.d" $(SILENT)  -rsi ${MP_CC_DIR}../ 
	
${OBJECTDIR}/input_control.o: input_control.c  nbproject/Makefile-${CND_CONF}.mk
	@${MKDIR} "${OBJECTDIR}" 
	@${RM} ${OBJECTDIR}/input_control.o.d 
	@${RM} ${OBJECTDIR}/input_control.o 
	${MP_CC} $(MP_EXTRA_CC_PRE)  input_control.c  -o ${OBJECTDIR}/input_control.o  -c -mcpu=$(MP_PROCESSOR_OPTION)  -MMD -MF "${OBJECTDIR}/input_control.o.d"        -g -omf=elf -fast-math -no-legacy-libc  $(COMPARISON_BUILD)  -O1 -I"../Libraries/Math_lib.X" -I"../Libraries/Periphery_lib.X" -I"../common" -I"../Libraries/SensorsLib.X" -msmart-io=1 -Wall -msfr-warn=off  
	@${FIXDEPS} "${OBJECTDIR}/input_control.o.d" $(SILENT)  -rsi ${MP_CC_DIR}../ 
	
${OBJECTDIR}/bmp180.o: bmp180.c  nbproject/Makefile-${CND_CONF}.mk
	@${MKDIR} "${OBJECTDIR}" 
	@${RM} ${OBJECTDIR}/bmp180.o.d 
	@${RM} ${OBJECTDIR}/bmp180.o 
	${MP_CC} $(MP_EXTRA_CC_PRE)  bmp180.c  -o ${OBJECTDIR}/bmp180.o  -c -mcpu=$(MP_PROCESSOR_OPTION)  -MMD -MF "${OBJECTDIR}/bmp180.o.d"        -g -omf=elf -fast-math -no-legacy-libc  $(COMPARISON_BUILD)  -O1 -I"../Libraries/Math_lib.X" -I"../Libraries/Periphery_lib.X" -I"../common" -I"../Libraries/SensorsLib.X" -msmart-io=1 -Wall -msfr-warn=off  
	@${FIXDEPS} "${OBJECTDIR}/bmp180.o.d" $(SILENT)  -rsi ${MP_CC_DIR}../ 
	
${OBJECTDIR}/hmc5883l.o: hmc5883l.c  nbproject/Makefile-${CND_CONF}.mk
	@${MKDIR} "${OBJECTDIR}" 
	@${RM} ${OBJECTDIR}/hmc5883l.o.d 
	@${RM} ${OBJECTDIR}/hmc5883l.o 
	${MP_CC} $(MP_EXTRA_CC_PRE)  hmc5883l.c  -o ${OBJECTDIR}/hmc5883l.o  -c -mcpu=$(MP_PROCESSOR_OPTION)  -MMD -MF "${OBJECTDIR}/hmc5883l.o.d"        -g -omf=elf -fast-math -no-legacy-libc  $(COMPARISON_BUILD)  -O1 -I"../Libraries/Math_lib.X" -I"../Libraries/Periphery_lib.X" -I"../common" -I"../Libraries/SensorsLib.X" -msmart-io=1 -Wall -msfr-warn=off  
	@${FIXDEPS} "${OBJECTDIR}/hmc5883l.o.d" $(SILENT)  -rsi ${MP_CC_DIR}../ 
	
${OBJECTDIR}/FAT32.o: FAT32.c  nbproject/Makefile-${CND_CONF}.mk
	@${MKDIR} "${OBJECTDIR}" 
	@${RM} ${OBJECTDIR}/FAT32.o.d 
	@${RM} ${OBJECTDIR}/FAT32.o 
	${MP_CC} $(MP_EXTRA_CC_PRE)  FAT32.c  -o ${OBJECTDIR}/FAT32.o  -c -mcpu=$(MP_PROCESSOR_OPTION)  -MMD -MF "${OBJECTDIR}/FAT32.o.d"        -g -omf=elf -fast-math -no-legacy-libc  $(COMPARISON_BUILD)  -O1 -I"../Libraries/Math_lib.X" -I"../Libraries/Periphery_lib.X" -I"../common" -I"../Libraries/SensorsLib.X" -msmart-io=1 -Wall -msfr-warn=off  
	@${FIXDEPS} "${OBJECTDIR}/FAT32.o.d" $(SILENT)  -rsi ${MP_CC_DIR}../ 
	
${OBJECTDIR}/SDcard.o: SDcard.c  nbproject/Makefile-${CND_CONF}.mk
	@${MKDIR} "${OBJECTDIR}" 
	@${RM} ${OBJECTDIR}/SDcard.o.d 
	@${RM} ${OBJECTDIR}/SDcard.o 
	${MP_CC} $(MP_EXTRA_CC_PRE)  SDcard.c  -o ${OBJECTDIR}/SDcard.o  -c -mcpu=$(MP_PROCESSOR_OPTION)  -MMD -MF "${OBJECTDIR}/SDcard.o.d"        -g -omf=elf -fast-math -no-legacy-libc  $(COMPARISON_BUILD)  -O1 -I"../Libraries/Math_lib.X" -I"../Libraries/Periphery_lib.X" -I"../common" -I"../Libraries/SensorsLib.X" -msmart-io=1 -Wall -msfr-warn=off  
	@${FIXDEPS} "${OBJECTDIR}/SDcard.o.d" $(SILENT)  -rsi ${MP_CC_DIR}../ 
	
${OBJECTDIR}/file_io.o: file_io.c  nbproject/Makefile-${CND_CONF}.mk
	@${MKDIR} "${OBJECTDIR}" 
	@${RM} ${OBJECTDIR}/file_io.o.d 
	@${RM} ${OBJECTDIR}/file_io.o 
	${MP_CC} $(MP_EXTRA_CC_PRE)  file_io.c  -o ${OBJECTDIR}/file_io.o  -c -mcpu=$(MP_PROCESSOR_OPTION)  -MMD -MF "${OBJECTDIR}/file_io.o.d"        -g -omf=elf -fast-math -no-legacy-libc  $(COMPARISON_BUILD)  -O1 -I"../Libraries/Math_lib.X" -I"../Libraries/Periphery_lib.X" -I"../common" -I"../Libraries/SensorsLib.X" -msmart-io=1 -Wall -msfr-warn=off  
	@${FIXDEPS} "${OBJECTDIR}/file_io.o.d" $(SILENT)  -rsi ${MP_CC_DIR}../ 
	
${OBJECTDIR}/error.o: error.c  nbproject/Makefile-${CND_CONF}.mk
	@${MKDIR} "${OBJECTDIR}" 
	@${RM} ${OBJECTDIR}/error.o.d 
	@${RM} ${OBJECTDIR}/error.o 
	${MP_CC} $(MP_EXTRA_CC_PRE)  error.c  -o ${OBJECTDIR}/error.o  -c -mcpu=$(MP_PROCESSOR_OPTION)  -MMD -MF "${OBJECTDIR}/error.o.d"        -g -omf=elf -fast-math -no-legacy-libc  $(COMPARISON_BUILD)  -O1 -I"../Libraries/Math_lib.X" -I"../Libraries/Periphery_lib.X" -I"../common" -I"../Libraries/SensorsLib.X" -msmart-io=1 -Wall -msfr-warn=off  
	@${FIXDEPS} "${OBJECTDIR}/error.o.d" $(SILENT)  -rsi ${MP_CC_DIR}../ 
	
${OBJECTDIR}/command_processor.o: command_processor.c  nbproject/Makefile-${CND_CONF}.mk
	@${MKDIR} "${OBJECTDIR}" 
	@${RM} ${OBJECTDIR}/command_processor.o.d 
	@${RM} ${OBJECTDIR}/command_processor.o 
	${MP_CC} $(MP_EXTRA_CC_PRE)  command_processor.c  -o ${OBJECTDIR}/command_processor.o  -c -mcpu=$(MP_PROCESSOR_OPTION)  -MMD -MF "${OBJECTDIR}/command_processor.o.d"        -g -omf=elf -fast-math -no-legacy-libc  $(COMPARISON_BUILD)  -O1 -I"../Libraries/Math_lib.X" -I"../Libraries/Periphery_lib.X" -I"../common" -I"../Libraries/SensorsLib.X" -msmart-io=1 -Wall -msfr-warn=off  
	@${FIXDEPS} "${OBJECTDIR}/command_processor.o.d" $(SILENT)  -rsi ${MP_CC_DIR}../ 
	
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
dist/${CND_CONF}/${IMAGE_TYPE}/Quadro.X.${IMAGE_TYPE}.${OUTPUT_SUFFIX}: ${OBJECTFILES}  nbproject/Makefile-${CND_CONF}.mk  ../Libraries/Periphery_lib.X/dist/default/debug/Periphery_lib.X.a ../Libraries/Math_lib.X/dist/default/debug/Math_lib.X.a ../Libraries/SensorsLib.X/dist/default/debug/SensorsLib.X.a  
	@${MKDIR} dist/${CND_CONF}/${IMAGE_TYPE} 
	${MP_CC} $(MP_EXTRA_LD_PRE)  -o dist/${CND_CONF}/${IMAGE_TYPE}/Quadro.X.${IMAGE_TYPE}.${OUTPUT_SUFFIX}  ${OBJECTFILES_QUOTED_IF_SPACED}    ../Libraries/Periphery_lib.X/dist/default/debug/Periphery_lib.X.a ../Libraries/Math_lib.X/dist/default/debug/Math_lib.X.a ../Libraries/SensorsLib.X/dist/default/debug/SensorsLib.X.a  -mcpu=$(MP_PROCESSOR_OPTION)        -D__DEBUG -D__MPLAB_DEBUGGER_PK3=1  -omf=elf -fast-math -no-legacy-libc  $(COMPARISON_BUILD)   -mreserve=data@0x800:0x81F -mreserve=data@0x820:0x821 -mreserve=data@0x822:0x823 -mreserve=data@0x824:0x825 -mreserve=data@0x826:0x84F   -Wl,,--defsym=__MPLAB_BUILD=1,--defsym=__MPLAB_DEBUG=1,--defsym=__DEBUG=1,--defsym=__MPLAB_DEBUGGER_PK3=1,$(MP_LINKER_FILE_OPTION),--heap=4096,--stack=16,--check-sections,--data-init,--pack-data,--handles,--isr,--no-gc-sections,--fill-upper=0,--stackguard=16,--no-force-link,--smart-io,-Map="${DISTDIR}/${PROJECTNAME}.${IMAGE_TYPE}.map",--report-mem,--memorysummary,dist/${CND_CONF}/${IMAGE_TYPE}/memoryfile.xml$(MP_EXTRA_LD_POST) 
	
else
dist/${CND_CONF}/${IMAGE_TYPE}/Quadro.X.${IMAGE_TYPE}.${OUTPUT_SUFFIX}: ${OBJECTFILES}  nbproject/Makefile-${CND_CONF}.mk  ../Libraries/Periphery_lib.X/dist/default/production/Periphery_lib.X.a ../Libraries/Math_lib.X/dist/default/production/Math_lib.X.a ../Libraries/SensorsLib.X/dist/default/production/SensorsLib.X.a 
	@${MKDIR} dist/${CND_CONF}/${IMAGE_TYPE} 
	${MP_CC} $(MP_EXTRA_LD_PRE)  -o dist/${CND_CONF}/${IMAGE_TYPE}/Quadro.X.${IMAGE_TYPE}.${DEBUGGABLE_SUFFIX}  ${OBJECTFILES_QUOTED_IF_SPACED}    ../Libraries/Periphery_lib.X/dist/default/production/Periphery_lib.X.a ../Libraries/Math_lib.X/dist/default/production/Math_lib.X.a ../Libraries/SensorsLib.X/dist/default/production/SensorsLib.X.a  -mcpu=$(MP_PROCESSOR_OPTION)        -omf=elf -fast-math -no-legacy-libc  $(COMPARISON_BUILD)  -Wl,,--defsym=__MPLAB_BUILD=1,$(MP_LINKER_FILE_OPTION),--heap=4096,--stack=16,--check-sections,--data-init,--pack-data,--handles,--isr,--no-gc-sections,--fill-upper=0,--stackguard=16,--no-force-link,--smart-io,-Map="${DISTDIR}/${PROJECTNAME}.${IMAGE_TYPE}.map",--report-mem,--memorysummary,dist/${CND_CONF}/${IMAGE_TYPE}/memoryfile.xml$(MP_EXTRA_LD_POST) 
	${MP_CC_DIR}/xc16-bin2hex dist/${CND_CONF}/${IMAGE_TYPE}/Quadro.X.${IMAGE_TYPE}.${DEBUGGABLE_SUFFIX} -a  -omf=elf  
	
endif


# Subprojects
.build-subprojects:
	cd ../Libraries/Periphery_lib.X && ${MAKE}  -f Makefile CONF=default
	cd ../Libraries/Math_lib.X && ${MAKE}  -f Makefile CONF=default
	cd ../Libraries/SensorsLib.X && ${MAKE}  -f Makefile CONF=default


# Subprojects
.clean-subprojects:
	cd ../Libraries/Periphery_lib.X && rm -rf "build/default" "dist/default"
	cd ../Libraries/Math_lib.X && rm -rf "build/default" "dist/default"
	cd ../Libraries/SensorsLib.X && rm -rf "build/default" "dist/default"

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
