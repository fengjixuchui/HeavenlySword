CODE_DIR		= ../../../..
include $(CODE_DIR)/job/makeopts


TARGET			= ppucallbackjob.spu.mod

SPU_SOURCES		+= ppucallback/ppucallbackjob/ppucallbackjob.cpp


#Linking with libsputhread.a is required for sending events
SPU_JOB_SYS_LIBS	+= -lsputhread


include $(CODE_DIR)/job/makerules
include $(CODE_DIR)/job/makelibs
