/*
 * Copyright (c) 2003-2006 Naughty Dog, Inc. 
 * A Wholly Owned Subsidiary of Sony Computer Entertainment, Inc.
 * Use and distribution without consent strictly prohibited
 */

#ifndef ICE_DISASM_H
#define ICE_DISASM_H


#include <iosfwd>

void DisassembleVertexProgram(std::ostream& output, const unsigned int *instr, unsigned long instrCount, unsigned int resultMask, bool hexCode=true);
void DisassembleFragmentProgram(std::ostream& output, const unsigned int *instr, unsigned long instrCount, bool swap = false, bool hexCode=true);
void DumpCommandBuffer(std::ostream& output, const void *buffer, unsigned long size, const unsigned int *remap = 0);


#endif
