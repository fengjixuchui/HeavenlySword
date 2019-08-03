#
# Set the environment variable HK_BUILD_SPEC to specify the configuration to build ("release" or "debug").
# Defaults to "release"
#
ifeq "$(origin HK_BUILD_SPEC)" "undefined"
  HK_BUILD_SPEC:=release
endif

ifeq "$(origin HK_BUILD_OS)" "undefined"
  ifeq "$(OS)" "Windows_NT"
    HK_BUILD_OS:=win32
  else
    HK_BUILD_OS:=linux
  endif
endif
# --- General ---

ifeq "$(origin CELL_SDK)" "undefined"
	CELL_SDK :=/usr/local/cell/0_5_0
endif

ifeq "$(origin HK_BUILD_OS)" "undefined"
HK_BUILD_OS:=linux
endif

ifeq "$(HK_BUILD_OS)" "linux"
	HOST_PATH := $(CELL_SDK)/host-linux
else
	HOST_PATH := $(CELL_SDK)/host-win32
endif

# --- SPU Specific ---

SPUGCC_DIR := $(HOST_PATH)/spu/bin
SPU_PREFIX := $(SPUGCC_DIR)/spu-lv2-

SPU_AR      := $(SPU_PREFIX)ar
SPU_AS      := $(SPU_PREFIX)gcc
SPU_CC      := $(SPU_PREFIX)gcc
SPU_CXX     := $(SPU_PREFIX)g++
SPU_CCLD    := $(SPU_PREFIX)gcc
SPU_NM      := $(SPU_PREFIX)nm

SPU_OBJCOPY := $(SPU_PREFIX)objcopy
SPU_OBJDUMP := $(SPU_PREFIX)objdump
SPU_RANLIB  := $(SPU_PREFIX)ranlib
SPU_SIZE    := $(SPU_PREFIX)size
SPU_STRINGS := $(SPU_PREFIX)strings
SPU_STRIP   := $(SPU_PREFIX)strip

SPU_DBG_FLAGS      := -g
SPU_OPTIMIZE_LV    := -O2
SPU_CSTDFLAGS      := 
SPU_CXXSTDFLAGS    := -std=c++98
SPU_CWARNFLAGS     := -Wall
SPU_INCDIRS        :=
SPU_CPPFLAGS       :=
SPU_MACHINE_OPTION :=

SPU_ASFLAGS  :=
SPU_MDFLAGS  := -MMD
SPU_LDFLAGS  := 
SPU_LOADLIBS :=
SPU_LDLIBS   := 
SPU_LDLIBDIR := 

CXX := $(SPU_CXX)
CXXFLAGS := $(SPU_CXXSTDFLAGS)
AR := $(SPU_AR)

LDFLAGS	+= $(SPU_LDLIBS)

ALL_SPU_CXXFLAGS := $(CXXFLAGS) $(SPU_MACHINE_OPTION) $(SPU_CWARNFLAGS)

ifeq "$(HK_BUILD_SPEC)" "fulldebug"
	FD_ALL_SPU_CXXFLAGS += -g -fno-inline -fno-exceptions -fno-rtti  -fno-strict-aliasing
	ALL_SPU_CXXFLAGS += -g -fno-inline -fno-exceptions -fno-rtti  -fno-strict-aliasing
	INCLUDE:= -I.. -I../../../sdk/include/physics -I../../../sdk/include/common -I../../../sdk/include/animation -I../../../sdk/include/complete -I../../common -I../../../common -I../..
	DEFINES += -DHK_DEBUG
	OBJDIR := ../../../obj/spu-gcc/havok_2+mayhem_client/$(HK_BUILD_SPEC)/$(LIB_NAME)
	OUTPUT_DIR := ../../../lib/spu-gcc/$(HK_BUILD_SPEC)
	TARGET := $(OUTPUT_DIR)/lib$(LIB_NAME).a
endif

ifeq "$(HK_BUILD_SPEC)" "debug"
	FD_ALL_SPU_CXXFLAGS += -g -fno-inline -fno-exceptions -fno-rtti  -fno-strict-aliasing
	ALL_SPU_CXXFLAGS += -g1 -O3 -fno-exceptions -fno-rtti  -fno-strict-aliasing -finline-limit=20000 -fno-implement-inlines -fno-inline-functions -Winline -funit-at-a-time --param large-function-insns=20000 --param inline-unit-growth=10000
	INCLUDE:= -I.. -I../../../sdk/include/physics -I../../../sdk/include/common -I../../../sdk/include/animation -I../../../sdk/include/complete -I../../common -I../../../common -I../..
	DEFINES += -DHK_DEBUG
	LDFLAGS += -g1
	OBJDIR := ../../../obj/spu-gcc/havok_2+mayhem_client/$(HK_BUILD_SPEC)/$(LIB_NAME)
	OUTPUT_DIR := ../../../lib/spu-gcc/$(HK_BUILD_SPEC)
	TARGET := $(OUTPUT_DIR)/lib$(LIB_NAME).a	
