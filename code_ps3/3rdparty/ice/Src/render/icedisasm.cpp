/*
 * Copyright (c) 2003-2006 Naughty Dog, Inc. 
 * A Wholly Owned Subsidiary of Sony Computer Entertainment, Inc.
 * Use and distribution without consent strictly prohibited
 */

#include <fstream>
#include "icedisasm.h"
using namespace std;

#define VERBOSE_DRAWCALLS 0 // Display more in-depth information

#ifndef DISASMPC
#define DISASMPC 0
#endif

#if VERBOSE_DRAWCALLS
#include "icerender.h"
#endif

#if VERBOSE_DRAWCALLS // this function is only used by the in-depth draw call dump
static float F16toF32(signed short i)
{
	// Extract the sign
	int s = (i >> 15) & 0x00000001;
	// Extract the exponent
	int e = (i >> 10) & 0x0000001f;
	// Extract the mantissa
	int m =  i        & 0x000003ff;

	if (e == 0)
	{
		if (m == 0)
		{
			s <<= 31;
			return *(float*)&s;
		}
		else
		{
			// Denormalized number -- renormalize it
			while (!(m & 0x00000400))
			{
				m <<= 1;
				e -=  1;
			}

			e += 1;
			m &= ~0x00000400;
		}
	}
	else if (e == 31)
	{
		if (m == 0)
		{
			// Positive or negative infinity
			s = (s << 31) | 0x7f800000;
			return *(float*)&s;
		}
		else
		{
			// NAN -- preserve sign and mantissa bits
			s = (s << 31) | 0x7f800000 | (m << 13);
			return *(float*)&s;
		}
	}

	// Normalized number
	e = e + (127 - 15);
	m = m << 13;

	// Assemble s, e and m.
	signed int intval = (s << 31) | (e << 23) | m;
	return *(float*)&intval;
}
#endif // VERBOSE_DRAWCALLS

static const char hexdigit[16] = {'0', '1', '2', '3', '4', '5', '6', '7', '8', '9', 'A', 'B', 'C', 'D', 'E', 'F'};


namespace
{
	enum Subchannel
	{
		kSubchannel3D,
		kSubchannelTransfer,
		kSubchannelSwizzle,
		kSubchannelSurface,
		kSubchannelClip,
		kSubchannelBlit,
		kSubchannelStretchBlit,
		kSubchannelRop,
		kSubchannelPattern,
		kSubchannelSync,
		kSubchannelLine,
		kSubchannelRect,
		kSubchannelUnknown
	};
	
	
	const char *subchannelName[] =
	{
		"3D",
		"Transfer",
		"Swizzle",
		"Surface",
		"Clip",
		"Blit",
		"Stretch Blit",
		"Rop",
		"Pattern",
		"Sync",
		"Line",
		"Rect",
		"Unknown"
	};
	
	
	struct FragmentDisassemData
	{
		bool			swapHalves;
		unsigned int	loopInstrIndex;
		unsigned int	repInstrIndex;
		unsigned int	elseInstrIndex;
		unsigned int	endifInstrIndex;
	};
	
	
	Subchannel		currentSubchannel[8];
}


static long GetTextLength(const char *text)
{
	const char *c = text;
	while (*c != 0) c++;
	return ((long) (c - text));
}

static void WriteHexWord(ostream& output, unsigned int word)
{
	output << hexdigit[word >> 28];
	output << hexdigit[(word >> 24) & 0x0F];
	output << hexdigit[(word >> 20) & 0x0F];
	output << hexdigit[(word >> 16) & 0x0F];
	output << hexdigit[(word >> 12) & 0x0F];
	output << hexdigit[(word >> 8) & 0x0F];
	output << hexdigit[(word >> 4) & 0x0F];
	output << hexdigit[word & 0x0F];
}

static void WriteHexHalf(ostream& output, unsigned int word)
{
	output << hexdigit[(word >> 12) & 0x0F];
	output << hexdigit[(word >> 8) & 0x0F];
	output << hexdigit[(word >> 4) & 0x0F];
	output << hexdigit[word & 0x0F];
}

#if 0 // Not used currently.
static void WriteHexByte(ostream& output, unsigned int word)
{
	output << hexdigit[(word >> 4) & 0x0F];
	output << hexdigit[word & 0x0F];
}
#endif


static unsigned long WriteVertexVectorOpcode(ostream& output, unsigned long opcode, long *len)
{
	static const char *instr[26] =
	{
		"NOP", "MOV", "MUL", "ADD", "MAD", "DP3", "DPH", "DP4", "DST", "MIN", "MAX", "SLT", "SGE", "ARL", "FRC", "FLR", "SEQ", "SFL", "SGT", "SLE", "SNE",
		"STR", "SSG", "ARR", "ARA", "TXL"
	};
	
	static const unsigned short oper[26] =
	{
		0x0000, 0x1100, 0x1110, 0x1101, 0x1111, 0x1110, 0x1110, 0x1110, 0x1110, 0x1110, 0x1110, 0x1110, 0x1110, 0x1100, 0x1100, 0x1100, 0x1110, 0x1110, 0x1110, 0x1110, 0x1110,
		0x1110, 0x1100, 0x1100, 0x1100, 0x1100
	};
	
	if (opcode < 26)
	{
		output << instr[opcode];
		*len = GetTextLength(instr[opcode]);
		return (oper[opcode]);
	}
	
	output << "???";
	*len = 3;
	return (0);
}

static unsigned long WriteVertexScalarOpcode(ostream& output, unsigned long opcode, long *len)
{
	static const char *instr[21] =
	{
		"NOP", "???", "RCP", "RCC", "RSQ", "EXP", "LOG", "LIT", "???", "BRA", "???", "CAL", "RET",
		"LG2", "EX2", "SIN", "COS", "???", "???", "PUSHA", "POPA"
	};
	
	static const unsigned short oper[21] =
	{
		0x0000, 0x0000, 0x1001, 0x1001, 0x1001, 0x1001, 0x1001, 0x1001, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000,
		0x1001, 0x1001, 0x1001, 0x1001, 0x0000, 0x0000, 0x0001, 0x1000
	};
	
	if (opcode < 21)
	{
		output << instr[opcode];
		*len = GetTextLength(instr[opcode]);
		return (oper[opcode]);
	}
	
	output << "???";
	*len = 3;
	return (0);
}

static void WriteVertexMask(ostream& output, unsigned long mask)
{
	if (mask != 0x0F)
	{
		output << ".";
		if (mask & 0x08) output << "x";
		if (mask & 0x04) output << "y";
		if (mask & 0x02) output << "z";
		if (mask & 0x01) output << "w";
	}
}

static void WriteVertexSwizzle(ostream& output, unsigned long swiz)
{
	if (swiz != 0x1B)
	{
		static const char comp[4] = {'x', 'y', 'z', 'w'};
		
		output << ".";
		output << comp[swiz >> 6];
		
		if ((swiz == 0x00) || (swiz == 0x55) || (swiz == 0xAA) || (swiz == 0xFF)) return;
		
		output << comp[(swiz >> 4) & 3];
		output << comp[(swiz >> 2) & 3];
		output << comp[swiz & 3];
	}
}

static void WriteVertexAttributeRegister(ostream& output, unsigned long reg, bool bracket = true)
{
	static const char *attributeName[16] =
	{
		"OPOS", "WGHT", "NRML", "COL0", "COL1", "FOGC", "6", "7",
		"TEX0", "TEX1", "TEX2", "TEX3", "TEX4", "TEX5", "TEX6", "TEX7"
	};
	
	if (bracket) output << "v[";
	output << attributeName[reg];
	if (bracket) output << "]";
}

static void WriteVertexOperand(ostream& output, const unsigned int *instr, long num, bool scalar)
{
	static const char comp[4] = {'x', 'y', 'z', 'w'};
	
	unsigned long	reg;
	unsigned long	src;
	unsigned long	swz;
	unsigned long	neg;
	unsigned long	abs;
	
	switch (num)
	{
		case 1:
			
			reg = (instr[2] >> 25) & 0x3F;
			src = (instr[2] >> 23) & 0x03;
			swz = ((instr[1] << 1) | (instr[2] >> 31)) & 0xFF;
			neg = (instr[1] >> 7) & 0x01;
			abs = (instr[0] >> 21) & 0x01;
			break;
		
		case 2:
			
			reg = (instr[2] >> 8) & 0x3F;
			src = (instr[2] >> 6) & 0x03;
			swz = (instr[2] >> 14) & 0xFF;
			neg = (instr[2] >> 22) & 0x01;
			abs = (instr[0] >> 22) & 0x01;
			break;
		
		default:
			
			reg = (instr[3] >> 23) & 0x3F;
			src = (instr[3] >> 21) & 0x03;
			swz = ((instr[2] << 3) | (instr[3] >> 29)) & 0xFF;
			neg = (instr[2] >> 5) & 0x01;
			abs = (instr[0] >> 23) & 0x01;
			break;
	}
	
	if (neg) output << "-";
	if (abs) output << "|";
	
	unsigned long vectorOpcode = (instr[1] >> 22) & 0x1F;
	unsigned long scalarOpcode = (instr[1] >> 27) & 0x1F;
	if ((vectorOpcode == 0x18) || ((scalar) && (scalarOpcode == 0x13)))
	{
		output << "A" << reg;
	}
	else
	{
		if (src == 2)
		{
			output << "v[";
			
			if (instr[0] & 0x08000000)
			{
				if (instr[0] & 0x01000000) output << "A1.";
				else output << "A0.";
				output << comp[instr[0] & 3] << "+";
				output << ((instr[1] >> 8) & 0x0F);
			}
			else
			{
				WriteVertexAttributeRegister(output, (instr[1] >> 8) & 0x0F, false);
			}
			
			output << "]";
		}
		else if (src == 3)
		{
			output << "c[";
			
			if (instr[3] & 2)
			{
				if (instr[0] & 0x01000000) output << "A1.";
				else output << "A0.";
				
				output << comp[instr[0] & 3] << "+";
			}
			
			output << ((instr[1] >> 12) & 0x01FF) << "]";
		}
		else
		{
			output << "R" << reg;
		}
	}
	
	WriteVertexSwizzle(output, swz);
	
	if (abs) output << "|";
}

