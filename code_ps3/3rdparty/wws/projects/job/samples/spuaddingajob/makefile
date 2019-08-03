CODE_DIR		= ../../..
include $(CODE_DIR)/job/makeopts


TARGET			= spuaddingajob.ppu.self

PPU_SOURCES		+= spuaddingajob/main.cpp
PPU_SOURCES		+= jobadderjob.spu.mod
PPU_SOURCES		+= secondaryjob.spu.mod

PPU_LIBS		+= libjobapi.a

SUBDIRS			+= jobadderjob
SUBDIRS			+= secondaryjob


include $(CODE_DIR)/job/makerules
include $(CODE_DIR)/job/makelibs
