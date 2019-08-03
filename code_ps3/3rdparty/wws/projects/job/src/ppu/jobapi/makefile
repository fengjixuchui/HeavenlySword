CODE_DIR		= ../../../..
include $(CODE_DIR)/job/makeopts


TARGET			= libjobapi.a

PPU_SOURCES		+= jobapi/auditmanager.cpp
PPU_SOURCES		+= jobapi/commandlistbuilder.cpp
PPU_SOURCES		+= jobapi/commandlistchecker.cpp
PPU_SOURCES		+= jobapi/embeddedjm.cpp
PPU_SOURCES		+= jobapi/eventhandler.cpp
PPU_SOURCES		+= jobapi/exceptionhandler.cpp
PPU_SOURCES		+= jobapi/jobdependency.cpp
PPU_SOURCES		+= jobapi/joblist.cpp
PPU_SOURCES		+= jobapi/jobprintf.cpp
PPU_SOURCES		+= jobapi/spumodule.cpp


PPU_SOURCES		+= jobmanager.pmbin
PPU_SOURCES		+= jobbootstrap.pmbin

CLEAN_DIRS		+= $(CODE_DIR)/job/src/spu/jobmanager
CLEAN_DIRS		+= $(CODE_DIR)/job/src/spu/jobbootstrap


#Hmm... Can I do this bit better?
#These rule should really go in makelibs, but I can't include that in here
# since there's a rule to chain off to make libjobapi.a in there.

.PHONY : %.phony
%.phony :
	$(MAKE) -C $(CODE_DIR)/job/src/spu/$*



#:NOTE: These is an empty rule - Without a command here, this would just be a dependency line
#so this has to have a command here
$(SPU_TEMP_DIR)/jobbin/jobmanager.pmbin		: jobmanager.phony
	true

$(SPU_TEMP_DIR)/jobbin/jobbootstrap.pmbin	: jobbootstrap.phony
	true

#For some strange reason it seems that linux won't pick up the upper rules, but will take this one
#Cygwin however seems unhappy with this one and relies on the ones above.  Hmmm...
$(SPU_TEMP_DIR)/jobbin/%.pmbin : %.phony
	true


include $(CODE_DIR)/job/makerules