static void DisassembleVertexInstruction(ostream& output, const unsigned int *instr, unsigned int resultMask, bool hexcode, bool html = false)
{
	bool nop = true;
	for (long a = 0; a < 2; a++)
	{
		unsigned long	opcode;
		long			charCount;
		
		if ((a == 0) && (hexcode))
		{
			WriteHexWord(output, instr[0]);
			output << " ";
			WriteHexWord(output, instr[1]);
			output << " ";
			WriteHexWord(output, instr[2]);
			output << " ";
			WriteHexWord(output, instr[3]);
			output << "    ";
		}
		
		bool addrDest = false;
		bool branch = false;
		unsigned long operandMask = 0;
		
		if (a == 0)
		{
			opcode = (instr[1] >> 22) & 0x1F;
			if (opcode != 0)
			{
				if ((opcode == 13) || (opcode == 23) || (opcode == 24)) addrDest = true;
				operandMask = WriteVertexVectorOpcode(output, opcode, &charCount);
			}
		}
		else
		{
			opcode = (instr[1] >> 27) & 0x1F;
			if ((opcode != 0) || (nop))
			{
				if (!nop)
				{
					if (html) output << "<BR>";
					else output << endl;
					if (hexcode) output << "                                       ";
				}
				
				if ((opcode == 19) || (opcode == 20)) addrDest = true;
				if ((opcode == 9) || (opcode == 11) || (opcode == 12)) branch = true;
				operandMask = WriteVertexScalarOpcode(output, opcode, &charCount);
				nop = false;
			}
			else if (opcode == 0)
			{
				nop = true;
			}
		}
		
		if (opcode != 0)
		{
			if (instr[0] & 0x20000000)
			{
				if (instr[0] & 0x02000000)
				{
					output << "C1";
					charCount += 2;
				}
				else
				{
					output << "C";
					charCount++;
				}
			}
			
			if (instr[0] & 0x04000000)
			{
				output << "_SAT";
				charCount += 4;
			}
			
			if (html) for (long k = charCount; k < 14; k++) output << "&nbsp;";
			else for (long k = charCount; k < 14; k++) output << " ";
			
			if (operandMask & 0x1000)
			{
				bool writeMask = ((a == 0) || (opcode != 0x14));
				unsigned long mask = ((a == 0) ? (instr[3] >> 13) : (instr[3] >> 17)) & 0x0F;
				
				unsigned long resultReg = (a == 0) ? (instr[0] & 0x40000000) : (instr[3] & 0x00001000);
				if (resultReg != 0)
				{
					static const char *result[32] =
					{
						"HPOS", "COL0", "COL1", "BFC0", "BFC1", "0x05", "0x06", "TEX0", "TEX1",
						"TEX2", "TEX3", "TEX4", "TEX5", "TEX6", "TEX7", "TEX8", "0x10", "0x11",
						"0x12", "0x13", "0x14", "0x15", "0x16", "0x17", "0x18", "0x19", "0x1A",
						"0x1B", "0x1C", "0x1D", "0x1E", "0x1F"
					};
					
					output << "o[";
					
					if (instr[0] & 0x10000000)
					{
						static const char comp[4] = {'x', 'y', 'z', 'w'};
						
						if (instr[0] & 0x01000000) output << "A1.";
						else output << "A0.";
						output << comp[instr[0] & 3] << "+";
						output << (long) ((instr[3] >> 2) & 0x1F);
					}
					else
					{
						unsigned long reg = (instr[3] >> 2) & 0x1F;
						if (reg == 5)
						{
							switch (mask) 
							{
								case 8:
									output << "FOGC";
									writeMask = false;
									break;
								case 4:
									output << "CLP0";
									writeMask = false;
									break;
								case 2:
									output << "CLP1";
									writeMask = false;
									break;
								case 1:
									output << "CLP2";
									writeMask = false;
									break;
								default:
									output << "FOGC|CLP0|CLP1|CLP2";
									break;
							}
						}
						else if (reg == 6) 
						{
							if (resultMask & 0x00002000) 
							{
								output << "TEX9";
							}
							else
							{
								switch (mask) 
								{
									case 8:
										output << "PSIZ";
										writeMask = false;
										break;
									case 4:
										output << "CLP3";
										writeMask = false;
										break;
									case 2:
										output << "CLP4";
										writeMask = false;
										break;
									case 1:
										output << "CLP5";
										writeMask = false;
										break;
									default:
										output << "PSIZ|CLP3|CLP4|CLP5";
										break;
								}
							}
						}
						else
						{
							output << result[reg];
						}
					}
					
					output << "]";
				}
				else
				{
					unsigned long index = (a == 0) ? ((instr[0] >> 15) & 0x3F) : ((instr[3] >> 7) & 0x1F);
					output << (addrDest ? "A" : "R");
					output << index;
				}
				
				if (writeMask) WriteVertexMask(output, mask);
			}
			
			if ((branch) && (opcode != 12))
			{
				output << "0x";
				WriteHexHalf(output, ((instr[2] << 3) | (instr[3] >> 29)) & 0x01FF);
			}
			
			if ((instr[0] & 0x00002000) || (branch))
			{
				static const char *code[8] =
				{
					"FL", "LT", "EQ", "LE", "GT", "NE", "GE", "TR"
				};
				
				if ((!branch) || (opcode != 12)) output << " ";
				output << "(" << code[(instr[0] >> 10) & 7];
				if (instr[0] & 0x00004000) output << "1";
				WriteVertexSwizzle(output, (instr[0] >> 2) & 0xFF);
				output << ")";
			}
			
			if (a == 0)
			{
				if (operandMask & 0x100)
				{
					output << ", ";
					WriteVertexOperand(output, instr, 1, false);
				}
				
				if (operandMask & 0x010)
				{
					output << ", ";
					WriteVertexOperand(output, instr, 2, false);
				}
				else if (opcode == 0x19)
				{
					output << ", TEX";
					output << ((instr[2] >> 8) & 0x3F);
				}
			}
			
			if (operandMask & 0x001)
			{
				if (operandMask & 0x1000) output << ", ";
				WriteVertexOperand(output, instr, 3, (a == 1));
			}
			
			output << ";";
			nop = false;
		}
	}
	
	if (!html) output << endl;
}

void DisassembleVertexProgram(ostream& output, const unsigned int *instr, unsigned long instrCount, unsigned int resultMask, bool hexCode)
{
	for (unsigned long i = 0; i < instrCount; i++)
	{
		DisassembleVertexInstruction(output, instr, resultMask, hexCode);
		instr += 4;
	}
}

static unsigned long WriteFragmentOpcode(ostream& output, unsigned long opcode, bool flow, long *len)
{
	static const char *instr[0x40] =
	{
		"NOP", "MOV", "MUL", "ADD", "MAD", "DP3", "DP4", "DST", "MIN", "MAX", "SLT", "SGE", "SLE", "SGT", "SNE", "SEQ",
		"FRC", "FLR", "KIL", "PK4B", "UP4B", "DDX", "DDY", "TEX", "TXP", "TXD", "RCP", "RSQ", "EX2", "LG2", "LIT", "LRP",
		"STR", "SFL", "COS", "SIN", "PK2H", "UP2H", "POW", "PK4UB", "UP4UB", "PK2US", "UP2US", "BEM", "PKG", "UPG", "DP2A", "TXL",
		"0x30", "TXB", "0x32", "TEXBEM", "TXPBEM", "BEMLUM", "REFL", "XWTEX", "DP2", "NRM", "DIV", "DIVSQ", "LIF", "FENCT", "FENCB", "0x3F"
	};
	
	static const char *flowInstr[6] =
	{
		"BRK", "CAL", "IF", "LOOP", "REP", "RET"
	};
	
	static const unsigned short oper[0x40] =
	{
		0x0000, 0x1100, 0x1110, 0x1110, 0x1111, 0x1110, 0x1110, 0x1110, 0x1110, 0x1110, 0x1110, 0x1110, 0x1110, 0x1110, 0x1110, 0x1110,
		0x1100, 0x1100, 0x0000, 0x1100, 0x1100, 0x1100, 0x1100, 0x1100, 0x1100, 0x1111, 0x1100, 0x1100, 0x1100, 0x1100, 0x1100, 0x1111,
		0x1110, 0x0000, 0x1100, 0x1100, 0x1100, 0x1100, 0x1110, 0x1100, 0x1100, 0x1100, 0x1100, 0x1111, 0x1100, 0x1100, 0x1111, 0x1100,
		0x0000, 0x1100, 0x0000, 0x1111, 0x1111, 0x1111, 0x1110, 0x1100, 0x1110, 0x1100, 0x1110, 0x1110, 0x1100, 0x0000, 0x0000, 0x0000
	};
	
	if ((flow) && (opcode < 6))
	{
		output << flowInstr[opcode];
		*len = GetTextLength(flowInstr[opcode]);
		return (0);
	}
	
	if (opcode < 0x40)
	{
		output << instr[opcode];
		*len = GetTextLength(instr[opcode]);
		return (oper[opcode]);
	}
	
	output << "???";
	*len = 3;
	return (0);
}

static void WriteFragmentMask(ostream& output, unsigned long mask)
{
	if (mask != 0x0F)
	{
		output << ".";
		if (mask & 0x01) output << "x";
		if (mask & 0x02) output << "y";
		if (mask & 0x04) output << "z";
		if (mask & 0x08) output << "w";
	}
}

static void WriteFragmentSwizzle(ostream& output, unsigned long swiz)
{
	if (swiz != 0xE4)
	{
		static const char comp[4] = {'x', 'y', 'z', 'w'};
		
		output << ".";
		output << comp[swiz & 3];
		
		if ((swiz == 0x00) || (swiz == 0x55) || (swiz == 0xAA) || (swiz == 0xFF)) return;
		
		output << comp[(swiz >> 2) & 3];
		output << comp[(swiz >> 4) & 3];
		output << comp[(swiz >> 6) & 3];
	}
}

static void WriteFragmentAttributeRegister(ostream& output, unsigned long reg, bool bracket = true)
{
	static const char *attributeName[16] =
	{
		"WPOS", "COL0", "COL1", "FOG", "TEX0", "TEX1", "TEX2", "TEX3",
		"TEX4", "TEX5", "TEX6", "TEX7", "TEX8", "TEX9", "FACE", "EYE"
	};
	
	if (bracket) output << "f[";
	output << attributeName[reg];
	if (bracket) output << "]";
}

static void WriteFragmentOperand(ostream& output, const unsigned int *instr, long num, int constIndex)
{
	unsigned long	reg;
	unsigned long	src;
	unsigned long	swz;
	unsigned long	neg;
	unsigned long	abs;
	unsigned long	clamp;
	
	switch (num)
	{
		case 1:
			
			reg = (instr[1] >> 2) & 0x7F;
			src = instr[1] & 0x03;
			swz = (instr[1] >> 9) & 0xFF;
			neg = (instr[1] >> 17) & 0x01;
			abs = (instr[1] >> 29) & 0x01;
			clamp = (instr[2] >> 19) & 0x07;
			break;
		
		case 2:
			
			reg = (instr[2] >> 2) & 0x7F;
			src = instr[2] & 0x03;
			swz = (instr[2] >> 9) & 0xFF;
			neg = (instr[2] >> 17) & 0x01;
			abs = (instr[2] >> 18) & 0x01;
			clamp = (instr[2] >> 22) & 0x07;
			break;
		
		default: // 3
			
			reg = (instr[3] >> 2) & 0x7F;
			src = instr[3] & 0x03;
			swz = (instr[3] >> 9) & 0xFF;
			neg = (instr[3] >> 17) & 0x01;
			abs = (instr[3] >> 18) & 0x01;
			clamp = (instr[2] >> 25) & 0x07;
			break;
	}
	
	if (neg) output << "-";
	if (abs) output << "|";
	
	if (src == 1)
	{
		if (instr[3] & 0x80000000) output << "g[";
		else output << "f[";
		
		if (instr[3] & 0x40000000)
		{
			output << "TEX";
			output << ((instr[3] >> 19) & 0x0F);
			output << "+A0.x";
		}
		else
		{
			WriteFragmentAttributeRegister(output, (instr[0] >> 13) & 0x0F, false);
		}
		
		output << "]";
	}
	else if (src == 2)
	{
		if ( constIndex==-1 )
			output << "const";
		else
			output << "c[" << constIndex << "]";
	}
	else
	{
		output << (((reg & 0x40) != 0) ? "H" : "R");
		output << (reg & 0x3F);
	}
	
	if (clamp != 0) switch (clamp)
	{
		case 1:
			output << "_H";
			break;
		case 2:
			output << "_X";
			break;
		case 3:
			output << "_B";
			break;
		case 4:
			output << "_S";
			break;
		case 5:
			output << "_N";
			break;
		default:
			output << "_" << clamp;
			break;
	}
	
	WriteFragmentSwizzle(output, swz);
	
	if (abs) output << "|";
}

