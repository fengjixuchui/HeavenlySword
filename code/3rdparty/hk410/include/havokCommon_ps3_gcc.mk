#
# Set the environment variable HK_BUILD_SPEC to specify the configuration to build ("release" or "debug").
# Defaults to "release"
#

ifeq "$(origin HK_BUILD_SPEC)" "undefined"
HK_BUILD_SPEC:=release
endif

# --- General ---

ifeq "$(origin CELL_SDK)" "undefined"
	CELL_SDK :=/usr/local/cell/0_5_0
endif

ifeq "$(origin HK_BUILD_OS)" "undefined"
  ifeq "$(OS)" "Windows_NT"
    HK_BUILD_OS:=win32
  else
    HK_BUILD_OS:=linux
  endif
endif

ifeq "$(HK_BUILD_OS)" "linux"
	HOST_PATH := $(CELL_SDK)/host-linux
else
	HOST_PATH := $(CELL_SDK)/host-win32
endif

# --- PPU Specific ---

PUGCC_DIR := $(HOST_PATH)/ppu/bin
PU_PREFIX := $(PUGCC_DIR)/ppu-lv2-

PU_AR   := $(PU_PREFIX)ar
PU_AS   := $(PU_PREFIX)as
PU_CCAS := $(PU_PREFIX)gcc
PU_CC   := $(PU_PREFIX)gcc
PU_CXX  := $(PU_PREFIX)g++
PU_LD   := $(PU_PREFIX)ld
PU_CCLD := $(PU_PREFIX)gcc
PU_NM   := $(PU_PREFIX)nm

PU_OBJCOPY := $(PU_PREFIX)objcopy
PU_OBJDUMP := $(PU_PREFIX)objdump
PU_RANLIB  := $(PU_PREFIX)ranlib
PU_SIZE    := $(PU_PREFIX)size
PU_STRINGS := $(PU_PREFIX)strings
PU_STRIP   := $(PU_PREFIX)strip

PU_DBG_FLAGS      := -g
PU_OPTIMIZE_LV    := -O2
PU_CSTDFLAGS      := 
PU_CXXSTDFLAGS    := -std=c++98 -mminimal-toc
PU_CWARNFLAGS     := -Wall -Wno-invalid-offsetof
PU_INCDIRS        :=
PU_CPPFLAGS       :=
PU_MACHINE_OPTION := -maltivec

PU_ASFLAGS  :=
PU_MDFLAGS  := -MMD
PU_LDFLAGS  :=
PU_LOADLIBS :=
PU_LDLIBS   :=
PU_LDLIBDIR :=


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
SPU_LDLIBS   := -llv2spu
SPU_LDLIBDIR := 


# --- Cg (JS) Specific ---

JS_TOOLS_DIR      := $(HOST_PATH)/Cg/bin
JS_CGC            := $(JS_TOOLS_DIR)/cgc
JS_SHADER_BUILDER := $(JS_TOOLS_DIR)/js_shader_builder

# ----------------------------------------------------------------------------------------


CXX := $(PU_CXX)
CXXFLAGS := $(PU_CXXSTDFLAGS)
AR := $(PU_AR)

PS3_LIBDIR := $(HOST_PATH)/target/ppu/lib
PS3_LIBDIRS := -L$(PS3_LIBDIR)
PS3_INCLUDES := -I$(HOST_PATH)/target/ppu/include

LDFLAGS	+= $(PU_LDLIBS)

ALL_PU_CXXFLAGS := $(CXXFLAGS) $(PU_MACHINE_OPTION) $(PU_CWARNFLAGS)

ifeq "$(HK_BUILD_SPEC)" "fulldebug"
	ALL_PU_CXXFLAGS += -g -G0 -fno-inline -fexceptions -frtti  -fno-strict-aliasing
	INCLUDE:= -I.. -I../../../sdk/include/physics -I../../../sdk/include/common -I../../../sdk/include/animation -I../../../sdk/include/complete -I../../common -I../../animation -I../../physics
	DEFINES += -DHK_DEBUG
	OBJDIR := ../../../obj/ps3-gcc/havok_2+mayhem_client/$(HK_BUILD_SPEC)/$(LIB_NAME)
	OUTPUT_DIR := ../../../lib/ps3-gcc/$(HK_BUILD_SPEC)
	TARGET := $(OUTPUT_DIR)/lib$(LIB_NAME).a
