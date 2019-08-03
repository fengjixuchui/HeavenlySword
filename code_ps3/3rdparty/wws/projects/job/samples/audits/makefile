CODE_DIR		= ../../..
include $(CODE_DIR)/job/makeopts


TARGET			= audits.ppu.self

PPU_SOURCES		+= audits/main.cpp
PPU_SOURCES		+= auditsjob.spu.mod

PPU_LIBS		+= libjobapi.a

SUBDIRS			+= auditsjob


include $(CODE_DIR)/job/makerules
include $(CODE_DIR)/job/makelibs