static long DisassembleFragmentInstruction(ostream& output, const unsigned int *instr, FragmentDisassemData *disassemData, bool hexCode=true, int constIndex=-1 )
{
	long	charCount;
	
	unsigned int *instr2 = const_cast<unsigned int *>(instr);
	if (disassemData->swapHalves)
	{
		instr2[0] = (instr2[0] << 16) | (instr2[0] >> 16);
		instr2[1] = (instr2[1] << 16) | (instr2[1] >> 16);
		instr2[2] = (instr2[2] << 16) | (instr2[2] >> 16);
		instr2[3] = (instr2[3] << 16) | (instr2[3] >> 16);
	}

	if ( hexCode )
	{
		WriteHexWord(output, instr[0]);
		output << " ";
		WriteHexWord(output, instr[1]);
		output << " ";
		WriteHexWord(output, instr[2]);
		output << " ";
		WriteHexWord(output, instr[3]);
		output << "    ";
	}
	
	unsigned long opcode = (instr[0] >> 24) & 0x3F;
	
	bool flow = ((instr[2] & 0x80000000) != 0);
	unsigned long operandMask = WriteFragmentOpcode(output, opcode, flow, &charCount);
	
	if ((opcode != 0) || (flow))
	{
		if (instr[0] & 0x00400000)
		{
			output << "H";
			charCount++;
		}
		
		if (instr[0] & 0x00000100)
		{
			output << "C";
			charCount++;
		}
		
		if (instr[0] & 0x00200000)
		{
			output << "_BX2";
			charCount += 4;
		}
		
		if (instr[2] & 0x70000000)
		{
			static const char *modifier[8] =
			{
				"   ", "_X2", "_X4", "_X8", "_??", "_D2", "_D4", "_D8"
			};
			
			output << modifier[(instr[2] >> 28) & 7];
			charCount += 3;
		}
		
		if (instr[0] & 0x80000000)
		{
			output << "_SAT";
			charCount += 4;
		}
		else if (instr[0] & 0x00800000)
		{
			output << "_SSAT";
			charCount += 5;
		}
		
		for (long k = charCount; k < 14; k++) output << " ";
		
		bool dest = false;
		if (operandMask & 0x1000)
		{
			output << (((instr[0] & 0x00000080) != 0) ? "H" : "R");
			if (instr[0] & 0x40000000) output << "C";
			else output << ((instr[0] >> 1) & 0x3F);
			
			WriteFragmentMask(output, (instr[0] >> 9) & 0x0F);
			dest = true;
		}
		
		unsigned long condition = (instr[1] >> 18) & 7;
		if ((condition != 7) || ((flow) && (opcode != 3) && (opcode != 4)))
		{
			static const char *code[8] =
			{
				"FL", "LT", "EQ", "LE", "GT", "NE", "GE", "TR"
			};
			
			if (dest) output << " ";
			output << "(" << code[condition];
			WriteFragmentSwizzle(output, (instr[1] >> 21) & 0xFF);
			output << ")";
		}
		
		if (operandMask & 0x100)
		{
			output << ", ";
			WriteFragmentOperand(output, instr, 1, constIndex);
		}
		
		if (operandMask & 0x010)
		{
			output << ", ";
			WriteFragmentOperand(output, instr, 2, constIndex);
		}
		
		if (operandMask & 0x001)
		{
			output << ", ";
			WriteFragmentOperand(output, instr, 3, constIndex);
		}
		
		if ((opcode == 0x17) || (opcode == 0x18) || (opcode == 0x19) || (opcode == 0x2F) || (opcode == 0x31) || (opcode == 0x33) || (opcode == 0x34) || (opcode == 0x37))
		{
			output << ", TEX";
			output << ((instr[0] >> 17) & 0x0F);
		}
		else if (flow)
		{
			if (opcode == 1)
			{
				output << ", 0x";
				WriteHexWord(output, (instr[2] >> 2) & 0x1FFFFFFF);
			}
			else if (opcode == 2)
			{
				int elseIndex = (instr[2] >> 2) & 0x1FFFFFFF;
				int endIndex = (instr[3] >> 2) & 0x1FFFFFFF;
				disassemData->elseInstrIndex = elseIndex;
				disassemData->endifInstrIndex = endIndex;
				
				output << ";   # 0x";
				WriteHexWord(output, elseIndex);
				output << ", 0x";
				WriteHexWord(output, endIndex);
			}
			else if (opcode == 3)
			{
				output << "{" << ((instr[2] >> 2) & 0xFF);
				output << ", " << ((instr[2] >> 10) & 0xFF);
				output << ", " << ((instr[2] >> 19) & 0xFF) << "}";
				
				int endIndex = (instr[3] >> 2) & 0x1FFFFFFF;
				disassemData->loopInstrIndex = endIndex;
				
				output << ";   # 0x";
				WriteHexWord(output, endIndex);
			}
			else if (opcode == 4)
			{
				output << ((instr[2] >> 2) & 0xFF);
				
				int endIndex = (instr[3] >> 2) & 0x1FFFFFFF;
				disassemData->repInstrIndex = endIndex;
				
				output << ";   # 0x";
				WriteHexWord(output, endIndex);
			}
		}
		
		output << ";";
	}

	long k = 1;
	if ((instr[1] & 2) || (instr[2] & 2) || (instr[3] & 2))
	{
		if (disassemData->swapHalves)
		{
			instr2[4] = (instr2[4] << 16) | (instr2[4] >> 16);
			instr2[5] = (instr2[5] << 16) | (instr2[5] >> 16);
			instr2[6] = (instr2[6] << 16) | (instr2[6] >> 16);
			instr2[7] = (instr2[7] << 16) | (instr2[7] >> 16);
		}

		if ( hexCode )
		{
			output << endl;
			WriteHexWord(output, instr[4]);
			output << " ";
			WriteHexWord(output, instr[5]);
			output << " ";
			WriteHexWord(output, instr[6]);
			output << " ";
			WriteHexWord(output, instr[7]);
		}
		else
		{
			// make a floating point constant
			F32 * fptr = (F32 *) (instr + 4);

			output << "\t\t";
			output << "# Const[";
			output << constIndex;
			output << "] = ( ";
			output << fptr[0];
			output << ", ";
			output << fptr[1];
			output << ", ";
			output << fptr[2];
			output << ", ";
			output << fptr[3];
			output << " )";
		}

		if (disassemData->swapHalves)
		{
			instr2[4] = (instr2[4] << 16) | (instr2[4] >> 16);
			instr2[5] = (instr2[5] << 16) | (instr2[5] >> 16);
			instr2[6] = (instr2[6] << 16) | (instr2[6] >> 16);
			instr2[7] = (instr2[7] << 16) | (instr2[7] >> 16);
		}

		k = 2;
	}

	if (disassemData->swapHalves)
	{
		instr2[0] = (instr2[0] << 16) | (instr2[0] >> 16);
		instr2[1] = (instr2[1] << 16) | (instr2[1] >> 16);
		instr2[2] = (instr2[2] << 16) | (instr2[2] >> 16);
		instr2[3] = (instr2[3] << 16) | (instr2[3] >> 16);
	}

	output << endl;

	return (k);
}

void DisassembleFragmentProgram(ostream& output, const unsigned int *instr, unsigned long instrCount, bool swap, bool hexCode )
{
	FragmentDisassemData	data;
	
	data.swapHalves = swap;
	data.loopInstrIndex = 0xFFFFFFFF;
	data.repInstrIndex = 0xFFFFFFFF;
	data.elseInstrIndex = 0xFFFFFFFF;
	data.endifInstrIndex = 0xFFFFFFFF;

	int constIndex = 0;
	
	for (unsigned long i = 0; i < instrCount;)
	{
		long count = DisassembleFragmentInstruction(output, instr, &data, hexCode, constIndex);
		instr += count * 4;
		i += count;

		constIndex += (count==2);
		
		if (data.loopInstrIndex == i) output << "                                       ENDLOOP;" << endl;
		if (data.repInstrIndex == i) output << "                                       ENDREP;" << endl;
		if (data.endifInstrIndex == i) output << "                                       ENDIF;" << endl;
		else if (data.elseInstrIndex == i) output << "                                       ELSE;" << endl;
	}
}

static void WriteCompareFunction(ostream& output, unsigned int data)
{
	output << "<CODE>";
	
	switch (data)
	{
		case 0x0200:
			output << "NEVER";
			break;
		case 0x0201:
			output << "LESS";
			break;
		case 0x0202:
			output << "EQUAL";
			break;
		case 0x0203:
			output << "LEQUAL";
			break;
		case 0x0204:
			output << "GREATER";
			break;
		case 0x0205:
			output << "NOTEQUAL";
			break;
		case 0x0206:
			output << "GEQUAL";
			break;
		case 0x0207:
			output << "ALWAYS";
			break;
	}
	
	output << "</CODE>";
}

static void WriteBlendFactor(ostream& output, unsigned int data)
{
	output << "<CODE>";
	
	switch (data)
	{
		case 0x0000:
			output << "ZERO";
			break;
		case 0x0001:
			output << "ONE";
			break;
		case 0x0300:
			output << "SRC_COLOR";
			break;
		case 0x0301:
			output << "ONE_MINUS_SRC_COLOR";
			break;
		case 0x0302:
			output << "SRC_ALPHA";
			break;
		case 0x0303:
			output << "ONE_MINUS_SRC_ALPHA";
			break;
		case 0x0304:
			output << "DST_ALPHA";
			break;
		case 0x0305:
			output << "ONE_MINUS_DST_ALPHA";
			break;
		case 0x0306:
			output << "DST_COLOR";
			break;
		case 0x0307:
			output << "ONE_MINUS_DST_COLOR";
			break;
		case 0x0308:
			output << "SRC_ALPHA_SATURATE";
			break;
		case 0x8001:
			output << "CONSTANT_COLOR";
			break;
		case 0x8002:
			output << "ONE_MINUS_CONSTANT_COLOR";
			break;
		case 0x8003:
			output << "CONSTANT_ALPHA";
			break;
		case 0x8004:
			output << "ONE_MINUS_CONSTANT_ALPHA";
			break;
	}
	
	output << "</CODE>";
}

