CODE_DIR		= ../../..
include $(CODE_DIR)/job/makeopts


TARGET			= sharedbuffer.ppu.self

PPU_SOURCES		+= sharedbuffer/main.cpp
PPU_SOURCES		+= sharedbufferjob1.spu.mod
PPU_SOURCES		+= sharedbufferjob2.spu.mod
PPU_SOURCES		+= sharedbufferjob3.spu.mod
PPU_SOURCES		+= sharedbufferjobx.spu.mod

PPU_LIBS		+= libjobapi.a

SUBDIRS			+= job1
SUBDIRS			+= job2
SUBDIRS			+= jobx
SUBDIRS			+= job3


include $(CODE_DIR)/job/makerules
include $(CODE_DIR)/job/makelibs
