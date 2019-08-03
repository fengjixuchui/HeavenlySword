CODE_DIR		= ../../..
include $(CODE_DIR)/job/makeopts


TARGET			= jobmanagertest.ppu.self

PPU_SOURCES		+= jobmanagertest/main.cpp

#Embed all modules and plugins
PPU_SOURCES		+= createaccbuffermodule.spu.mod
PPU_SOURCES		+= fillaccbuffermodule.spu.mod
PPU_SOURCES		+= writeaccbuffermodule.spu.mod
PPU_SOURCES		+= nonaccbuffermodule.spu.mod
PPU_SOURCES		+= auditsmodule.spu.mod
PPU_SOURCES		+= dmalistmodule.spu.mod
PPU_SOURCES		+= fixedaddrmodule.spu.mod
PPU_SOURCES		+= module1.spu.mod
PPU_SOURCES		+= module2.spu.mod
PPU_SOURCES		+= plugintestmodule.spu.mod
PPU_SOURCES		+= plugin1.spu.plugin
PPU_SOURCES		+= plugin2.spu.plugin

PPU_LIBS		+= libjobapi.a

SUBDIRS			+= spu


include $(CODE_DIR)/job/makerules
include $(CODE_DIR)/job/makelibs
