/*
 * Copyright (c) 2003, 2004, 2005 Naughty Dog, Inc. 
 * A Wholly Owned Subsidiary of Sony Computer Entertainment, Inc.
 * Use and distribution without consent strictly prohibited
 */

#include "iceeffects.h"
#include "icefiles.h"
#include "icecg.h"
#include "icefx.h"

using namespace Ice;
using namespace Ice::Graphics;
using namespace Ice::Fx;
using namespace Ice::Cg;
using namespace Ice::Render;

// Helper function
void UploadFragmentProgramMicrocodeToVRAM(Render::FragmentProgram *renderProgram)
{
	U32 size = renderProgram->m_microcodeSize;
	void *vramAddress = Render::AllocateLinearVideoMemory(size);
	renderProgram->m_offsetAndContext = Render::TranslateAddressToOffset(vramAddress) | kFragmentProgramVideoMemory;

	const U32 *source = renderProgram->GetMicrocode() - 1;
	U32 *destin = static_cast<U32 *>(vramAddress) - 1;

	size >>= 2;
	while (size != 0)
	{
		*++destin = *++source;
		size--;
	}
}

Effect *Graphics::LoadEffect(const char *inName)
{
	printf("Loading %s\n", inName);

	File::File file(inName);
	U64 fileSize = file.GetSize();
	U8 *effectBinary = new U8[fileSize];
	file.Read(0, fileSize, effectBinary);

	Effect *effect = (Effect *)effectBinary;

	// For each fragment program in the effect, upload its microcode to VRAM
    for (const Technique *tech = GetFirstTechnique(effect); tech; tech = GetNextTechnique(tech)) {
        for (const Pass *pass = GetFirstPass(tech); pass; pass = GetNextPass(pass)) {
            const Program *program = GetPassFragmentProgram(pass);
            Render::FragmentProgram *renderProgram = program ? GetRenderFragmentProgram(program) : nullptr;
            if (renderProgram) {
				UploadFragmentProgramMicrocodeToVRAM(renderProgram);
            }
        }
    }

	return effect;
}

Render::VertexProgram *Graphics::LoadVertexProgram(const char *inName)
{
	printf("Loading %s\n", inName);

	File::File file(inName);
	U64 fileSize = file.GetSize();
	ICE_ASSERT(fileSize > 0);
	U8 *programBinary = new U8[fileSize];
	file.Read(0, fileSize, programBinary);

	return (Render::VertexProgram *)programBinary;
}

Render::FragmentProgram *Graphics::LoadFragmentProgram(const char *inName)
{
	printf("Loading %s\n", inName);

	File::File file(inName);
	U64 fileSize = file.GetSize();
	ICE_ASSERT(fileSize > 0);
	U8 *programBinary = new U8[fileSize];
	file.Read(0, fileSize, programBinary);

	Render::FragmentProgram *fProgram = (Render::FragmentProgram *)programBinary;

	UploadFragmentProgramMicrocodeToVRAM(fProgram);

	return fProgram;
}