endif

ifeq "$(HK_BUILD_SPEC)" "debug"
	ALL_PU_CXXFLAGS += -g1 -G0 -O3 -fexceptions -frtti  -fno-strict-aliasing -finline-limit=8000 -fno-implement-inlines -fno-inline-functions -Winline -funit-at-a-time --param large-function-insns=20000 --param inline-unit-growth=10000
	INCLUDE:= -I.. -I../../../sdk/include/physics -I../../../sdk/include/common -I../../../sdk/include/animation -I../../../sdk/include/complete -I../../common -I../../animation -I../../physics
	DEFINES += -DHK_DEBUG
	LDFLAGS += -g1
	OBJDIR := ../../../obj/ps3-gcc/havok_2+mayhem_client/$(HK_BUILD_SPEC)/$(LIB_NAME)
	OUTPUT_DIR := ../../../lib/ps3-gcc/$(HK_BUILD_SPEC)
	TARGET := $(OUTPUT_DIR)/lib$(LIB_NAME).a	
endif

ifeq "$(HK_BUILD_SPEC)" "release"
	ALL_PU_CXXFLAGS += -G0 -O3 -fexceptions -frtti  -fno-strict-aliasing -finline-limit=8000 -fno-implement-inlines -fno-inline-functions -Winline -funit-at-a-time --param large-function-insns=20000 --param inline-unit-growth=10000
	INCLUDE:= -I.. -I../../../sdk/include/physics -I../../../sdk/include/common -I../../../sdk/include/animation -I../../../sdk/include/complete -I../../common -I../../animation -I../../physics
	OBJDIR := ../../../obj/ps3-gcc/havok_2+mayhem_client/$(HK_BUILD_SPEC)/$(LIB_NAME)
	OUTPUT_DIR := ../../../lib/ps3-gcc/$(HK_BUILD_SPEC)
	TARGET := $(OUTPUT_DIR)/lib$(LIB_NAME).a	
endif

COMPILE_COMMAND := $(CXX) $(CPPFLAGS) $(ALL_PU_CXXFLAGS)
LINK_EXECUTABLE := $(PU_CXX) $(LDFLAGS)
LINK_SPECIALS := 
LINK_LIBRARY := $(PU_AR) -rcu


#################

STD_INCLUDES :=  $(PS3_INCLUDES) 

COMPILED_LIBS += 
LINK_LIBS:=$(addprefix -l, $(LIBS) $(STD_LIBS)) $(COMPILED_LIBS)
STD_LIBS :=
LINK_EXE_PRE = 
LINK_EXE = $(PU_CXX) $(LDFLAGS) 
LINK_EXE_POST = 
LINK_LIB = $(PU_AR) -rcu

# configuration specifics...
WARNING	:=
DEFINES +=
COMPILE = $(CXX) $(CPPFLAGS) $(ALL_PU_CXXFLAGS) $(INCLUDE) $(OPTIMISE) $(WARNING) $(DEFINES) $(STD_INCLUDES)

SRCX:= $(notdir $(SRC))
OBJSUFFIX := .o
OBJ := $(addprefix $(OBJDIR)/, $(addsuffix $(OBJSUFFIX), $(basename $(SRCX))))
vpath %.cpp $(sort $(dir $(SRC)))
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


$(TARGET) : $(OBJ) force_linking
	@$(LINK_LIB) $@ $(OBJ)
	
force_linking:	


# COMPILE is a 'function' taking arguments (source, object, extra flags)
$(OBJDIR)/%.o : %.cpp 
	@echo $(notdir $<)
	@$(COMPILE) $(INCLUDE) -c $< -o $@

clean:
ifeq "$(HK_BUILD_ENV)" "win32"
	-cmd /C "del /S /F /Q $(subst /,\, $(OBJDIR)/*.o $(TARGET))"
else
	rm $(OBJDIR)/*.o $(TARGET)
endif

