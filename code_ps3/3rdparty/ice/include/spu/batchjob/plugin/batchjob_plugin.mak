## batchjob_plugin.mak is made to be included into a Makefile that defines the following:
#
# SPU_SOURCE_FILES = <list of source files; supported extensions are .cpp, .c, .spu, and .s>
# SPU_TARGET = <plugin name without extension>
#
## paths to various directories, compatible with those defined in ice/src/runtime/branchspec.mak:
# HOST = <linux or win32>
# SDK_ROOT = <path to your cell sdk directory>
# ICE_ROOT = <path to ice>
# ICE_BUILD = <path to your build directory for intermediate files>
# ICE_OUTPUT = <path to your output directory for output files>
#
## batchjob_plugin.mak builds $(ICE_OUTPUT)/$(SPU_TARGET).spu.bin from the source files 
## listed relative to ./ in $(SPU_SOURCE_FILES), dumping all intermediate files in 
## $(ICE_BUILD)/$(HOST)/$(SPU_TARGET).
##################################################################################################

ICE_BIN 	= $(ICE_ROOT)/bin
ICE_SRC		= $(ICE_ROOT)/src/runtime/ice
ICE_BATCHJOB_PLUGIN = $(ICE_SRC)/batchjob/plugin
OBJDIR		= $(ICE_BUILD)/$(HOST)/$(SPU_TARGET)

SPU_CC		= $(SDK_ROOT)/host-$(HOST)/spu/bin/spu-lv2-gcc
SPU_AS		= $(SDK_ROOT)/host-$(HOST)/spu/bin/spu-lv2-gcc -x assembler
SPU_LD		= $(SDK_ROOT)/host-$(HOST)/spu/bin/spu-lv2-gcc
SPU_OBJCOPY	= $(SDK_ROOT)/host-$(HOST)/spu/bin/spu-lv2-objcopy
FRONTEND    	= $(ICE_BIN)/$(HOST)/frontend

SPU_SOURCE_SPU 	= $(filter %.spu, $(SPU_SOURCE_FILES))
SPU_SOURCE_CPP 	= $(filter %.cpp, $(SPU_SOURCE_FILES))
SPU_SOURCE_C 	= $(filter %.c, $(SPU_SOURCE_FILES))
SPU_SOURCE_S 	= $(filter %.s, $(SPU_SOURCE_FILES))

SPU_TARGET_BIN = $(SPU_TARGET).spu.bin
SPU_TARGET_ELF = $(SPU_TARGET).spu.elf
SPU_PATHED_OBJS = $(addprefix $(OBJDIR)/, $(SPU_SOURCE_SPU:.spu=.spu.o)) \
		  $(addprefix $(OBJDIR)/, $(SPU_SOURCE_CPP:.cpp=.cpp.o)) \
		  $(addprefix $(OBJDIR)/, $(SPU_SOURCE_C:.c=.c.o)) \
		  $(addprefix $(OBJDIR)/, $(SPU_SOURCE_S:.s=.s.o))
SPU_PATHED_DEPS = $(addprefix $(OBJDIR)/, $(SPU_SOURCE_SPU:.spu=.spu.d)) \
		  $(addprefix $(OBJDIR)/, $(SPU_SOURCE_CPP:.cpp=.cpp.d)) \
		  $(addprefix $(OBJDIR)/, $(SPU_SOURCE_C:.c=.c.d))
SPU_PATHED_DEPS_TMP 	= $(addprefix $(OBJDIR)/, $(SPU_SOURCE_CPP:.cpp=.cpp.d.tmp)) \
			  $(addprefix $(OBJDIR)/, $(SPU_SOURCE_C:.c=.c.d.tmp))
SPU_PATHED_SOURCE_FILES	= $(addprefix $(OBJDIR)/, $(SPU_SOURCE_FILES))
SPU_PATHED_TARGET_BIN 	= $(addprefix $(ICE_OUTPUT)/, $(SPU_TARGET_BIN))
SPU_PATHED_TARGET_ELF 	= $(addprefix $(OBJDIR)/, $(SPU_TARGET_ELF))


SPU_OPTIMISATION_OPTIONS = -Os --param max-inline-insns-single=300 --param inline-unit-growth=200

SPU_COMPILE_OPTIONS = -I. -I$(ICE_SRC)/system -I$(ICE_SRC)/batchjob -include $(ICE_SRC)/system/spu/icetypes.h
SPU_COMPILE_OPTIONS += -g $(SPU_OPTIMISATION_OPTIONS) -nostartfiles -fpic -DNDEBUG=1 -MD
SPU_COMPILE_OPTIONS += -Wextra -Wall -Winline -Wshadow -Wpointer-arith -Wcast-qual -Wwrite-strings
SPU_COMPILE_OPTIONS += -Wsign-compare -Wmissing-noreturn -Werror -Wundef
SPU_COMPILE_OPTIONS += -Wreturn-type -Wunused -Wchar-subscripts -Wconversion -Wendif-labels -Wformat
SPU_COMPILE_OPTIONS += -Winit-self -Wmissing-braces -Wstrict-aliasing=2