static void WriteBlendEquation(ostream& output, unsigned int data)
{
	output << "<CODE>";
	
	switch (data)
	{
		case 0x8006:
			output << "FUNC_ADD";
			break;
		case 0x8007:
			output << "MIN";
			break;
		case 0x8008:
			output << "MAX";
			break;
		case 0x800A:
			output << "FUNC_SUBTRACT";
			break;
		case 0x800B:
			output << "FUNC_REVERSE_SUBTRACT";
			break;
	}
	
	output << "</CODE>";
}

static void WriteStencilOperation(ostream& output, unsigned int data)
{
	output << "<CODE>";
	
	switch (data)
	{
		case 0x0000:
			output << "ZERO";
			break;
		case 0x1E00:
			output << "KEEP";
			break;
		case 0x1E01:
			output << "REPLACE";
			break;
		case 0x1E02:
			output << "INCR";
			break;
		case 0x1E03:
			output << "DECR";
			break;
		case 0x150A:
			output << "INVERT";
			break;
		case 0x8507:
			output << "INCR_WRAP";
			break;
		case 0x8508:
			output << "DECR_WRAP";
			break;
	}
	
	output << "</CODE>";
}

static void WriteShadingModel(ostream& output, unsigned int data)
{
	output << "<CODE>";
	
	switch (data)
	{
		case 0x1D00:
			output << "FLAT";
			break;
		case 0x1D01:
			output << "SMOOTH";
			break;
	}
	
	output << "</CODE>";
}

static void WriteLogicOperation(ostream& output, unsigned int data)
{
	output << "<CODE>";
	
	switch (data)
	{
		case 0x1500:
			output << "CLEAR";
			break;
		case 0x1501:
			output << "AND";
			break;
		case 0x1502:
			output << "AND_REVERSE";
			break;
		case 0x1503:
			output << "COPY";
			break;
		case 0x1504:
			output << "AND_INVERTED";
			break;
		case 0x1505:
			output << "NOOP";
			break;
		case 0x1506:
			output << "XOR";
			break;
		case 0x1507:
			output << "OR";
			break;
		case 0x1508:
			output << "NOR";
			break;
		case 0x1509:
			output << "EQUIV";
			break;
		case 0x150A:
			output << "INVERT";
			break;
		case 0x150B:
			output << "OR_REVERSE";
			break;
		case 0x150C:
			output << "COPY_INVERTED";
			break;
		case 0x150D:
			output << "OR_INVERTED";
			break;
		case 0x150E:
			output << "NAND";
			break;
		case 0x150F:
			output << "SET";
			break;
	}
	
	output << "</CODE>";
}

static void WriteFogMode(ostream& output, unsigned int data)
{
	output << "<CODE>";
	
	switch (data)
	{
		case 0x0800:
			output << "EXP";
			break;
		case 0x0801:
			output << "EXP2";
			break;
		case 0x2601:
			output << "LINEAR";
			break;
	}
	
	output << "</CODE>";
}

static void WriteDrawMode(ostream& output, unsigned int data)
{
	output << "<CODE>";
	
	switch (data)
	{
		case 0x0000:
			output << "NONE";
			break;
		case 0x0001:
			output << "POINTS";
			break;
		case 0x0002:
			output << "LINES";
			break;
		case 0x0003:
			output << "LINE_LOOP";
			break;
		case 0x0004:
			output << "LINE_STRIP";
			break;
		case 0x0005:
			output << "TRIANGLES";
			break;
		case 0x0006:
			output << "TRIANGLE_STRIP";
			break;
		case 0x0007:
			output << "TRIANGLE_FAN";
			break;
		case 0x0008:
			output << "QUADS";
			break;
		case 0x0009:
			output << "QUAD_STRIP";
			break;
		case 0x000A:
			output << "POLYGON";
			break;
	}
	
	output << "</CODE>";
}

static void WritePolygonMode(ostream& output, unsigned int data)
{
	output << "<CODE>";
	
	switch (data)
	{
		case 0x1B00:
			output << "POINT";
			break;
		case 0x1B01:
			output << "LINE";
			break;
		case 0x1B02:
			output << "FILL";
			break;
	}
	
	output << "</CODE>";
}

static void WriteCullFace(ostream& output, unsigned int data)
{
	output << "<CODE>";
	
	switch (data)
	{
		case 0x0404:
			output << "FRONT";
			break;
		case 0x0405:
			output << "BACK";
			break;
		case 0x0408:
			output << "FRONT_AND_BACK";
			break;
	}
	
	output << "</CODE>";
}

static void WriteFrontFace(ostream& output, unsigned int data)
{
	output << "<CODE>";
	
	switch (data)
	{
		case 0x0900:
			output << "CW";
			break;
		case 0x0901:
			output << "CCW";
			break;
	}
	
	output << "</CODE>";
}

static void WriteClearChannels(ostream& output, unsigned int data)
{
	output << "<CODE>";
	bool comma = false;
	
	if (data & 0x10)
	{
		output << "R";
		comma = true;
	}
	
	if (data & 0x20)
	{
		if (comma) output << "</CODE>, <CODE>";
		output << "G";
		comma = true;
	}
	
	if (data & 0x40)
	{
		if (comma) output << "</CODE>, <CODE>";
		output << "B";
		comma = true;
	}
	
	if (data & 0x80)
	{
		if (comma) output << "</CODE>, <CODE>";
		output << "A";
		comma = true;
	}
	
	if (data & 0x01)
	{
		if (comma) output << "</CODE>, <CODE>";
		output << "D";
		comma = true;
	}
	
	if (data & 0x02)
	{
		if (comma) output << "</CODE>, <CODE>";
		output << "S";
	}
	
	output << "</CODE>";
}

static void WriteAttributeFormat(ostream& output, unsigned int data)
{
	output << "<CODE>";
	
	if ((data & 0x00F0) != 0)
	{
		switch (data & 0x0F)
		{
			case 0x01:
				output << "SHORT_N";
				break;
			case 0x02:
				output << "FLOAT";
				break;
			case 0x03:
				output << "HALF_FLOAT";
				break;
			case 0x04:
				output << "UNSIGNED_BYTE_N";
				break;
			case 0x05:
				output << "SHORT";
				break;
			case 0x06:
				output << "PACKED 10-11-11";
				break;
			case 0x07:
				output << "UNSIGNED_BYTE";
				break;
		}
		
		output << "</CODE> &times; <CODE>" << ((data >> 4) & 0x0F);
		output << "</CODE>, <CODE>STRIDE = " << (data >> 8);
		
		unsigned int divider = data >> 16;
		if (divider != 0) output << "</CODE>, <CODE>DIVIDER = " << divider;
	}
	else
	{
		output << "DISABLED";
	}
	
	output << "</CODE>";
}

static void WriteVertexAttributes(ostream& output, unsigned int data)
{
	bool space = false;
	for (long a = 0; a < 16; a++)
	{
		if (data & 1)
		{
			if (space) output << " ";
			else space = true;
			
			WriteVertexAttributeRegister(output, a);
		}
		
		data >>= 1;
	}
}

static void WriteVertexResults(ostream& output, unsigned int data)
{
	bool space = false;
	for (long a = 0; a < 22; a++)
	{
		if (data & 1)
		{
			if (space) output << " ";
			else space = true;
			
			static const char *resultName[22] =
			{
				"COL0", "COL1", "BFC0", "BFC1", "FOGC", "PSIZ", "CLP0", "CLP1", "CLP2", "CLP3", "CLP4",
				"CLP5", "TEX8", "TEX9", "TEX0", "TEX1", "TEX2", "TEX3", "TEX4", "TEX5", "TEX6", "TEX7"
			};
			
			output << "o[" << resultName[a] << "]";
		}
		
		data >>= 1;
	}
}

static void WriteTextureFormat(ostream& output, unsigned int data)
{
	output << "<CODE>";
	
	switch ((data >> 8) & 0x1F)
	{
		case 0x01:
			output << "X8";
			break;
		case 0x02:
			output << "R5G5B5A1";
			break;
		case 0x03:
			output << "RGBA4";
			break;
		case 0x04:
			output << "R5G6B5";
			break;
		case 0x05:
			output << "RGBA8";
			break;
		case 0x06:
			output << "DXT1";
			break;
		case 0x07:
			output << "DXT3";
			break;
		case 0x08:
			output << "DXT5";
			break;
		case 0x09:
			output << "SY8";
			break;
		case 0x0A:
			output << "X7SY9";
			break;
		case 0x0B:
			output << "XY8";
			break;
		case 0x0C:
			output << "SG8SB8";
			break;
		case 0x0D:
			output << "B8R8, G8R8";
			break;
		case 0x0E:
			output << "R8B8, R8G8";
			break;
		case 0x0F:
			output << "R6G5B5";
			break;
		case 0x10:
			output << "D24";
			break;
		case 0x11:
			output << "FLOAT_D24";
			break;
		case 0x12:
			output << "D16";
			break;
		case 0x13:
			output << "FLOAT_D16";
			break;
		case 0x14:
			output << "X16";
			break;
		case 0x15:
			output << "XY16";
			break;
		case 0x16:
			output << "A4V6YB6A4U6YA6";
			break;
		case 0x17:
			output << "R5G5B5A1";
			break;
		case 0x18:
			output << "UNSIGNED_HILO8";
			break;
		case 0x19:
			output << "SIGNED_HILO8";
			break;
		case 0x1A:
			output << "FLOAT_RGBA16";
			break;
		case 0x1B:
			output << "FLOAT_RGBA32";
			break;
		case 0x1C:
			output << "FLOAT_X";
			break;
		case 0x1D:
			output << "X1R5G5B5";
			break;
		case 0x1E:
			output << "X8R8G8B8";
			break;
		case 0x1F:
			output << "FLOAT_GR16";
			break;
	}
	
	output << "</CODE>, <CODE>";
	
	switch (data & 0xF4)
	{
		case 0x10:
			output << "1D";
			break;
		case 0x20:
			output << "2D";
			break;
		case 0x30:
			output << "3D";
			break;
		case 0x24:
			output << "CUBE";
			break;
	}
	
	output << "</CODE>, <CODE>LEVELS = " << ((data >> 16) & 0x0F);
	
	if (data & 0x00004000) output << "</CODE>, <CODE>DIRECT";
	if (data & 0x00002000) output << "</CODE>, <CODE>LINEAR";
	
	output << "</CODE>";
}

