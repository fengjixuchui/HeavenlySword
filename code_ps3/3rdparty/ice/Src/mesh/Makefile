include ../../branchspec.mak

MAKE_FSELF  = $(SDK_ROOT)/host-$(HOST)/bin/make_fself
PPU_CC		= $(SDK_ROOT)/host-$(HOST)/ppu/bin/ppu-lv2-gcc
PPU_LD		= $(SDK_ROOT)/host-$(HOST)/ppu/bin/ppu-lv2-gcc
PPU_AR		= $(SDK_ROOT)/host-$(HOST)/ppu/bin/ppu-lv2-ar
PPU_RANLIB	= $(SDK_ROOT)/host-$(HOST)/ppu/bin/ppu-lv2-ranlib
PPU_OBJCOPY	= $(SDK_ROOT)/host-$(HOST)/ppu/bin/ppu-lv2-objcopy
SPU_CC		= $(SDK_ROOT)/host-$(HOST)/spu/bin/spu-lv2-gcc
SPU_AS		= $(SDK_ROOT)/host-$(HOST)/spu/bin/spu-lv2-gcc
SPU_LD		= $(SDK_ROOT)/host-$(HOST)/spu/bin/spu-lv2-gcc
SPU_OBJCOPY	= $(SDK_ROOT)/host-$(HOST)/spu/bin/spu-lv2-objcopy

FRONTEND    	= $(ICE_BIN)/$(HOST)/frontend
CGFX2BIN	= $(ICE_BIN)/$(HOST)/cgfx2bin
SPU_ELF2BIN	= $(ICE_BIN)/$(HOST)/spu-elf2bin
BIN2C		= $(ICE_BIN)/$(HOST)/bin2c



ICE_BASE	= ..
ICEMESH		= .
ICEMESHSPU	= $(ICEMESH)/spu

ICEINC		= $(ICE_BASE)/../../../include
ICEINCPPU	= $(ICEINC)/ppu
ICEINCSPU	= $(ICEINC)/spu
ICEINCCOMMON	= $(ICEINC)/common

ICEINCRENDER	= $(ICEINCPPU)/render

ICELIB		= $(ICE_BASE)/../../../lib
ICELIBPPU	= $(ICELIB)/ppu
ICELIBSPU	= $(ICELIB)/spu

ICEJOBELF	= $(ICE_BASE)/../../../jobelf
ICEJOBELFSPU	= $(ICEJOBELF)/spu

WWS_JOB_MANAGER_USE_C_VERSION 	= 0
WWS_JOB_MANAGER_RELEASE		= 1

ifeq ($(WWS_JOB_MANAGER_USE_C_VERSION), 0)
#Must be consistent with LsMemoryLimits::kJobAreaBasePageNum<<10, which is currently 16<<10
WWSJOB_SPUMODULE_LSA_OFFSET = 0x04000
else
WWSJOB_SPUMODULE_LSA_OFFSET = 0x0C000	#C++ WwsJobManager value
endif

WWS_SYSTEM_BASE	= $(ICE_BASE)/../$(WWS_ROOT)/projects/system
WWSSYSTEMPPUDIR	= $(WWS_SYSTEM_BASE)/include/ppu
WWSSYSTEMSPUDIR	= $(WWS_SYSTEM_BASE)/include/spu
WWS_BASE	= $(ICE_BASE)/../$(WWS_ROOT)/projects/job
WWSCOMMONDIR	= $(WWS_BASE)/include/common
WWSPPUDIR	= $(WWS_BASE)/include/ppu
WWSSPUDIR	= $(WWS_BASE)/include/spu

ifeq ($(WWS_JOB_MANAGER_RELEASE), 1)
WWSLIBDIR	= $(ICE_BASE)/../$(WWS_ROOT)/lib/release
else
WWSLIBDIR	= $(ICE_BASE)/../$(WWS_ROOT)/lib/debug
endif

JOBCRT0		= $(WWSLIBDIR)/spu/jobcrt0.spu.o
LIBJOBAPISPU	= $(WWSLIBDIR)/spu/libjobapi.spu.a

FRONTEND_FLAGS	=

# Common compiler flags
DEF_FLAGS 	= -DWWS_JOB_USE_C_VERSION=$(WWS_JOB_MANAGER_USE_C_VERSION) \
		  -DJOBMANAGER_BASE_LSA=$(WWSJOB_SPUMODULE_LSA_OFFSET) \
		  -DUSERSTRING=$(USER) 

