.PHONY: clean all ppu spu linux win32

ICE = ../src/runtime/ice

all: ppu spu linux win32

clean: 

ppu: 

	# Ice Render
	mkdir -p ppu/render
	cp -f $(ICE)/render/*.h  ppu/render
	cp -f $(ICE)/render/*.inl  ppu/render

	# Mesh
	mkdir -p ppu/mesh
	cp -f $(ICE)/mesh/icemesh.h ppu/mesh
	cp -f $(ICE)/mesh/icemeshfrontend.h ppu/mesh
	cp -f $(ICE)/mesh/icemeshstructs.h ppu/mesh

	# BatchJob
	mkdir -p ppu/batchjob
	cp -f $(ICE)/batchjob/icebatchjob.h ppu/batchjob
	cp -f $(ICE)/batchjob/icebatchjobbuffers.h ppu/batchjob
	cp -f $(ICE)/batchjob/icebatchjobcore.h ppu/batchjob
	cp -f $(ICE)/batchjob/icebatchjobcore.inl ppu/batchjob
	cp -f $(ICE)/batchjob/icebatchjobdebug.h ppu/batchjob
	cp -f $(ICE)/batchjob/icebatchjobdebug.inl ppu/batchjob
	cp -f $(ICE)/batchjob/icebatchjobdispatcher.inl ppu/batchjob
	cp -f $(ICE)/batchjob/icebatchjoblist.h ppu/batchjob
	cp -f $(ICE)/batchjob/icebatchjobplugin.h ppu/batchjob
	cp -f $(ICE)/batchjob/icebatchjobpluginargs.h ppu/batchjob
	cp -f $(ICE)/batchjob/icecommandstream.h ppu/batchjob
	cp -f $(ICE)/batchjob/icelocalmemory.h ppu/batchjob
	cp -f $(ICE)/batchjob/icememorybuffer.h ppu/batchjob
	cp -f $(ICE)/batchjob/iceworkbuffer.h ppu/batchjob

	# Anim
	mkdir -p ppu/anim
	cp -f $(ICE)/anim/iceanimbatchmgr.h ppu/anim
	cp -f $(ICE)/anim/iceanimdebug.h ppu/anim
	cp -f $(ICE)/anim/iceanimstack.h ppu/anim
	cp -f $(ICE)/anim/iceanimtaskman.h ppu/anim
	cp -f $(ICE)/anim/iceanimutil.h ppu/anim
	cp -f $(ICE)/anim/iceanimstructs.h ppu/anim
	cp -f $(ICE)/anim/icejointanim.h ppu/anim
	cp -f $(ICE)/anim/icejointanimcore.h ppu/anim
	cp -f $(ICE)/anim/icejointtransform.h ppu/anim
	cp -f $(ICE)/anim/iceanimbatchpriv.h ppu/anim
	cp -f $(ICE)/anim/iceanimclippriv.h ppu/anim
	cp -f $(ICE)/anim/icejointhierarchypriv.h ppu/anim

spu:

	# Ice Render
	mkdir -p spu/render
	cp -f $(ICE)/render/*.h  spu/render
	cp -f $(ICE)/render/*.inl  spu/render

	# BatchJob
	mkdir -p spu/batchjob
	cp -f $(ICE)/batchjob/spu/dispatcher.h.spu spu/batchjob
	cp -f $(ICE)/batchjob/spu/dispatcher.inl.spu spu/batchjob
	cp -f $(ICE)/batchjob/icebatchjob.h spu/batchjob
	cp -f $(ICE)/batchjob/icebatchjobbuffers.h spu/batchjob
	cp -f $(ICE)/batchjob/icebatchjobcore.h spu/batchjob
	cp -f $(ICE)/batchjob/icebatchjobdebug.h spu/batchjob
	cp -f $(ICE)/batchjob/icebatchjobdebug.inl spu/batchjob
	cp -f $(ICE)/batchjob/icebatchjobdispatcher.inl spu/batchjob

	# BatchJob Plugin System
	mkdir -p spu/batchjob/plugin
	cp -f $(ICE)/batchjob/plugin/batchjob_plugin.ld spu/batchjob/plugin
	cp -f $(ICE)/batchjob/plugin/batchjob_plugin.mak spu/batchjob/plugin
	cp -f $(ICE)/batchjob/plugin/batchjob_plugincrt0.spu.s spu/batchjob/plugin

	# Anim
	mkdir -p spu/anim
	cp -f $(ICE)/anim/iceanimstructs.h spu/anim
	cp -f $(ICE)/anim/iceanimdebug.h spu/anim
	cp -f $(ICE)/anim/icejointtransform.h spu/anim
	cp -f $(ICE)/anim/icejointanimcore.h spu/anim
	cp -f $(ICE)/anim/iceanimbatchpriv.h spu/anim
	cp -f $(ICE)/anim/iceanimclippriv.h spu/anim
	cp -f $(ICE)/anim/icejointhierarchypriv.h spu/anim

linux:

win32:

