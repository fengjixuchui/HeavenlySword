#if !defined( HERESY_CMACROS_H )
#define HERESY_CMACROS_H

//! cmd is first RSX register that this pushbuffer command will change, 
//! payload is the number of bytes to copy to this register (default is incrementing set the non-increment flag in the other case)
#define Heresy_Cmd( cmd, payload) ((cmd) | ((payload) << 16))
//! check this push buffer has space for this many 32 bits
#define Heresy_CheckPBSpace( pb, space ) ntAssert( (pb->m_pCurrent+(space)) < pb->m_pEnd )
//! just a helper to easily port etc
#define Heresy_Assert( chk ) ntAssert( chk )
//! increments the push buffer
#define Heresy_IncPB( pb, space ) pb->m_pCurrent+=(space)

// these set and copy macros are for ease of porting and optimisation once we hit SPU etc. nothing special or complex nor should they
// be over stressed as they are delibrately simple as buggery

// dest is assumed to be a 32 bit pointer, src is an actual uint32_t
#define Heresy_Set32bit( dest, src )	\
	*(dest) = (src);
#define Heresy_Set128bit( dest, src )	NT_MEMCPY( dest, &src, 16 )

// source and dest are assumed to be uint32_t* pointers or float* cos that basically all heresy uses...
#define Heresy_CopyN32bits( dest, src, N )	NT_MEMCPY( dest, src, N*4)
#define Heresy_CopyN128bits( dest, src, N )	NT_MEMCPY( dest, src, N*16)

#define Heresy_TextureAddressReg( s, t, r, remap_bit, zfunc )		\
	(s) | ((t) << 8) | ((r) << 16) | ((remap_bit) << 13) | ((zfunc) << 28)

#define Heresy_TextureFilterReg( magfilter, minfilter, lodbias ) \
	(lodbias) | (0x2 << 12) | ((minfilter) << 16) | ((magfilter) << 24)

#define Heresy_IndexArrayType( type, rampool )					\
	(type) | (rampool)
#define Heresy_VertexStreamFormatReg( type, count, stride, divider )	\
	(type) | ((count) << 4) | ((stride) << 8) | ((divider)<<16 )

#define Heresy_IndexArrayParamRegs( count, indexoffset )		\
	indexoffset | (((count)-1)<<24)

#define Heresy_PushBufferInlineFixupToken 0xC000

#define Heresy_ScissorHorizontal( x, width )							\
	((width) << 16) | x

#define Heresy_ScissorVertical( y, height )							\
	((height) << 16) | y


#endif