REL_FLAGS	= -DNDEBUG=1

DEBUG_FLAGS	= -DICEDEBUG=1 -DMESH_PRINTF_ENABLED=1

#
# PPU compiler flags
#
PPU_INCDIR	= -I$(ICEINCCOMMON) \
		  -I$(ICEINCPPU) \
		  -I$(ICEMESH) \
		  -I$(ICEINCRENDER) \
		  -I$(WWSSYSTEMPPUDIR) \
		  -I$(WWSCOMMONDIR) \
		  -I$(WWSPPUDIR)

PPU_REL_FLAGS	= $(RELEASE_FLAGS)
PPU_DEBUG_FLAGS = $(DEBUG_FLAGS)
PPU_INC_FLAGS   = $(PPU_INCDIR) -include $(ICEINCPPU)/icetypes.h 
PPU_OPT_FLAGS   = -maltivec -ffast-math -fno-exceptions -fno-rtti -fprefetch-loop-arrays 
PPU_WARN_FLAGS	= -Wall -Wextra -W -Wno-unused-function -Wno-switch 
PPU_DEF_FLAGS   = -DICE_TARGET_PS3 -DICE_IMAGE_BIN=$(ICE_IMAGE_BIN) $(HOSTDEFS) $(DEF_FLAGS)

# Don't edit this following line, edit the one's above.
PPU_CFLAGS	= -O4 $(PPU_REL_FLAGS) $(PPU_INC_FLAGS) $(PPU_OPT_FLAGS) $(PPU_WARN_FLAGS) $(PPU_DEF_FLAGS)
PPU_CFLAGS_D	= -O0 $(PPU_DEBUG_FLAGS) $(PPU_INC_FLAGS) $(PPU_OPT_FLAGS) $(PPU_WARN_FLAGS) $(PPU_DEF_FLAGS)
PPU_LDFLAGS	= 

#
# SPU compiler flags
#
SPU_INCDIR	= -I$(ICEINCCOMMON) \
		  -I$(ICEINCSPU) \
		  -I$(ICEMESH) \
		  -I$(WWSSYSTEMSPUDIR) \
		  -I$(WWSCOMMONDIR) \
		  -I$(WWSSPUDIR)

SPU_REL_FLAGS	= $(RELEASE_FLAGS)
SPU_DEBUG_FLAGS	= $(DEBUG_FLAGS)
SPU_INC_FLAGS	= $(SPU_INCDIR) -include $(ICEINCSPU)/icetypes.h
SPU_OPT_FLAGS	= -ffast-math -fno-exceptions -fno-rtti
PPU_WARN_FLAGS	= -Wall -Wextra
SPU_DEF_FLAGS	= 

# Don't edit this following line, edit the one's above.
SPU_CFLAGS	= -O2 $(SPU_REL_FLAGS) $(SPU_INC_FLAGS) $(SPU_OPT_FLAGS) $(SPU_WARN_FLAGS) $(SPU_DEF_FLAGS)
SPU_CFLAGS_D	= -O2 $(SPU_DEBUG_FLAGS) $(SPU_INC_FLAGS) $(SPU_OPT_FLAGS) $(SPU_WARN_FLAGS) $(SPU_DEF_FLAGS)
SPU_LDFLAGS	=

# This directory must be deep enough so that none of the relative
# source path names pop out of the build directory and dump their
# output files in random spots in the source tree.
OBJDIR		= $(ICE_BUILD)/$(HOST)/libs/runtime/ice/mesh

WWSJOB_LIBS	= $(LIBJOBAPISPU) $(JOBCRT0)
WWSJOB_LDFLAGS	= -nostdlib \
		  $(WWSJOB_LIBS) \
		  -T $(WWS_BASE)/src/spu/job_module.ld \
		  -Wl,--warn-constructors \
		  -Wl,--defsym,kJobMemoryBaseAddr=$(WWSJOB_SPUMODULE_LSA_OFFSET)

# ----------------------------------------------------------------------------------------------------
#   The files for the mesh processing frontend running on PPU.
# ----------------------------------------------------------------------------------------------------

FESRCS		= $(ICEMESH)/icemeshfrontend.cpp

FEOBJS		= $(addprefix $(OBJDIR)/, $(FESRCS:.cpp=.o))
FEOBJS_D	= $(addprefix $(OBJDIR)/, $(FESRCS:.cpp=.od))
FEDEPS		= $(FEOBJS:.o=.d)

