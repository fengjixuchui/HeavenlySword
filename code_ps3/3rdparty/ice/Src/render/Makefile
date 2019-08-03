include ../../branchspec.mak

ifeq ($(OS),CYGWIN)
HOST       	= win32
HOSTDEFS	= -DHOST_WIN32
else
HOST       	= linux
HOSTDEFS 	= -DHOST_LINUX
endif

CC 		= $(SDK_ROOT)/host-$(HOST)/ppu/bin/ppu-lv2-gcc
AR		= $(SDK_ROOT)/host-$(HOST)/ppu/bin/ppu-lv2-ar
AS		= $(SDK_ROOT)/host-$(HOST)/ppu/bin/ppu-lv2-as

CC_SPU	= $(SDK_ROOT)/host-$(HOST)/spu/bin/spu-lv2-gcc
AR_SPU	= $(SDK_ROOT)/host-$(HOST)/spu/bin/spu-lv2-ar
AS_SPU	= $(SDK_ROOT)/host-$(HOST)/spu/bin/spu-lv2-as

INCDIR 		= -I../../../../include/common
INCDIR_S	= $(INCDIR) \
		  -I../../../../include/spu \
		  -I../../$(WWS_ROOT)/projects/job/include/spu

CFLAGS_BASE	= $(INCDIR) -I../../../../include/ppu -include icetypes.h -maltivec -ffast-math -fno-exceptions -fno-rtti -fprefetch-loop-arrays -fstrict-aliasing -W -Wall -Wstrict-aliasing=2 -g -DICERENDER_090
# -Wno-unused-function -Wno-switch 
CFLAGS_D	= $(CFLAGS_BASE) -DICEDEBUG=1 -DICERENDER_FAST=0
CFLAGS_A	= $(CFLAGS_BASE) -O2 -DICERENDER_FAST=1
CFLAGS		= $(CFLAGS_BASE) -O2 -DICERENDER_FAST=0

CFLAGS_S	= $(INCDIR_S) -include icetypes.h -ffast-math -fno-exceptions -fno-rtti -W -Wall -Wstrict-aliasing=2 -ffunction-sections -fdata-sections -fstrict-aliasing
# -Wno-unused-function -Wno-switch 
CFLAGS_S	+= -g -DICERENDER_090
CFLAGS_S_D	= $(CFLAGS_S) -DICEDEBUG=1
CFLAGS_S	+= -O2 -DICERENDER_FAST=0

LIBRARY 			= libicerender.a
LIBRARY_ASM			= libicerender-asm.a
LIBRARY_DEBUG 		= libicerender-debug.a
LIBRARY_SPU 		= libicerender-spu.a
LIBRARY_SPU_DEBUG	= libicerender-spu-debug.a

OBJS		= icecommandcontext.o icerender.o icerendertarget.o icerenderprocs.o icedisasm.o icememory.o icevertexprogram.o
OBJS_A		= icecommandcontext.oa icecommandcontextsafe.oa icecommandcontextunsafe.oa icerender.oa icerendertarget.oa icerenderprocs.oa icedisasm.oa icememory.oa icevertexprogram.oa
OBJS_D		= icecommandcontext.od icerender.od icerendertarget.od icerenderprocs.od icedisasm.od icememory.od icevertexprogram.od
OBJS_S		= icecommandcontext.os icerendertarget.os icerenderprocs.os icevertexprogram.os
OBJS_S_D	= icecommandcontext.osd icerendertarget.osd icerenderprocs.osd icevertexprogram.osd

PUBLISH_LIB_DIR = ../../../../lib/ppu/
PUBLISH_SPULIB_DIR = ../../../../lib/spu/

all: $(PUBLISH_LIB_DIR)$(LIBRARY) $(PUBLISH_LIB_DIR)$(LIBRARY_ASM) $(PUBLISH_LIB_DIR)$(LIBRARY_DEBUG) $(PUBLISH_SPULIB_DIR)$(LIBRARY_SPU) $(PUBLISH_SPULIB_DIR)$(LIBRARY_SPU_DEBUG)

p4edit:
	-p4 edit $(PUBLISH_LIB_DIR)$(LIBRARY) $(PUBLISH_LIB_DIR)$(LIBRARY_ASM) $(PUBLISH_LIB_DIR)$(LIBRARY_DEBUG) $(PUBLISH_H_DIR)...h

