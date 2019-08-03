//--------------------------------------------------
//!
//!	\file TextureReader.h
//!	Class that wraps the getting and setting of 
//! texture values
//!
//--------------------------------------------------

#ifndef _TEX_READER_PS3_H
#define _TEX_READER_PS3_H

#include "core/half.h"
#include "gfx/gfxformat.h"

//--------------------------------------------------
//!
//! TextureReader
//!
//--------------------------------------------------
class TextureReader
{
public:
	TextureReader( void* pData, GFXFORMAT fmt );

	void Next( u_int step = 1 );
	void Reset();

	// read current value into a 4 vec, rgba style
	CVector Current( void ) const;

	// set current value from a 4 vec
	void Set( const CVector& rgba );

private:
	void*		m_pStart;
	void*		m_pCurr;
	GFXFORMAT	m_fmt;
	int			m_iStride;
};

#endif // _TEX_READER_PS3_H