# ----------------------------------------------------------------------------------------------------
#   The files for mesh processing running on SPU or PPU.
# ----------------------------------------------------------------------------------------------------

MESHCPPSRCS	= $(ICEMESH)/icemesh.cpp \
		  $(ICEMESH)/icemeshblend.cpp \
		  $(ICEMESH)/icemeshcommand.cpp \
		  $(ICEMESH)/icemeshcompress.cpp \
		  $(ICEMESH)/icemeshconstdata.cpp \
		  $(ICEMESH)/icemeshdecompress.cpp \
		  $(ICEMESH)/icemeshmemcopy.cpp \
		  $(ICEMESH)/icemeshmisc.cpp \
		  $(ICEMESH)/icemeshpm.cpp \
		  $(ICEMESH)/icemeshshadow.cpp \
		  $(ICEMESH)/icemeshskin.cpp \
		  $(ICEMESH)/icemeshtrim.cpp \
		  $(ICEMESH)/icemeshdm.cpp \
		  $(ICEMESH)/icemeshstats.cpp

MESHSPUSRCS	= $(ICEMESHSPU)/cmd_setup_matrices.spu \
		  $(ICEMESHSPU)/cmd_setup_nvstream.spu \
		  $(ICEMESHSPU)/cmd_setup_skinning.spu \
		  $(ICEMESHSPU)/cmd_skin_object.spu \
		  $(ICEMESHSPU)/const_data.spu \
		  $(ICEMESHSPU)/copybytes.spu \
		  $(ICEMESHSPU)/copyqwords.spu \
		  $(ICEMESHSPU)/decomp_index_6b.spu \
		  $(ICEMESHSPU)/decomp_f32.spu \
		  $(ICEMESHSPU)/decomp_f16.spu \
		  $(ICEMESHSPU)/decomp_i16_u8.spu \
		  $(ICEMESHSPU)/decomp_x11y11z10.spu \
		  $(ICEMESHSPU)/decomp_matrices.spu \
		  $(ICEMESHSPU)/decomp_swformat.spu \
		  $(ICEMESHSPU)/blenduniform.spu \
		  $(ICEMESHSPU)/extrudeprofileedges.spu \
		  $(ICEMESHSPU)/generatefacingtable.spu \
		  $(ICEMESHSPU)/generateprofileedgelist.spu \
		  $(ICEMESHSPU)/trimtriangles.spu \
		  $(ICEMESHSPU)/insert_f32.spu \
		  $(ICEMESHSPU)/insert_i16.spu \
		  $(ICEMESHSPU)/insert_f16.spu \
		  $(ICEMESHSPU)/insert_u8.spu \
		  $(ICEMESHSPU)/insert_x11y11z10.spu \
		  $(ICEMESHSPU)/shufs.spu \
		  $(ICEMESHSPU)/skinning.spu \
		  $(ICEMESHSPU)/comp_index_6b.spu \
		  $(ICEMESHSPU)/pm_continuous_tower.spu \
		  $(ICEMESHSPU)/pm_discrete_tower.spu \
		  $(ICEMESHSPU)/selectverts_4b.spu \
		  $(ICEMESHSPU)/selectverts_8b.spu \
		  $(ICEMESHSPU)/selectverts_12b.spu \
		  $(ICEMESHSPU)/selectverts_16Nb.spu \
		  $(ICEMESHSPU)/selectverts_24b.spu \
		  $(ICEMESHSPU)/selectverts_40b.spu \
		  $(ICEMESHSPU)/selectverts_generic.spu

PPUMESHCPPOBJS	= $(addprefix $(OBJDIR)/, $(MESHCPPSRCS:.cpp=.o))
PPUMESHCPPOBJS_D= $(addprefix $(OBJDIR)/, $(MESHCPPSRCS:.cpp=.od))
PPUMESHDEPS	= $(PPUMESHCPPOBJS:.o=.d)

SPUMESHCPPOBJS	= $(addprefix $(OBJDIR)/, $(MESHCPPSRCS:.cpp=.spu_shared.o))
SPUMESHCPPOBJS_D= $(addprefix $(OBJDIR)/, $(MESHCPPSRCS:.cpp=.spu_shared.od))
SPUMESHSPUOBJS	= $(addprefix $(OBJDIR)/, $(MESHSPUSRCS:.spu=.spu.o))
SPUMESHOBJS	= $(SPUMESHCPPOBJS) $(SPUMESHSPUOBJS)
SPUMESHOBJS_D	= $(SPUMESHCPPOBJS_D) $(SPUMESHSPUOBJS)
SPUMESHDEPS	= $(SPUMESHOBJS:.o=.d)

