CODE_DIR		= ../../../..
include $(CODE_DIR)/job/makeopts

TARGET			= jobmanager.pmbin 

SPU_SOURCES += jobmanager/jmcrt0.spu.s
SPU_SOURCES += jobmanager/interrupthandlerasm.spu.s

ifeq ($(MODE), ASM)

SPU_SOURCES += jobmanager/data256aligned.spu.s
SPU_SOURCES += jobmanager/data128aligned.spu.s
SPU_SOURCES += jobmanager/data16.spu.s
SPU_SOURCES += jobmanager/asserts.spu
SPU_SOURCES += jobmanager/freedmatagid.spu
SPU_SOURCES += jobmanager/jobapi.spu
SPU_SOURCES += jobmanager/tryexecutecmds.spu
SPU_SOURCES += jobmanager/isdmatagmaskdone.spu
SPU_SOURCES += jobmanager/interrupthandler.spu
SPU_SOURCES += jobmanager/allocatejob.spu
SPU_SOURCES += jobmanager/decrementdependency.spu
SPU_SOURCES += jobmanager/auditwriter.spu
SPU_SOURCES += jobmanager/startbarrierednulllistwithinterrupt.spu
SPU_SOURCES += jobmanager/usedmatagid.spu
SPU_SOURCES += jobmanager/getpagemasks.spu
SPU_SOURCES += jobmanager/setmem.spu
SPU_SOURCES += jobmanager/trydumpsharebuf.spu
SPU_SOURCES += jobmanager/trydumpallstoresharebufs.spu
SPU_SOURCES += jobmanager/main.spu
SPU_SOURCES += jobmanager/returntospurskernel.spu
SPU_SOURCES += jobmanager/pollspurskernel.spu
SPU_SOURCES += jobmanager/tryexecuteloadcmds.spu
SPU_SOURCES += jobmanager/getlogicalbuffer.spu
SPU_SOURCES += jobmanager/tryfreetagandusedpages.spu
SPU_SOURCES += jobmanager/freelogicalbuffer.spu
SPU_SOURCES += jobmanager/getbuffertag.spu
SPU_SOURCES += jobmanager/trychangefreetoloadjob.spu
SPU_SOURCES += jobmanager/changeloadtorunjob.spu
SPU_SOURCES += jobmanager/changeruntostorejob.spu
SPU_SOURCES += jobmanager/trychangestoretofreejob.spu

# No bootstrap with SPU downcoded mode
SPU_UPLOAD_ADDRESS		= 0xA00

else

SPU_SOURCES		+= jobmanager/jobheadercache.cpp
SPU_SOURCES		+= jobmanager/interrupthandler.cpp
SPU_SOURCES		+= jobmanager/jobmanager.cpp
SPU_SOURCES		+= jobmanager/allocatejob.cpp
SPU_SOURCES		+= jobmanager/auditwriter.cpp
SPU_SOURCES		+= jobmanager/data.cpp

#JobManager is built above BootStrap
SPU_UPLOAD_ADDRESS		= 0x1000

endif


SPU_PM_LINKER_SCRIPT	= job_manager.ld

SPU_COMPILE_OPTIONS		+= -fno-zero-initialized-in-bss


include $(CODE_DIR)/job/makerules
include $(CODE_DIR)/job/makelibs