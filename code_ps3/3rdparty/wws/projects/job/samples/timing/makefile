CODE_DIR		= ../../..
include $(CODE_DIR)/job/makeopts


TARGET			= timing.ppu.self

PPU_SOURCES		+= timing/main.cpp
PPU_SOURCES		+= timingjob1.spu.mod
PPU_SOURCES		+= timingjob2.spu.mod
PPU_SOURCES		+= timingjob3.spu.mod
PPU_SOURCES		+= timingjobx.spu.mod

PPU_LIBS		+= libjobapi.a

SUBDIRS			+= job1
SUBDIRS			+= job2
SUBDIRS			+= jobx
SUBDIRS			+= job3


include $(CODE_DIR)/job/makerules
include $(CODE_DIR)/job/makelibs
