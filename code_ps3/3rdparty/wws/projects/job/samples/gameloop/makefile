CODE_DIR		= ../../..
include $(CODE_DIR)/job/makeopts


TARGET			= gameloop.ppu.self

PPU_SOURCES		+= gameloop/main.cpp
PPU_SOURCES		+= simplemodule.spu.mod

PPU_LIBS		+= libjobapi.a

SUBDIRS			+= simplemodule


include $(CODE_DIR)/job/makerules
include $(CODE_DIR)/job/makelibs