static void WriteTextureSwizzle(ostream& output, unsigned int data)
{
	static char comp[4] = {'A', 'R', 'G', 'B'};
	
	output << "<CODE>RGBA = ";
	
	unsigned int code = (data >> 10) & 3;
	if (code == 0) output << "0";
	else if (code == 1) output << "1";
	else output << comp[(data >> 2) & 3];
	
	code = (data >> 12) & 3;
	if (code == 0) output << "0";
	else if (code == 1) output << "1";
	else output << comp[(data >> 4) & 3];
	
	code = (data >> 14) & 3;
	if (code == 0) output << "0";
	else if (code == 1) output << "1";
	else output << comp[(data >> 6) & 3];
	
	code = (data >> 8) & 3;
	if (code == 0) output << "0";
	else if (code == 1) output << "1";
	else output << comp[data & 3];
	
	output << "</CODE>";
}

static void WriteTextureControl1(ostream& output, unsigned int data)
{
	output << "<CODE>";
	
	for (long a = 0; a < 3; a++)
	{
		switch ((data >> (a * 8)) & 0x0F)
		{
			case 0x01:
				output << "REPEAT";
				break;
			case 0x02:
				output << "MIRROR_REPEAT";
				break;
			case 0x03:
				output << "CLAMP_TO_EDGE";
				break;
			case 0x04:
				output << "CLAMP_TO_BORDER";
				break;
			case 0x05:
				output << "CLAMP";
				break;
			case 0x06:
				output << "MIRROR_CLAMP_TO_EDGE";
				break;
			case 0x07:
				output << "MIRROR_CLAMP_TO_BORDER";
				break;
			case 0x08:
				output << "MIRROR_CLAMP";
				break;
		}
		
		if (a < 2) output << "</CODE>, <CODE>";
	}
	
	output << "</CODE>";
	
	if (data & 0x00001000) output << "<BR><CODE>NORMAL_EXPAND</CODE>";
}

static void WriteTextureControl2(ostream& output, unsigned int data)
{
	output << "<CODE>MIN_LOD = " << ((data >> 27) & 0x0F);
	output << "<BR>MAX_LOD = " << ((data >> 15) & 0x0F);
	
	unsigned int aniso = (data >> 4) & 0x07;
	if (aniso != 0)
	{
		static const char *anisoText[7] =
		{
			"2", "4", "6", "8", "10", "12", "16"
		};
		
		output << "<BR>MAX_ANISOTROPY = " << anisoText[aniso - 1];
	}
	
	output << "</CODE>";
}

static void WriteTextureFilter(ostream& output, unsigned int data)
{
	output << "<CODE>";
	
	for (long a = 0; a < 2; a++)
	{
		switch ((data >> (a * 8 + 16)) & 0x0F)
		{
			case 0x01:
				output << "NEAREST";
				break;
			case 0x02:
				output << "LINEAR";
				break;
			case 0x03:
				output << "NEAREST_MIPMAP_NEAREST";
				break;
			case 0x04:
				output << "LINEAR_MIPMAP_NEAREST";
				break;
			case 0x05:
				output << "NEAREST_MIPMAP_LINEAR";
				break;
			case 0x06:
				output << "LINEAR_MIPMAP_LINEAR";
				break;
		}
		
		if (a == 0) output << "</CODE>, <CODE>";
	}
	
	int bias = (int) ((data & 0x1FFF) << 19) >> 19;
	if (bias != 0) output << "<BR>LOD_BIAS = " << (float) bias / 256.0F;
	
	output << "</CODE>";
}

static void DescribeTextureCommand(ostream& output, unsigned int reg, const unsigned int *data)
{
	unsigned long unit = (reg >> 5) & 0x0F;
	switch (reg & 0xFE1F)
	{
		case 0x1A00:
			output << "Texture address, unit " << unit;
			break;
		case 0x1A04:
			output << "Texture format, unit " << unit << "<BR>";
			WriteTextureFormat(output, *data);
			break;
		case 0x1A08:
			output << "Texture control 1, unit " << unit << "<BR>";
			WriteTextureControl1(output, *data);
			break;
		case 0x1A0C:
			output << "Texture control 2, unit " << unit << "<BR>";
			WriteTextureControl2(output, *data);
			break;
		case 0x1A10:
			output << "Texture swizzle, unit " << unit << "<BR>";
			WriteTextureSwizzle(output, *data);
			break;
		case 0x1A14:
			output << "Texture filter, unit " << unit << "<BR>";
			WriteTextureFilter(output, *data);
			break;
		case 0x1A18:
			output << "Texture size 1, unit " << unit;
			break;
		case 0x1A1C:
			output << "Texture border color, unit " << unit;
			break;
	}
}

static void DescribeVertexTextureCommand(ostream& output, unsigned int reg, const unsigned int *data)
{
	unsigned long unit = (reg >> 5) & 0x03;
	switch (reg & 0xFF9F)
	{
		case 0x0900:
			output << "Vertex texture address, unit " << unit;
			break;
		case 0x0904:
			output << "Vertex texture format, unit " << unit << "<BR>";
			WriteTextureFormat(output, *data);
			break;
		case 0x0908:
			output << "Vertex texture control 1, unit " << unit << "<BR>";
			WriteTextureControl1(output, *data);
			break;
		case 0x090C:
			output << "Vertex texture control 2, unit " << unit << "<BR>";
			WriteTextureControl2(output, *data);
			break;
		case 0x0910:
			output << "Vertex texture swizzle, unit " << unit;
			break;
		case 0x0914:
			output << "Vertex texture filter, unit " << unit << "<BR>";
			WriteTextureFilter(output, *data);
			break;
		case 0x0918:
			output << "Vertex texture size, unit " << unit;
			break;
		case 0x091C:
			output << "Vertex texture border color, unit " << unit;
			break;
	}
}

static void WriteRasterOperation(ostream& output, unsigned int data)
{
	output << "<CODE>";
	
	for (long a = 0; a < 2; a++)
	{
		switch (data & 0x0F)
		{
			case 0x00:
				output << "CLEAR";
				break;
			case 0x01:
				output << "NOR";
				break;
			case 0x02:
				output << "AND_INVERTED";
				break;
			case 0x03:
				output << "COPY_INVERTED";
				break;
			case 0x04:
				output << "AND_REVERSE";
				break;
			case 0x05:
				output << "INVERT";
				break;
			case 0x06:
				output << "XOR";
				break;
			case 0x07:
				output << "NAND";
				break;
			case 0x08:
				output << "AND";
				break;
			case 0x09:
				output << "EQUIV";
				break;
			case 0x0A:
				output << "NOOP";
				break;
			case 0x0B:
				output << "OR_INVERTED";
				break;
			case 0x0C:
				output << "COPY";
				break;
			case 0x0D:
				output << "OR_REVERSE";
				break;
			case 0x0E:
				output << "OR";
				break;
			case 0x0F:
				output << "SET";
				break;
		}
		
		if (a == 0)
		{
			output << "</CODE>, <CODE>";
			data >>= 4;
		}
	}
	
	output << "</CODE>";
}

static unsigned int transferSource = 0;
static unsigned int transferDest = 0;
static unsigned int transferSize;
static const unsigned int *addressRemap;

static void DumpFragmentProgram(ostream& output)
{
	#ifdef DISASMPC
	
		if (addressRemap)
		{
			const unsigned int *addr = (unsigned int *) (transferSource - addressRemap[0] + addressRemap[1]);
			DisassembleFragmentProgram(output, addr, transferSize / 16);
		}
	#else
		(void)output;
	#endif
}

static void Describe2DCommand(ostream& output, unsigned int reg, Subchannel sc, const unsigned int *data)
{
	if (sc == kSubchannelTransfer)
	{
		switch (reg)
		{
			case 0x030C:
				output << "Transfer source address";
				transferSource = *data;
				break;
			case 0x0310:
				output << "Transfer destination address";
				if (*data != 0) transferDest = *data;
				break;
			case 0x0314:
				output << "Transfer source pitch";
				break;
			case 0x0318:
				output << "Transfer destination pitch";
				break;
			case 0x031C:
				output << "Transfer byte count per row";
				transferSize = *data;
				break;
			case 0x0320:
				output << "Transfer row count";
				break;
			default:
				output << "&nbsp;";
				break;
		}
	}
	else if (sc == kSubchannelClip)
	{
		switch (reg)
		{
			case 0x0300:
				output << "Blit clip <I>x</I>-<I>y</I>";
				break;
			case 0x0304:
				output << "Blit clip width-height";
				break;
			default:
				output << "&nbsp;";
				break;
		}
	}
	else if (sc == kSubchannelSwizzle)
	{
		switch (reg)
		{
			case 0x0300:
				output << "Swizzled surface format";
				break;
			case 0x0304:
				output << "Swizzled surface destination address";
				break;
			default:
				output << "&nbsp;";
				break;
		}
	}
	else if (sc == kSubchannelSurface)
	{
		switch (reg)
		{
			case 0x0300:
				output << "Surface format";
				break;
			case 0x0304:
				output << "Surface pitch src-dst";
				break;
			case 0x0308:
				output << "Surface source address";
				break;
			case 0x030C:
				output << "Surface destination address";
				break;
			default:
				output << "&nbsp;";
				break;
		}
	}
	else if (sc == kSubchannelBlit)
	{
		switch (reg)
		{
			case 0x0300:
				output << "Blit source <I>x</I>-<I>y</I>";
				break;
			case 0x0304:
				output << "Blit destination <I>x</I>-<I>y</I>";
				break;
			case 0x0308:
				output << "Blit width-height";
				break;
			case 0x0400:
				output << "Blit data";
				break;
			default:
				output << "&nbsp;";
				break;
		}
	}
	else if (sc == kSubchannelStretchBlit)
	{
		switch (reg)
		{
			case 0x0300:
				output << "Stretch blit format";
				break;
			case 0x0304:
				output << "Stretch blit operation";
				break;
			case 0x0308:
				output << "Stretch blit clip <I>x</I>-<I>y</I>";
				break;
			case 0x030C:
				output << "Stretch blit clip width-height";
				break;
			case 0x0310:
				output << "Stretch blit destination <I>x</I>-<I>y</I>";
				break;
			case 0x0314:
				output << "Stretch blit destination width-height";
				break;
			case 0x0318:
				output << "Stretch blit <I>du</I>/<I>dx</I>";
				break;
			case 0x031C:
				output << "Stretch blit <I>dv</I>/<I>dy</I>";
				break;
			case 0x0400:
				output << "Stretch blit source width-height";
				break;
			case 0x0404:
				output << "Stretch blit source pitch-origin";
				break;
			case 0x0408:
				output << "Stretch blit source address";
				break;
			case 0x040C:
				output << "Stretch blit source <I>u</I>-<I>v</I>";
				break;
			default:
				output << "&nbsp;";
				break;
		}
	}
	else if (sc == kSubchannelRop)
	{
		if (reg == 0x0300)
		{
			output << "Rop set<BR>";
			WriteRasterOperation(output, *data);
		}
		else
		{
			output << "&nbsp;";
		}
	}
	else if (sc == kSubchannelPattern)
	{
		if ((reg >= 0x0700) && (reg < 0x0800))
		{
			if ((reg & 0x001F) == 0) output << "Pattern 2";
			else output << "&nbsp;";
		}
		else
		{
			switch (reg)
			{
				case 0x0300:
					output << "Pattern format";
					break;
				case 0x030C:
					output << "Pattern select";
					break;
				case 0x0310:
					output << "Pattern color 0";
					break;
				case 0x0314:
					output << "Pattern color 1";
					break;
				case 0x0318:
					output << "Pattern mask 0";
					break;
				case 0x031C:
					output << "Pattern mask 1";
					break;
				default:
					output << "&nbsp;";
					break;
			}
		}
	}
	else if (sc == kSubchannelSync)
	{
		switch (reg)
		{
			case 0x02F0:
				output << "V-sync signal address";
				break;
			case 0x02F4:
				output << "V-sync signal value";
				break;
			default:
				output << "&nbsp;";
				break;
		}
	}
}