$(PUBLISH_LIB_DIR)$(LIBRARY): $(OBJS)
	mkdir -p $(PUBLISH_LIB_DIR)
	rm -f $(PUBLISH_LIB_DIR)$(LIBRARY)
	$(AR) rs $(PUBLISH_LIB_DIR)$(LIBRARY) $(OBJS) 

$(PUBLISH_LIB_DIR)$(LIBRARY_ASM): $(OBJS_A)
	mkdir -p $(PUBLISH_LIB_DIR)
	rm -f $(PUBLISH_LIB_DIR)$(LIBRARY_ASM)
	$(AR) rs $(PUBLISH_LIB_DIR)$(LIBRARY_ASM) $(OBJS_A) 

$(PUBLISH_LIB_DIR)$(LIBRARY_DEBUG): $(OBJS_D) $(LIBGCM) $(LIBSYSUTIL)
	mkdir -p $(PUBLISH_LIB_DIR)
	rm -f $(PUBLISH_LIB_DIR)$(LIBRARY_DEBUG)
	$(AR) rs $(PUBLISH_LIB_DIR)$(LIBRARY_DEBUG) $(OBJS_D)

$(PUBLISH_SPULIB_DIR)$(LIBRARY_SPU): $(OBJS_S)
	mkdir -p $(PUBLISH_SPULIB_DIR)
	rm -f $(PUBLISH_SPULIB_DIR)$(LIBRARY_SPU)
	$(AR_SPU) rs $(PUBLISH_SPULIB_DIR)$(LIBRARY_SPU) $(OBJS_S) 

$(PUBLISH_SPULIB_DIR)$(LIBRARY_SPU_DEBUG): $(OBJS_S_D)
	mkdir -p $(PUBLISH_SPULIB_DIR)
	rm -f $(PUBLISH_SPULIB_DIR)$(LIBRARY_SPU_DEBUG)
	$(AR_SPU) rs $(PUBLISH_SPULIB_DIR)$(LIBRARY_SPU_DEBUG) $(OBJS_S_D)

%.o: %.cpp
	$(CC) $(CFLAGS) -c $< -o $@

%.oa: %.cpp
	$(CC) $(CFLAGS_A) -c $< -o $@

%.o: %.ppu.S
	$(CC) -c $< -o $@

%.oa: %.ppu.S
	$(CC) -c $< -o $@

%.od: %.cpp
	$(CC) $(CFLAGS_D) -c $< -o $@

%.od: %.ppu.S
	$(CC) -c $< -o $@

%.os: %.cpp
	$(CC_SPU) $(CFLAGS_S) -c $< -o $@

%.os: %.spu.S
	$(CC_SPU) -c $< -o $@

%.osd: %.cpp
	$(CC_SPU) $(CFLAGS_S_D) -c $< -o $@

src-tar: all
	tar cvfz icerender-src-`date +'%Y-%m-%d'`.tgz Makefile $(OBJS:.o=.cpp) *.inl *.h $(LIBRARY) $(LIBRARY_DEBUG) $(LIBRARY_SPU) $(LIBRARY_SPU_DEBUG)
#	tar cvfz icerender-`date +'%Y-%m-%d'`.tgz README Makefile $(OBJS:.o=.cpp) *.inl *.h  $(LIBRARY) $(LIBRARY_DEBUG) $(LIBRARY_SPU) $(LIBRARY_SPU_DEBUG)

tar: all
	tar cvfz icerender-`date +'%Y-%m-%d'`.tgz *.h $(LIBRARY) $(LIBRARY_DEBUG) $(LIBRARY_SPU) $(LIBRARY_SPU_DEBUG)

clean:
	rm -f $(OBJS)
	rm -f $(OBJS_A)
	rm -f $(OBJS_D)
	rm -f $(OBJS_S)
	rm -f $(OBJS_S_D)
	rm -f $(PUBLISH_LIB_DIR)$(LIBRARY)
	rm -f $(PUBLISH_LIB_DIR)$(LIBRARY_ASM)
	rm -f $(PUBLISH_LIB_DIR)$(LIBRARY_DEBUG)
	rm -f $(PUBLISH_SPULIB_DIR)$(LIBRARY_SPU)
	rm -f $(PUBLISH_SPULIB_DIR)$(LIBRARY_SPU_DEBUG)
