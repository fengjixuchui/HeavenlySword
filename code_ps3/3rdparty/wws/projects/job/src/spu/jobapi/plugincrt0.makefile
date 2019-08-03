CODE_DIR		= ../../../..
include $(CODE_DIR)/job/makeopts

TARGET			= plugincrt0.spu.o

SPU_SOURCES		+= jobapi/plugincrt0.spu.s

include $(CODE_DIR)/job/makerules