SPU_JOB_LINK_OPTIONS = -g -fpic -Wl,-q -nostartfiles

FRONTEND_OPTIONS = -silent -dwarf -D NDEBUG=1

SPU_PATHED_LIBS = 
SPU_PATHED_OBJS += $(OBJDIR)/batchjob_plugincrt0.spu.o

# Kill the default rules!
.SUFFIXES:

.PRECIOUS: %.spu.elf

all: $(SPU_PATHED_TARGET_BIN)

$(SPU_PATHED_TARGET_BIN) : $(SPU_PATHED_TARGET_ELF)
	@echo Creating $@ for SPU from $< by objcopy
	@mkdir -p $(@D)
	@$(SPU_OBJCOPY) -O binary $< $@

$(SPU_PATHED_TARGET_ELF) : $(SPU_PATHED_OBJS) $(SPU_PATHED_LIBS) $(ICE_BATCHJOB_PLUGIN)/batchjob_plugin.ld
	@echo Linking $@ for SPU from $<
	@mkdir -p $(@D)
	@$(SPU_LD) $(SPU_PATHED_OBJS) $(SPU_PATHED_LIBS) $(SPU_JOB_LINK_OPTIONS) -T $(ICE_BATCHJOB_PLUGIN)/batchjob_plugin.ld -o $@

$(OBJDIR)/batchjob_plugincrt0.spu.o : $(ICE_BATCHJOB_PLUGIN)/batchjob_plugincrt0.spu.s
	@echo Assembling $@ for SPU from $<
	@mkdir -p $(@D)
	@$(SPU_AS) $(SPU_COMPILE_OPTIONS) -x assembler -c $< -o $@

$(OBJDIR)/%.cpp.o : %.cpp
	@echo Compiling $@ for SPU from $<
	@mkdir -p $(@D)
	@$(SPU_CC) $(SPU_COMPILE_OPTIONS) -c $< -o $@

$(OBJDIR)/%.c.o : %.c
	@echo Compiling $@ for SPU from $<
	@mkdir -p $(@D)
	@$(SPU_CC) $(SPU_COMPILE_OPTIONS) -c $< -o $@

$(OBJDIR)/%.s.o : %.s
	@echo Assembling $@ for SPU from $<
	@mkdir -p $(@D)
	@$(SPU_AS) $(SPU_COMPILE_OPTIONS) -c $< -o $@

$(OBJDIR)/%.spu.o : $(OBJDIR)/%.spu.S
	@echo Assembling $@ for SPU from $<
	@mkdir -p $(@D)
	@$(SPU_AS) $(SPU_COMPILE_OPTIONS) -c $< -o $@

$(OBJDIR)/%.spu.S : %.spu
	@mkdir -p $(dir $@)
	@$(FRONTEND) $(FRONTEND_OPTIONS) $< -o $@

clean:
	@rm -f $(SPU_PATHED_TARGET_BIN) $(SPU_PATHED_TARGET_ELF) $(SPU_PATHED_OBJS) $(SPU_PATHED_DEPS) $(SPU_PATHED_DEPS_TMP)

# Automatic dependency generation (.d files)
$(OBJDIR)/%.spu.d : %.spu
	@echo " * Updating SPU dependencies for $<"
	@mkdir -p $(dir $@)
	@rm -f $@
	@$(FRONTEND) -MM -MQ "$(OBJDIR)/$*.spu.o" -MF $@ $(FRONTEND_OPTIONS) $<

$(OBJDIR)/%.cpp.d : %.cpp
	@echo " * Updating SPU dependencies for $<"
	@mkdir -p $(dir $@)
	@rm -f $@
	@$(SPU_CC) -MM -MQ "$(OBJDIR)/$*.spu.o" -o $@.tmp $(SPU_COMPILE_OPTIONS) $<
	@cat $@.tmp | sed "s%\($*\)\.cpp\.o[ :]*%\1.cpp.o $@ : %g" > $@
	@[ -s $@ ] || rm -f $@ $@.spu.cpp.d.tmp
	@rm -f $@.tmp

$(OBJDIR)/%.c.d : %.c
	@echo " * Updating SPU dependencies for $<"
	@mkdir -p $(dir $@)
	@rm -f $@
	@$(SPU_CC) -MM -MQ "$(OBJDIR)/$*.spu.o" -o $@.tmp $(SPU_COMPILE_OPTIONS) $<
	@cat $@.tmp | sed "s%\($*\)\.c\.o[ :]*%\1.c.o $@ : %g" > $@
	@[ -s $@ ] || rm -f $@ $@.spu.c.d.tmp
	@rm -f $@.tmp

# Include the per-source-file dependencies, unless we're building the
# "clean" target.
ifeq ($(filter %clean , $(MAKECMDGOALS)),)
-include $(SPU_PATHED_DEPS)
endif