static void DescribeCommand(ostream& output, unsigned int reg, Subchannel sc, const unsigned int *data)
{
#if VERBOSE_DRAWCALLS // Display more in-depth information
	static int numComponents[16] = {0};
	static int stride[16] = {0};
	static int vertexType[16] = {0};
	static void *vertexData[16] = {NULL};
	static void *indexData = NULL;
	static unsigned int indexSize = 0;
#endif

	if (reg == 0x0050)
	{
		output << "Reference";
		return;
	}
	
	if (reg == 0x0064)
	{
		output << "Semaphore address";
		return;
	}
	
	if (reg == 0x0068)
	{
		output << "Semaphore wait for value";
		return;
	}
	
	if (reg == 0x006C)
	{
		output << "Semaphore set value";
		return;
	}
	
	float value = *reinterpret_cast<const float *>(data);
	
	if (sc != kSubchannel3D)
	{
		Describe2DCommand(output, reg, sc, data);
		return;
	}
	
	if ((reg >= 0x1A00) && (reg < 0x1C00))
	{
		DescribeTextureCommand(output, reg, data);
		return;
	}
	
	if ((reg >= 0x0900) && (reg < 0x0980))
	{
		DescribeVertexTextureCommand(output, reg, data);
		return;
	}
	
	if ((reg >= 0x0B00) && (reg < 0x0B40))
	{
		unsigned long unit = (reg >> 2) & 0x0F;
		output << "Texture control 3, unit " << unit;
		return;
	}
	
	if ((reg >= 0x1840) && (reg < 0x1880))
	{
		unsigned long unit = (reg >> 2) & 0x0F;
		output << "Texture size 2, unit " << unit;
		return;
	}
	
	if ((reg >= 0x1D00) && (reg < 0x1D40))
	{
		unsigned long unit = (reg >> 2) & 0x0F;
		output << "Texture control 4, unit " << unit;
		return;
	}
	
	if ((reg >= 0x0B40) && (reg < 0x0B60))
	{
		unsigned long texcoord = (reg >> 2) & 0x0F;
		output << "Texture coordinate control, texcoord " << texcoord;
		return;
	}
	
	if ((reg >= 0x1680) && (reg < 0x16C0))
	{
		unsigned long array = (reg >> 2) & 0x0F;
		output << "Vertex attribute array address " << array;
#if VERBOSE_DRAWCALLS // Display more in-depth information
		vertexData[array] = Ice::Render::TranslateOffsetToAddress(*data);
#endif
		return;
	}
	
	if ((reg >= 0x1740) && (reg < 0x1780))
	{
		unsigned long array = (reg >> 2) & 0x0F;
		output << "Vertex attribute format " << array << "<BR>";
		WriteAttributeFormat(output, *data);
#if VERBOSE_DRAWCALLS // Display more in-depth information
		stride[array] = *data >> 8;
		numComponents[array] = ((*data >> 4) & 0x0F);
		vertexType[array] = *data & 0x0F;
#endif
		return;
	}
	
	if ((reg >= 0x0A80) && (reg < 0x0B00))
	{
		unsigned long attrib = (reg >> 3) & 0x0F;
		output << "Vertex attrib 4Ns, attrib " << attrib;
		return;
	}
	
	if ((reg >= 0x1500) && (reg < 0x1600))
	{
		unsigned long attrib = (reg >> 4) & 0x0F;
		output << "Vertex attrib 3f, attrib " << attrib;
		output << " (" << value << ")";
		return;
	}
	
	if ((reg >= 0x1880) && (reg < 0x1900))
	{
		unsigned long attrib = (reg >> 3) & 0x0F;
		output << "Vertex attrib 2f, attrib " << attrib;
		output << " (" << value << ")";
		return;
	}
	
	if ((reg >= 0x1900) && (reg < 0x1940))
	{
		unsigned long attrib = (reg >> 2) & 0x0F;
		output << "Vertex attrib 2s, attrib " << attrib;
		return;
	}
	
	if ((reg >= 0x1940) && (reg < 0x1980))
	{
		unsigned long attrib = (reg >> 2) & 0x0F;
		output << "Vertex attrib 4Nub, attrib " << attrib;
		return;
	}
	
	if ((reg >= 0x1980) && (reg < 0x1A00))
	{
		unsigned long attrib = (reg >> 3) & 0x0F;
		output << "Vertex attrib 4s, attrib " << attrib;
		return;
	}
	
	if ((reg >= 0x1C00) && (reg < 0x1D00))
	{
		unsigned long attrib = (reg >> 4) & 0x0F;
		output << "Vertex attrib 4f, attrib " << attrib;
		output << " (" << value << ")";
		return;
	}
	
	if ((reg >= 0x1E40) && (reg < 0x1E80))
	{
		unsigned long attrib = (reg >> 2) & 0x0F;
		output << "Vertex attrib 1f, attrib " << attrib;
		output << " (" << value << ")";
		return;
	}
	
	if ((reg >= 0x1F00) && (reg < 0x1F40))
	{
		static const char comp[4] = {'x', 'y', 'z', 'w'};
		
		unsigned long slot = (reg >> 4) & 0x03;
		output << "Vertex program constant " << slot << " <I>" << comp[(reg >> 2) & 0x03] << "</I>";
		output << " (" << value << ")";
		return;
	}
	
	if ((reg >= 0x02C0) && (reg < 0x0300))
	{
		if ((reg & 0x04) == 0) output << "Window clip left-right ";
		else output << "Window clip top-bottom ";
		output << ((reg - 0x02C0) >> 3);
		return;
	}
	
	switch (reg)
	{
		case 0x0100:
			output << "Nop";
			// Output a special Debug String for easier push buffer debugging
			if(*data == 0xDB65DB65)
				output << "<BR> Debug String: " << (char *)&data[1];
			break;
		case 0x018C:
			output << "Color buffer 1 context";
			break;
		case 0x0194:
			output << "Color buffer 0 context";
			break;
		case 0x0198:
			output << "Depth buffer context";
			break;
		case 0x01A8:
			output << "Report context";
			break;
		case 0x01B4:
			output << "Color buffer 2 context";
			break;
		case 0x01B8:
			output << "Color buffer 3 context";
			break;
		case 0x0200:
			output << "Render target left-width";
			break;
		case 0x0204:
			output << "Render target top-height";
			break;
		case 0x0208:
			output << "Color buffer format";
			break;
		case 0x020C:
			output << "Color buffer 0 pitch";
			break;
		case 0x0210:
			output << "Color buffer 0 base address";
			break;
		case 0x0214:
			output << "Depth buffer base address";
			break;
		case 0x0218:
			output << "Color buffer 1 base address";
			break;
		case 0x021C:
			output << "Color buffer 1 pitch";
			break;
		case 0x0220:
			output << "Draw buffer mask";
			break;
		case 0x022C:
			output << "Depth buffer pitch";
			break;
		case 0x0234:
			output << "Invalidate Depth-cull";
			break;
		case 0x0280:
			output << "Color buffer 2 pitch";
			break;
		case 0x0284:
			output << "Color buffer 3 pitch";
			break;
		case 0x0288:
			output << "Color buffer 2 base address";
			break;
		case 0x028C:
			output << "Color buffer 3 base address";
			break;
		case 0x02B8:
			output << "Render target offset";
			break;
		case 0x02BC:
			output << "Window clip mode";
			break;
		case 0x0300:
			output << "Dither enable";
			break;
		case 0x0304:
			output << "Alpha test enable";
			break;
		case 0x0308:
			output << "Alpha function<BR>";
			WriteCompareFunction(output, *data);
			break;
		case 0x030C:
			output << "Alpha reference value";
			break;
		case 0x0310:
			output << "Blend enable";
			break;
		case 0x0314:
			output << "Blend source factor<BR><CODE>RGB = </CODE>";
			WriteBlendFactor(output, *data & 0xFFFF);
			output << "<BR><CODE>&nbsp;&nbsp;A = </CODE>";
			WriteBlendFactor(output, *data >> 16);
			break;
		case 0x0318:
			output << "Blend destination factor<BR><CODE>RGB = </CODE>";
			WriteBlendFactor(output, *data & 0xFFFF);
			output << "<BR><CODE>&nbsp;&nbsp;A = </CODE>";
			WriteBlendFactor(output, *data >> 16);
			break;
		case 0x031C:
			output << "Blend color";
			break;
		case 0x0320:
			output << "Blend equation<BR><CODE>RGB = </CODE>";
			WriteBlendEquation(output, *data & 0xFFFF);
			output << "<BR><CODE>&nbsp;&nbsp;A = </CODE>";
			WriteBlendEquation(output, *data >> 16);
			break;
		case 0x0324:
			output << "Color mask";
			break;
		case 0x0328:
			output << "Stencil test enable front";
			break;
		case 0x032C:
			output << "Stencil mask front";
			break;
		case 0x0330:
			output << "Stencil function front<BR>";
			WriteCompareFunction(output, *data);
			break;
		case 0x0334:
			output << "Stencil reference value front";
			break;
		case 0x0338:
			output << "Stencil comparison mask front";
			break;
		case 0x033C:
			output << "Stencil fail operation front<BR>";
			WriteStencilOperation(output, *data);
			break;
		case 0x0340:
			output << "Stencil pass depth fail operation front<BR>";
			WriteStencilOperation(output, *data);
			break;
		case 0x0344:
			output << "Stencil pass depth pass operation front<BR>";
			WriteStencilOperation(output, *data);
			break;
		case 0x0348:
			output << "Stencil test enable back";
			break;
		case 0x034C:
			output << "Stencil mask back";
			break;
		case 0x0350:
			output << "Stencil function back<BR>";
			WriteCompareFunction(output, *data);
			break;
		case 0x0354:
			output << "Stencil reference value back";
			break;
		case 0x0358:
			output << "Stencil comparison mask back";
			break;
		case 0x035C:
			output << "Stencil fail operation back<BR>";
			WriteStencilOperation(output, *data);
			break;
		case 0x0360:
			output << "Stencil pass depth fail operation back<BR>";
			WriteStencilOperation(output, *data);
			break;
		case 0x0364:
			output << "Stencil pass depth pass operation back<BR>";
			WriteStencilOperation(output, *data);
			break;
		case 0x0368:
			output << "Shading model<BR>";
			WriteShadingModel(output, *data);
			break;
		case 0x036C:
			output << "MRT blend enable";
			break;
		case 0x0370:
			output << "MRT color mask";
			break;
		case 0x0374:
			output << "Logic operation enable";
			break;
		case 0x0378:
			output << "Logic operation<BR>";
			WriteLogicOperation(output, *data);
			break;
		case 0x037C:
			output << "Floating-point blend color";
			break;
		case 0x0380:
			output << "Depth bounds test enable";
			break;
		case 0x0384:
			output << "Depth bounds minimum (" << value << ")";
			break;
		case 0x0388:
			output << "Depth bounds maximum (" << value << ")";
			break;
		case 0x0394:
			output << "Depth range minimum (" << value << ")";
			break;
		case 0x0398:
			output << "Depth range maximum (" << value << ")";
			break;
		case 0x03B8:
			output << "Line width";
			break;
		case 0x03BC:
			output << "Line smooth enable";
			break;
		case 0x08C0:
			output << "Scissor left-width";
			break;
		case 0x08C4:
			output << "Scissor top-height";
			break;
		case 0x08CC:
			output << "Fog mode<BR>";
			WriteFogMode(output, *data);
			break;
		case 0x08D0:
			output << "Fog bias (" << value << ")";
			break;
		case 0x08D4:
			output << "Fog scale (" << value << ")";
			break;
		case 0x08E4:
			output << "Fragment program address";
			if ((*data & ~7) == transferDest)
			{
				output << "<BR><PRE STYLE=\"text-indent: 0;\">";
				DumpFragmentProgram(output);
				output << "</PRE>";
			}
			break;
		case 0x0A00:
			output << "Viewport left-width";
			break;
		case 0x0A04:
			output << "Viewport top-height";
			break;
		case 0x0A1C:
			output << "Depth-cull sync";
			break;
		case 0x0A20:
			output << "Viewport translate <I>x</I> (" << value << ")";
			break;
		case 0x0A24:
			output << "Viewport translate <I>y</I> (" << value << ")";
			break;
		case 0x0A28:
			output << "Viewport translate <I>z</I> (" << value << ")";
			break;
		case 0x0A2C:
			output << "Viewport translate <I>w</I> (" << value << ")";
			break;
		case 0x0A30:
			output << "Viewport scale <I>x</I> (" << value << ")";
			break;
		case 0x0A34:
			output << "Viewport scale <I>y</I> (" << value << ")";
			break;
		case 0x0A38:
			output << "Viewport scale <I>z</I> (" << value << ")";
			break;
		case 0x0A3C:
			output << "Viewport scale <I>w</I> (" << value << ")";
			break;
		case 0x0A60:
			output << "Polygon offset point enable";
			break;
		case 0x0A64:
			output << "Polygon offset line enable";
			break;
		case 0x0A68:
			output << "Polygon offset fill enable";
			break;
		case 0x0A6C:
			output << "Depth function<BR>";
			WriteCompareFunction(output, *data);
			break;
		case 0x0A70:
			output << "Depth mask";
			break;
		case 0x0A74:
			output << "Depth test enable";
			break;
		case 0x0A78:
			output << "Polygon offset factor (" << value << ")";
			break;
		case 0x0A7C:
			output << "Polygon offset units (" << value << ")";
			break;
		case 0x0B80:
			output << "Vertex program instruction";
			break;
		case 0x0B88:
			output << "<CODE STYLE=\"text-indent: 0;\">";
			DisassembleVertexInstruction(output, data - 2, false, true);
			output << "</CODE>";
			break;
		case 0x0B90:
			output << "Vertex program instruction";
			break;
		case 0x0B98:
			output << "<CODE STYLE=\"text-indent: 0;\">";
			DisassembleVertexInstruction(output, data - 2, false, true);
			output << "</CODE>";
			break;
		case 0x0BA0:
			output << "Vertex program instruction";
			break;
		case 0x0BA8:
			output << "<CODE STYLE=\"text-indent: 0;\">";
			DisassembleVertexInstruction(output, data - 2, false, true);
			output << "</CODE>";
			break;
		case 0x0BB0:
			output << "Vertex program instruction";
			break;
		case 0x0BB8:
			output << "<CODE STYLE=\"text-indent: 0;\">";
			DisassembleVertexInstruction(output, data - 2, false, true);
			output << "</CODE>";
			break;
		case 0x0BC0:
			output << "Vertex program instruction";
			break;
		case 0x0BC8:
			output << "<CODE STYLE=\"text-indent: 0;\">";
			DisassembleVertexInstruction(output, data - 2, false, true);
			output << "</CODE>";
			break;
		case 0x0BD0:
			output << "Vertex program instruction";
			break;
		case 0x0BD8:
			output << "<CODE STYLE=\"text-indent: 0;\">";
			DisassembleVertexInstruction(output, data - 2, false, true);
			output << "</CODE>";
			break;
		case 0x0BE0:
			output << "Vertex program instruction";
			break;
		case 0x0BE8:
			output << "<CODE STYLE=\"text-indent: 0;\">";
			DisassembleVertexInstruction(output, data - 2, false, true);
			output << "</CODE>";
			break;
		case 0x0BF0:
			output << "Vertex program instruction";
			break;
		case 0x0BF8:
			output << "<CODE STYLE=\"text-indent: 0;\">";
			DisassembleVertexInstruction(output, data - 2, false, true);
			output << "</CODE>";
			break;
		case 0x1428:
			output << "Secondary color enable";
			break;
		case 0x142C:
			output << "Back-facing color enable";
			break;
		case 0x1438:
			output << "Culling Control";
			break;
		case 0x1450:
			output << "Vertex program control 0";
			break;
		case 0x1478:
			output << "Clip plane control";
			break;
		case 0x147C:
			output << "Polygon stipple enable";
			break;
		case 0x1480:
			output << "Polygon stipple pattern";
			break;
		case 0x1710:
			output << "Invalidate pre-transform vertex cache";
			break;
		case 0x1714:
			output << "Invalidate post-transform vertex cache";
			break;
		case 0x1718:
			output << "Graphics pipe nop";
			break;
		case 0x1738:
			output << "Vertex array base offset";
			break;
		case 0x173C:
			output << "Index array base offset";
			break;
		case 0x17C8:
			output << "Reset Report";
			break;
		case 0x17CC:
			output << "Pixel counter enable";
			break;
		case 0x1800:
			output << "Write Report";
			break;
		case 0x1804:
			output << "Depth-cull stats enable";
			break;
		case 0x1808:
			output << "Draw mode<BR>";
			WriteDrawMode(output, *data);
			break;
		case 0x180C:
			output << "Draw 16-bit elements immediate";
			break;
		case 0x1810:
			output << "Draw 32-bit elements immediate";
			break;
		case 0x1814:
		{
			output << "Draw arrays";
#if VERBOSE_DRAWCALLS // Display more in-depth information
			//try
			{
				output << "<BR>";
				int numElems = (*data >> 24)+1;
				int offset = *data & 0xFFFFFF;
				for(int i=0; i<numElems; ++i)
				{
					int io = i + offset;
					output << io << ": ";
					for(int ii = 0; ii < 16; ++ii)
					{
						if((vertexData[ii] != NULL) && ((unsigned int)(unsigned long)vertexData[ii] != ~0))
						{
							output << "\t" << ii << "(" ;
							int s = stride[ii];
							for(int c = 0; c < numComponents[ii]; ++c)
							{
								if(c != 0) output << ", ";
								int cs = 0;
								switch(vertexType[ii])
								{
								case 0x01: cs = 2; break; // SN
								case 0x02: cs = 4; break; // F32
								case 0x03: cs = 2; break; // F16
								case 0x04: cs = 1; break; // BN
								case 0x05: cs = 1; break; // S
								case 0x06: cs = 4; break; // 101111
								case 0x07: cs = 1; break; // B
								}
								void *d = &((U8*)vertexData[ii])[s*io+c*cs];
								switch(vertexType[ii])
								{
								case 0x01: output << ((float)(*(unsigned short*)d) - 32767.f) / 65536.f; break;
								case 0x02: output << (*(float*)d); break;
								case 0x03: output << F16toF32(*(short*)d); break;
								case 0x04: output << ((float)(*(unsigned char*)d) - 127.f) / 256.f; break;
								case 0x05: output << (*(short*)d); break;
								//case 0x06: output << (*(float*)d); break; // 101111
								case 0x07: output << (*(unsigned char*)d); break;
								}
							}
							output << ")";
						}
					}
					output << "<BR>";
				}
			}
			//catch(...)
			{
			}
#endif
			break;
		}
		case 0x1818:
			output << "Draw arrays immediate";
			break;
		case 0x181C:
			output << "Index buffer address";
#if VERBOSE_DRAWCALLS // Display more in-depth information
			indexData = (unsigned short*)Ice::Render::TranslateOffsetToAddress(*data);
#endif
			break;
		case 0x1820:
			output << "Index buffer format";
#if VERBOSE_DRAWCALLS // Display more in-depth information
			indexSize = (*data & 0x10) ? 2 : 4;
#endif
			break;
		case 0x1824:
		{
			output << "Draw elements";
#if VERBOSE_DRAWCALLS // Display more in-depth information
			//try
			{
				output << "<BR>";
				if((indexData != NULL) && ((unsigned int)(unsigned long)indexData != ~0))
				{
					int numElems = (*data >> 24)+1;
					int offset = *data & 0xFFFFFF;
					for(int i=0; i<numElems; ++i)
					{
						int io = (indexSize == 2) ? ((unsigned short*)indexData)[i + offset] : ((unsigned int*)indexData)[i + offset];
						output << io << ": ";
						for(int ii = 0; ii < 16; ++ii)
						{
							if((vertexData[ii] != NULL) && ((unsigned int)(unsigned long)vertexData[ii] != ~0))
							{
								output << "\t" << ii << "(" ;
								int s = stride[ii];
								for(int c = 0; c < numComponents[ii]; ++c)
								{
									if(c != 0) output << ", ";
									int cs = 0;
									switch(vertexType[ii])
									{
									case 0x01: cs = 2; break; // SN
									case 0x02: cs = 4; break; // F32
									case 0x03: cs = 2; break; // F16
									case 0x04: cs = 1; break; // BN
									case 0x05: cs = 1; break; // S
									case 0x06: cs = 4; break; // 101111
									case 0x07: cs = 1; break; // B
									}
									void *d = &((U8*)vertexData[ii])[s*io+c*cs];
									switch(vertexType[ii])
									{
									case 0x01: output << ((float)(*(unsigned short*)d) - 32767.f) / 65536.f; break;
									case 0x02: output << (*(float*)d); break;
									case 0x03: output << F16toF32(*(short*)d); break; // F16
									case 0x04: output << ((float)(*(unsigned char*)d) - 127.f) / 256.f; break;
									case 0x05: output << (*(short*)d); break;
									//case 0x06: output << (*(float*)d); break; // 101111
									case 0x07: output << (*(unsigned char*)d); break;
									}
								}
								output << ")";
							}
						}
						output << "<BR>";
					}
				}
			}
			//catch(...)
			{
			}
#endif
			break;
		}
		case 0x1828:
			output << "Polygon mode front<BR>";
			WritePolygonMode(output, *data);
			break;
		case 0x182C:
			output << "Polygon mode back<BR>";
			WritePolygonMode(output, *data);
			break;
		case 0x1830:
			output << "Cull face mode<BR>";
			WriteCullFace(output, *data);
			break;
		case 0x1834:
			output << "Front face mode<BR>";
			WriteFrontFace(output, *data);
			break;
		case 0x1838:
			output << "Polygon smooth enable";
			break;
		case 0x183C:
			output << "Cull face enable";
			break;
		case 0x1D60:
			output << "Fragment program control";
			break;
		case 0x1D64:
			output << "Vertex program constant range";
			break;
		case 0x1D6C:
			output << "Semaphore signal address";
			break;
		case 0x1D70:
			output << "Back-end write semaphore release.";
			break;
		case 0x1D74:
			output << "Texture read semaphore release.";
			break;
		case 0x1D78:
			output << "Depth clamp control";
			break;
		case 0x1D7C:
			output << "Multisample control";
			break;
		case 0x1D84:
			output << "Depth-cull enable";
			break;
		case 0x1D88:
			output << "Render Target Aux Height";
			break;
		case 0x1D8C:
			output << "Depth/stencil clear value";
			break;
		case 0x1D90:
			output << "Color clear value";
			break;
		case 0x1D94:
			output << "Clear<BR>";
			WriteClearChannels(output, *data);
			break;
		case 0x1D98:
			output << "Clear Window left-width";
			break;
		case 0x1D9C:
			output << "Clear Window top-height";
			break;
		case 0x1DA4:
			output << "Render target control 1";
			break;
		case 0x1DAC:
			output << "Primitive restart enable";
			break;
		case 0x1DB0:
			output << "Primitive restart index";
			break;
		case 0x1DB4:
			output << "Line stipple enable";
			break;
		case 0x1DB8:
			output << "Line stipple pattern";
			break;
		case 0x1E98:
			output << "Render control";
			break;
		case 0x1E9C:
			output << "Vertex program instruction load slot";
			break;
		case 0x1EA0:
			output << "Vertex program execute slot";
			break;
		case 0x1EA4:
			output << "Depth-cull depth params";
			break;
		case 0x1EA8:
			output << "Depth-cull feedback params";
			break;
		case 0x1EAC:
			output << "Depth-cull stencil params";
			break;
		case 0x1EE0:
			output << "Global point size (" << value << ")";
			break;
		case 0x1EE4:
			output << "Vertex program point size enable";
			break;
		case 0x1EE8:
			output << "Point sprite control";
			break;
		case 0x1EF8:
			output << "Vertex program limits";
			break;
		case 0x1EFC:
			output << "Vertex program constant load slot";
			break;
		case 0x1F00:
			output << "Vertex program constant 0";
			break;
		case 0x1F10:
			output << "Vertex program constant 1";
			break;
		case 0x1F20:
			output << "Vertex program constant 2";
			break;
		case 0x1F30:
			output << "Vertex program constant 3";
			break;
		case 0x1F40:
			output << "Vertex program constant 4";
			break;
		case 0x1F50:
			output << "Vertex program constant 5";
			break;
		case 0x1F60:
			output << "Vertex program constant 6";
			break;
		case 0x1F70:
			output << "Vertex program constant 7";
			break;
		case 0x1FC0:
			output << "Vertex frequency control";
			break;
		case 0x1FC4:
			output << "Vertex result mapping 1";
			break;
		case 0x1FC8:
			output << "Vertex result mapping 2";
			break;
		case 0x1FCC:
			output << "Vertex result mapping 3";
			break;
		case 0x1FD0:
			output << "Vertex result mapping 4";
			break;
		case 0x1FD4:
			output << "Vertex result mapping 5";
			break;
		case 0x1FD8:
			output << "Invalidate texture cache";
			break;
		case 0x1FEC:
			output << "Fragment program control 2";
			break;
		case 0x1FF0:
			output << "Vertex attribute mask<BR><CODE>";
			WriteVertexAttributes(output, *data);
			output << "</CODE>";
			break;
		case 0x1FF4:
			output << "Vertex result mask<BR><CODE>";
			WriteVertexResults(output, *data);
			output << "</CODE>";
			break;
		default:
			output << "&nbsp;";
			break;
	}
}