JOBMESH		= $(OBJDIR)/icemesh.spu.elf
JOBMESH_D	= $(OBJDIR)/icemesh-debug.spu.elf

OBJS		= $(FEOBJS) $(FEOBJS_D) $(PPUMESHCPPOBJS) $(PPUMESHCPPOBJS_D) $(SPUMESHOBJS) $(SPUMESHOBJS_D)
DEPS		= $(FEDEPS) $(PPUMESHDEPS) $(SPUMESHDEPS)

# Search path for .cpp files
vpath	%.cpp	$(ICEMESH)
# Search path for .h files
vpath	%.h	$(ICEMESH)
# Search path for .spu files
vpath	%.spu	$(ICEMESHSPU)

LIBICEMESHFE	= $(ICELIBPPU)/libicemeshfrontend.a
LIBICEMESHFE_D	= $(ICELIBPPU)/libicemeshfrontend-debug.a
LIBICEMESH	= $(ICELIBPPU)/libicemesh.a
LIBICEMESH_D	= $(ICELIBPPU)/libicemesh-debug.a
ICEMESHSPUELF	= $(ICEJOBELFSPU)/icemesh.spu.elf
ICEMESHSPUELF_D	= $(ICEJOBELFSPU)/icemesh-debug.spu.elf

# Kill the default rules!
.SUFFIXES:

.PHONY: clean
all: $(LIBICEMESHFE) $(LIBICEMESHFE_D) $(LIBICEMESH) $(LIBICEMESH_D) $(ICEMESHSPUELF) $(ICEMESHSPUELF_D)
release: $(LIBICEMESHFE) $(LIBICEMESH) $(ICEMESHSPUELF)
debug: $(LIBICEMESHFE_D) $(LIBICEMESH_D) $(ICEMESHSPUELF_D)
frontend: $(LIBICEMESHFE) $(LIBICEMESHFE_D)
ppu: $(LIBICEMESH) $(LIBICEMESH_D) 
spu: $(ICEMESHSPUELF) $(ICEMESHSPUELF_D)

$(LIBICEMESHFE): $(FEOBJS)
	@echo " * Building library $@"
	@mkdir -p $(dir $@)
	@rm -f $@
	@$(PPU_AR) rs $(LIBICEMESHFE) $(FEOBJS)

$(LIBICEMESHFE_D): $(FEOBJS_D)
	@echo " * Building library $@"
	@mkdir -p $(dir $@)
	@rm -f $@
	@$(PPU_AR) rs $(LIBICEMESHFE_D) $(FEOBJS_D)

$(LIBICEMESH): $(PPUMESHCPPOBJS)
	@echo " * Building library $@"
	@mkdir -p $(dir $@)
	@rm -f $@
	@$(PPU_AR) rs $(LIBICEMESH) $(PPUMESHCPPOBJS)

$(LIBICEMESH_D): $(PPUMESHCPPOBJS_D)
	@echo " * Building library $@"
	@mkdir -p $(dir $@)
	@rm -f $@
	@$(PPU_AR) rs $(LIBICEMESH_D) $(PPUMESHCPPOBJS_D)

$(ICEMESHSPUELF): $(JOBMESH)
	@mkdir -p $(dir $@)
	@cp -f $(JOBMESH) $(ICEMESHSPUELF)

$(ICEMESHSPUELF_D): $(JOBMESH_D)
	@mkdir -p $(dir $@)
	@cp -f $(JOBMESH_D) $(ICEMESHSPUELF_D)

$(JOBMESH): $(SPUMESHOBJS) $(WWSJOB_LIBS)
	@echo " * Linking SPU job $@ for WWS JobManager"
	@mkdir -p $(dir $@)
	@$(SPU_LD) $(SPUMESHOBJS) $(SPU_LDFLAGS) -Ttext $(WWSJOB_SPUMODULE_LSA_OFFSET) $(WWSJOB_LDFLAGS) -o $@

$(JOBMESH_D): $(SPUMESHOBJS_D) $(WWSJOB_LIBS)
	@echo " * Linking SPU job $@ for WWS JobManager"
	@mkdir -p $(dir $@)
	@$(SPU_LD) $(SPUMESHOBJS_D) $(SPU_LDFLAGS) -Ttext $(WWSJOB_SPUMODULE_LSA_OFFSET) $(WWSJOB_LDFLAGS) -o $@

