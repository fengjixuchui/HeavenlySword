CODE_DIR		= ../../../..
include $(CODE_DIR)/job/makeopts

TARGET			= libjobapi.spu.a

SPU_SOURCES		+= jobapi/commandlistchecker.cpp
SPU_SOURCES		+= jobapi/jobdependencydecrement.cpp
SPU_SOURCES		+= jobapi/jobevents.cpp
SPU_SOURCES		+= jobapi/jobprintf.spu.s
SPU_SOURCES		+= jobapi/jobspudma.cpp
SPU_SOURCES		+= jobapi/jobadder.cpp
SPU_SOURCES		+= jobapi/jobatomic.cpp


include $(CODE_DIR)/job/makerules
