CODE_DIR		= ../../../..
include $(CODE_DIR)/job/makeopts

TARGET			= jobbootstrap.pmbin 

SPU_SOURCES		+= jobbootstrap/bscrt0.spu.s
SPU_SOURCES		+= jobbootstrap/bootstrapmain.cpp


#BootStrap code goes directly above spurs kernel
SPU_UPLOAD_ADDRESS	= 0xa00

SPU_PM_LINKER_SCRIPT	= boot_strap.ld


#Always optimise the bootstrapper regardless of debug/release etc..
OPTIMISATION_OPTIONS	+= -Os
OPTIMISATION_OPTIONS	+= --param inline-unit-growth=200


include $(CODE_DIR)/job/makerules
include $(CODE_DIR)/job/makelibs