$(OBJDIR)/%.spu.cpp.o : %.spu.cpp
	@echo " * Compiling $< for SPU"
	@mkdir -p $(dir $@)
	@$(SPU_CC) $(SPU_CFLAGS) -c $< -o $@

$(OBJDIR)/%.spu.cpp.od : %.spu.cpp
	@echo " * Compiling (Debug) $< for SPU"
	@mkdir -p $(dir $@)
	@$(SPU_CC) $(SPU_CFLAGS_D) -c $< -o $@

$(OBJDIR)/%.spu_shared.o : %.cpp
	@echo " * Compiling $< for SPU from PPU .cpp file"
	@mkdir -p $(dir $@)
	@$(SPU_CC) $(SPU_CFLAGS) -c $< -o $@

$(OBJDIR)/%.spu_shared.od : %.cpp
	@echo " * Compiling (Debug) $< for SPU from PPU .cpp file"
	@mkdir -p $(dir $@)
	@$(SPU_CC) $(SPU_CFLAGS_D) -c $< -o $@

$(OBJDIR)/%.spu.o : %.spu.c
	@echo " * Compiling $< for SPU"
	@mkdir -p $(dir $@)
	@$(SPU_CC) $(SPU_CFLAGS) -c $< -o $@

$(OBJDIR)/%.spu.od : %.spu.c
	@echo " * Compiling (Debug) $< for SPU"
	@mkdir -p $(dir $@)
	@$(SPU_CC) $(SPU_CFLAGS_D) -c $< -o $@

$(OBJDIR)/%.spu.o : $(OBJDIR)/%.spu.S
	@echo " * Assembling $< for SPU"
	@$(SPU_AS) -c $< -o $@

$(OBJDIR)/%.spu.S : %.spu
	@mkdir -p $(dir $@)
	@$(FRONTEND) -silent $(FRONTEND_FLAGS) $< -o $@

$(OBJDIR)/%.o : %.cpp
	@echo " * Compiling $<"
	@mkdir -p $(dir $@)
	@$(PPU_CC) $(PPU_CFLAGS) -c $< -o $@

$(OBJDIR)/%.od : %.cpp
	@echo " * Compiling (Debug) $<"
	@mkdir -p $(dir $@)
	@$(PPU_CC) $(PPU_CFLAGS_D) -c $< -o $@

clean:
	rm -f $(DEPS) $(DEPS:.d=.d.tmp)
	rm -f $(OBJS) $(SPUMESHSPUOBJS:.o=.S)
	rm -f $(JOBMESH) $(JOBMESH_D)
	rm -f $(LIBICEMESHFE) $(LIBICEMESHFE_D) $(LIBICEMESH) $(LIBICEMESH_D) $(ICEMESHSPUELF) $(ICEMESHSPUELF_D)


# Automatic dependency generation (.d files)
$(OBJDIR)/%.spu.d : %.spu
	@echo " * Updating SPU dependencies for $<"
	@mkdir -p $(dir $@)
	@rm -f $@
	@$(FRONTEND) -MM -MQ "$(OBJDIR)/$*.spu.o" -MF $@ $(FRONTEND_FLAGS) $<

$(OBJDIR)/%.spu.d : %.spu.cpp
	@echo " * Updating SPU dependencies for $<"
	@mkdir -p $(dir $@)
	@rm -f $@
	@$(SPU_CC) -Werror -MM -MQ "$(OBJDIR)/$*.spu.o" -o $@.tmp $(SPU_CFLAGS) $<
	@cat $@.tmp | sed "s%\($*\)\.spu\.o[ :]*%\1.spu.o $@ : %g" > $@
	@[ -s $@ ] || rm -f $@ $@.spu.d.tmp
	@rm -f $@.tmp

$(OBJDIR)/%.d : %.cpp
	@echo " * Updating dependencies for $<"
	@mkdir -p $(dir $@)
	@rm -f $@
	@$(PPU_CC) -Werror -MM -MQ "$(OBJDIR)/$*.o" -o $@.tmp $(PPU_CFLAGS) $<
	@cat $@.tmp | sed "s%\($*\)\.o[ :]*%\1.o $@ : %g" > $@
	@[ -s $@ ] || rm -f $@ $@.d.tmp
	@rm -f $@.tmp

# Include the per-source-file dependencies, unless we're building the
# "clean" target.
ifeq ($(filter %clean , $(MAKECMDGOALS)),)
-include $(DEPS)
endif 
