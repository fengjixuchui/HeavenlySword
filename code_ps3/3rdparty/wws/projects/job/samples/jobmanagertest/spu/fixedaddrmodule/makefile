CODE_DIR		= ../../../../..
include $(CODE_DIR)/job/makeopts


TARGET			= fixedaddrmodule.spu.mod

SPU_SOURCES		+= jobmanagertest/spu/fixedaddrmodule/fixedaddrmodule.cpp


# Note: for this job only, I will force it to compile & run at a different adrs entirely				 
# This equate of 0x10000 (below, in bytes) *MUST* match the value
# of kCodeBufferSetBasePageNum (in pages) in CreateFixedAddrModuleJob() in jobmanagertest/main.cpp
SPU_UPLOAD_ADDRESS	= 0x10000


include $(CODE_DIR)/job/makerules
include $(CODE_DIR)/job/makelibs