endif

ifeq "$(HK_BUILD_SPEC)" "release"
	FD_ALL_SPU_CXXFLAGS += -g -fno-inline -fno-exceptions -fno-rtti  -fno-strict-aliasing
	ALL_SPU_CXXFLAGS += -O3 -fno-exceptions -fno-rtti  -fno-strict-aliasing -finline-limit=20000 -fno-implement-inlines -fno-inline-functions -Winline -funit-at-a-time --param large-function-insns=20000 --param inline-unit-growth=10000
	INCLUDE:= -I.. -I../../../sdk/include/physics -I../../../sdk/include/common -I../../../sdk/include/animation -I../../../sdk/include/complete -I../../common -I../../../common -I../..
	OBJDIR := ../../../obj/spu-gcc/havok_2+mayhem_client/$(HK_BUILD_SPEC)/$(LIB_NAME)
	OUTPUT_DIR := ../../../lib/spu-gcc/$(HK_BUILD_SPEC)
	TARGET := $(OUTPUT_DIR)/lib$(LIB_NAME).a	
endif

COMPILE_COMMAND := $(CXX) $(CPPFLAGS) $(ALL_SPU_CXXFLAGS)
LINK_EXECUTABLE := $(SPU_CXX) $(LDFLAGS)
LINK_SPECIALS := 
LINK_LIBRARY := $(SPU_AR) -rcu


#################

STD_INCLUDES := 

COMPILED_LIBS += 
LINK_LIBS:=$(addprefix -l, $(LIBS) $(STD_LIBS)) $(COMPILED_LIBS)
STD_LIBS :=
LINK_EXE_PRE = 
LINK_EXE = $(SPU_CXX) $(LDFLAGS) 
LINK_EXE_POST = 
LINK_LIB = $(SPU_AR) -rcu

# configuration specifics...
WARNING	:=
DEFINES +=
COMPILE = $(CXX) $(CPPFLAGS) $(ALL_SPU_CXXFLAGS) $(INCLUDE) $(OPTIMISE) $(WARNING) $(DEFINES) $(STD_INCLUDES)
FD_COMPILE = $(CXX) $(CPPFLAGS) $(FD_ALL_SPU_CXXFLAGS) $(INCLUDE) $(OPTIMISE) $(WARNING) $(DEFINES) $(STD_INCLUDES)

OBJSUFFIX := .o
SRCX:= $(notdir $(SRC))
OBJ := $(addprefix $(OBJDIR)/, $(addsuffix $(OBJSUFFIX), $(basename $(SRCX))))

FD_OBJSUFFIX := .x
FD_SRCX:= $(notdir $(FD_SRC))
FD_OBJ := $(addprefix $(OBJDIR)/, $(addsuffix $(FD_OBJSUFFIX), $(basename $(FD_SRCX) )))

vpath %.cpp $(sort $(dir $(SRC)))
vpath %.cpp $(sort $(dir $(FD_SRC)))
vpath %.dsm $(sort $(dir $(SRC)))

all : echo_options dirs $(TARGET)
	@echo $(notdir $(TARGET))

silent : dirs $(TARGET)

echo_options:
#	@echo "$(TARGET) : ($(HK_BUILD_SPEC))"
#	@echo "compile: $(COMPILE)"
#	@echo "include: $(INCLUDE)"
#	@echo "linklib: $(LINK_LIB)"
#	@echo "linkexe: $(LINK_EXE)"

dirs:
ifeq "$(HK_BUILD_ENV)" "win32"
	-cmd /C "if not exist $(subst /,\,$(OUTPUT_DIR)) mkdir $(subst /,\,$(OUTPUT_DIR))"
	-cmd /C "if not exist $(subst /,\,$(OBJDIR)) mkdir $(subst /,\,$(OBJDIR))"
else
	@mkdir -p $(OUTPUT_DIR)
	@mkdir -p $(OBJDIR)
endif

$(TARGET) : $(OBJ) $(FD_OBJ) force_linking
	@$(LINK_LIB) $@ $(OBJ) $(FD_OBJ)
	
force_linking:	


# COMPILE is a 'function' taking arguments (source, object, extra flags)
$(OBJDIR)/%.o : %.cpp 
	@echo $(notdir $<)
	@$(COMPILE) $(INCLUDE) -c $< -o $@

$(OBJDIR)/%.x : %.cpp 
	@echo debug $(notdir $<)
	@$(FD_COMPILE) $(INCLUDE) -c $< -o $@

clean:
ifeq "$(HK_BUILD_ENV)" "win32"
	-cmd /C "del /S /F /Q $(subst /,\, $(OBJDIR)/*.o $(TARGET))"
	-cmd /C "del /S /F /Q $(subst /,\, $(OBJDIR)/*.x $(TARGET))"
else
	rm -f $(OBJDIR)/*.o $(TARGET)
	rm -f $(OBJDIR)/*.x $(TARGET)
endif

