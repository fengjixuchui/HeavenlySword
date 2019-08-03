CODE_DIR		= ../../..
include $(CODE_DIR)/job/makeopts


TARGET			= helloworld.ppu.self

PPU_SOURCES		+= helloworld/main.cpp
PPU_SOURCES		+= helloworldjob.spu.mod

PPU_LIBS		+= libjobapi.a

SUBDIRS			+= helloworldjob


include $(CODE_DIR)/job/makerules
include $(CODE_DIR)/job/makelibs
