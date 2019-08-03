CODE_DIR		= ../../..
include $(CODE_DIR)/job/makeopts


TARGET			= ppucallback.ppu.self

PPU_SOURCES		+= ppucallback/main.cpp
PPU_SOURCES		+= ppucallbackjob.spu.mod

PPU_LIBS		+= libjobapi.a

SUBDIRS			+= ppucallbackjob


include $(CODE_DIR)/job/makerules
include $(CODE_DIR)/job/makelibs