void DumpCommandBuffer(ostream& output, const void *buffer, unsigned long size, const unsigned int *remap)
{
	addressRemap = remap;
	
	#if DISASMPC
	
		currentSubchannel[0] = kSubchannel3D;
		currentSubchannel[1] = kSubchannel3D;
		currentSubchannel[2] = kSubchannelStretchBlit;
		currentSubchannel[3] = kSubchannelTransfer;
		currentSubchannel[4] = kSubchannelSwizzle;
		currentSubchannel[5] = kSubchannelBlit;
		currentSubchannel[6] = kSubchannelSurface;
		currentSubchannel[7] = kSubchannelClip;
	
	#else
	
		currentSubchannel[0] = kSubchannel3D;
		currentSubchannel[1] = kSubchannelTransfer;
		currentSubchannel[2] = kSubchannelStretchBlit;
		currentSubchannel[3] = kSubchannelSurface;
		currentSubchannel[4] = kSubchannelSwizzle;
		currentSubchannel[5] = kSubchannelBlit;
		currentSubchannel[6] = kSubchannelStretchBlit;
		currentSubchannel[7] = kSubchannelClip;
	
	#endif
	
	output << "<!DOCTYPE HTML PUBLIC \"-//W3C//DTD HTML 4.01 Transitional//EN\" \"http://www.w3.org/TR/html4/loose.dtd\">" << endl;
	output << "<HTML>" << endl << "<HEAD>" << endl;
	output << "<TITLE>Command Buffer Dump</TITLE>" << endl;
	output << "<STYLE type=\"text/css\">" << endl;
    output << "TD.cmd1 {vertical-align: top; background-color: #DDDDDD; border-top: solid #884444 1px; padding: 2px 4px 12px 4px;}" << endl;
    output << "TD.cmd2 {vertical-align: top; background-color: #DDDDDD; border-top: solid #BBBBBB 1px; padding: 2px 4px 2px 40px;}" << endl;
    output << "TD.cmd3 {vertical-align: top; background-color: #DDDDDD; border-top: solid #BBBBBB 1px; padding: 2px 4px 12px 40px;}" << endl;
    output << "TD.desc1 {width: 600px; vertical-align: top; font-family: arial; font-size: 10pt; background-color: #EEEEEE; border-top: solid #884444 1px; padding: 1px 4px 12px 10px;}" << endl;
    output << "TD.desc2 {width: 600px; vertical-align: top; text-indent: -16px; font-family: arial; font-size: 10pt; background-color: #EEEEEE; border-top: solid #BBBBBB 1px; padding: 1px 4px 2px 26px;}" << endl;
    output << "TD.desc3 {width: 600px; vertical-align: top; text-indent: -16px; font-family: arial; font-size: 10pt; background-color: #EEEEEE; border-top: solid #BBBBBB 1px; padding: 1px 4px 12px 26px;}" << endl;
	output << "CODE {font-family: \"courier new\"; font-size: 10pt;}" << endl;
	output << "</STYLE" << endl << "</HEAD><BODY>" << endl;
	output << "<TABLE CELLSPACING=0 CELLPADDING=0 BORDER=0 STYLE=\"margin-left: 48px;\">" << endl;
	
	const unsigned int *cmdptr = static_cast<const unsigned int *>(buffer);
	const unsigned int *endptr = cmdptr + size / 4;
	
	while (cmdptr < endptr)
	{
		output << "<TR><TD CLASS=\"cmd1\"><CODE>" << endl;
		
		unsigned int cmd = *cmdptr++;
		WriteHexHalf(output, cmd >> 16);
		output << " ";
		WriteHexHalf(output, cmd);
		
		output << "</CODE></TD>" << endl << "<TD CLASS=\"desc1\">";
		if ((cmd & 0x20030003) != 0)
		{
			output << "<SPAN STYLE=\"color: #A00000;\">";
			if ((cmd & 0x20000001) != 0) output << "Jump";
			else if ((cmd & 0x00000002) != 0) output << "Call";
			else if ((cmd & 0x00020000) != 0) output << "Return";
			else if ((cmd & 0x00010000) == 0x00010000) output << "SLI";
			output << "</SPAN>";
		}
		// Properly Interpret 0's as Nops
		else if(cmd == 0)
		{
			output << "<SPAN STYLE=\"color: #A00000;\">";
			output << "Nop";
			output << "</SPAN>";
		}
		else
		{
			output << "&nbsp;";
		}
		
		output << "</TD></TR>" << endl;
		
		if ((cmd & 0x20030003) == 0)
		{
			unsigned int len = (cmd >> 18) & 0x01FF;
			unsigned int reg = cmd & 0x1FFC;
			unsigned int map = (cmd >> 13) & 0x0007;
			Subchannel sc = currentSubchannel[map];
			
			const unsigned int *data = cmdptr;
			cmdptr += len;
			
			if ((reg == 0x0000) && (len >= 1))
			{
				switch (*data & 0xFFFF)
				{
					case 0x4301:
						currentSubchannel[map] = kSubchannelRop;
						break;
					case 0x4401:
						currentSubchannel[map] = kSubchannelPattern;
						break;
					case 0x4901:
						currentSubchannel[map] = kSubchannelSync;
						break;
					case 0x4A01:
						currentSubchannel[map] = kSubchannelRect;
						break;
					case 0x5F02:
						currentSubchannel[map] = kSubchannelBlit;
						break;
					default:
						currentSubchannel[map] = kSubchannelUnknown;
						break;
				}
			}
			
			for (unsigned int a = 0; a < len; a++)
			{
				if (a < len - 1) output << "<TR><TD CLASS=\"cmd2\"><CODE>";
				else output << "<TR><TD CLASS=\"cmd3\"><CODE>";
				
				WriteHexWord(output, data[a]);
				output << "</CODE></TD>" << endl;
				
				if (a < len - 1) output << "<TD CLASS=\"desc2\">";
				else output << "<TD CLASS=\"desc3\">";
				
				if (reg != 0)
				{
					if (sc != kSubchannel3D) output << "<SPAN STYLE=\"color: #008000;\">";
					DescribeCommand(output, reg, sc, &data[a]);
					if (sc != kSubchannel3D) output << "</SPAN>";
				}
				else
				{
					output << "<B>Subchannel " << map << " = " << subchannelName[currentSubchannel[map]] << "</B>";
				}
				
				if((cmd & 0x40000000) == 0)
					reg += 4;
				
				output << "</TD></TR>" << endl;
			}
		}
	}
	
	output << endl << "</TABLE>" << endl;
	output << "</BODY></HTML>" << endl;
}
