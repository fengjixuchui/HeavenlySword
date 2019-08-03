//--------------------------------------------------
//!
//!	\file TextureReader.h
//!	Class that wraps the getting and setting of 
//! texture values
//!
//--------------------------------------------------

#ifndef _TEX_READER_PC_H
#define _TEX_READER_PC_H

#include "gfx/gfxformat.h"

//--------------------------------------------------
//!
//! TextureReader
//!
//--------------------------------------------------
class TextureReader
{
public:
	TextureReader( void* pData, GFXFORMAT fmt )
	{
		ntAssert( pData );

		m_pStart = pData;
		m_pCurr = pData;
		m_fmt = fmt;
		
		switch( m_fmt )
		{
		case GF_RGB8:			m_iStride = 3; break;
		case GF_ARGB8:			m_iStride = 4; break;
		case GF_XRGB8:			m_iStride = 4; break;
		case GF_L8:				m_iStride = 1; break;
		case GF_A8:				m_iStride = 1; break;
		case GF_L8A8:			m_iStride = 2; break;
		case GF_V8U8:			m_iStride = 2; break;
		case GF_ABGR16F:		m_iStride = 8; break;
		case GF_ABGR32F:		m_iStride = 16; break;		
		case GF_R16F:			m_iStride = 2; break;
		case GF_R32F:			m_iStride = 4; break;
		
		default:
			ntError_p( 0, ("Unsupported tex format") );
		}
	}

	void Next( u_int step = 1 )		{ m_pCurr = ((uint8_t*)m_pCurr + (m_iStride*step)); }
	void Reset()					{ m_pCurr = m_pStart; }

	// read current value into a 4 vec, rgba style
	CVector Current( void ) const
	{
		switch( m_fmt )
		{
		case GF_RGB8:
			{
				struct Pixel { u_char b,g,r; };
				Pixel* pData = reinterpret_cast<Pixel*>( m_pCurr );
				CVector rgba( pData->r, pData->g, pData->b, 1.0f );
				return rgba * (1.0f / 255.0f);
			}

		case GF_ARGB8:
			{
				struct Pixel { u_char b,g,r,a; };
				Pixel* pData = reinterpret_cast<Pixel*>( m_pCurr );
				CVector rgba( pData->r, pData->g, pData->b, pData->a );
				return rgba * (1.0f / 255.0f);
			}

		case GF_XRGB8:
			{
				struct Pixel { u_char b,g,r,a; };
				Pixel* pData = reinterpret_cast<Pixel*>( m_pCurr );
				CVector rgba( pData->r, pData->g, pData->b, 1.0f );
				return rgba * (1.0f / 255.0f);
			}

		case GF_L8:
		case GF_A8:
			{
				struct Pixel { u_char val; };
				Pixel* pData = reinterpret_cast<Pixel*>( m_pCurr );
				CVector rgba( pData->val, pData->val, pData->val, pData->val );
				return rgba * (1.0f / 255.0f);
			}

		case GF_L8A8:
			{
				struct Pixel { u_char l,a; };
				Pixel* pData = reinterpret_cast<Pixel*>( m_pCurr );
				CVector rgba( pData->l, pData->l, pData->l, pData->a );
				return rgba * (1.0f / 255.0f);
			}

		case GF_V8U8:
			{
				struct Pixel { u_char b,g; };
				Pixel* pData = reinterpret_cast<Pixel*>( m_pCurr );
				CVector rgba( pData->b, pData->g, pData->b, pData->g );
				return rgba * (1.0f / 255.0f);
			}
	
		case GF_ABGR16F:
			{
				struct Pixel { D3DXFLOAT16 r,g,b,a; };
				Pixel* pData = reinterpret_cast<Pixel*>( m_pCurr );
				return CVector( pData->r, pData->g, pData->b, pData->a );
			}

		case GF_ABGR32F:
			{
				struct Pixel { float r,g,b,a; };
				Pixel* pData = reinterpret_cast<Pixel*>( m_pCurr );
				return CVector( pData->r, pData->g, pData->b, pData->a );
			}

		case GF_R16F:
			{
				struct Pixel { D3DXFLOAT16 r; };
				Pixel* pData = reinterpret_cast<Pixel*>( m_pCurr );
				return CVector( pData->r, pData->r, pData->r, pData->r );
			}

		case GF_R32F:
			{
				struct Pixel { float r; };
				Pixel* pData = reinterpret_cast<Pixel*>( m_pCurr );
				return CVector( pData->r, pData->r, pData->r, pData->r );
			}
		}

		return CVector( CONSTRUCT_CLEAR );
	}

	// set current value from a 4 vec
	void Set( const CVector& rgba )
	{
		switch( m_fmt )
		{
		case GF_RGB8:
			{
				struct Pixel { u_char b,g,r; };
				Pixel* pData = reinterpret_cast<Pixel*>( m_pCurr );
				
				pData->r = (u_char)(ntstd::Clamp( ((int)(rgba.X() * 255.5f)), 0, 0xff ));
				pData->g = (u_char)(ntstd::Clamp( ((int)(rgba.Y() * 255.5f)), 0, 0xff ));
				pData->b = (u_char)(ntstd::Clamp( ((int)(rgba.Z() * 255.5f)), 0, 0xff ));
			}
			break;

		case GF_ARGB8:
			{
				struct Pixel { u_char b,g,r,a; };
				Pixel* pData = reinterpret_cast<Pixel*>( m_pCurr );
				
				pData->r = (u_char)(ntstd::Clamp( ((int)(rgba.X() * 255.5f)), 0, 0xff ));
				pData->g = (u_char)(ntstd::Clamp( ((int)(rgba.Y() * 255.5f)), 0, 0xff ));
				pData->b = (u_char)(ntstd::Clamp( ((int)(rgba.Z() * 255.5f)), 0, 0xff ));
				pData->a = (u_char)(ntstd::Clamp( ((int)(rgba.W() * 255.5f)), 0, 0xff ));
			}
			break;

		case GF_XRGB8:
			{
				struct Pixel { u_char b,g,r,a; };
				Pixel* pData = reinterpret_cast<Pixel*>( m_pCurr );
				
				pData->r = (u_char)(ntstd::Clamp( ((int)(rgba.X() * 255.5f)), 0, 0xff ));
				pData->g = (u_char)(ntstd::Clamp( ((int)(rgba.Y() * 255.5f)), 0, 0xff ));
				pData->b = (u_char)(ntstd::Clamp( ((int)(rgba.Z() * 255.5f)), 0, 0xff ));
			}
			break;

		case GF_L8:	// using rgba.X() IS CORRECT FOR L8 format!
			{
				struct Pixel { u_char l; };
				Pixel* pData = reinterpret_cast<Pixel*>( m_pCurr );
				pData->l = (u_char)(ntstd::Clamp( ((int)(rgba.X() * 255.5f)), 0, 0xff ));
			}
			break;

		case GF_A8:	// using rgba.W() IS CORRECT FOR A8 format!
			{
				struct Pixel { u_char a; };
				Pixel* pData = reinterpret_cast<Pixel*>( m_pCurr );
				pData->a = (u_char)(ntstd::Clamp( ((int)(rgba.W() * 255.5f)), 0, 0xff ));
			}
			break;

		case GF_L8A8: // using rgba.X() && rgba.W() IS CORRECT FOR L8 format!
			{
				struct Pixel { u_char l, a; };
				Pixel* pData = reinterpret_cast<Pixel*>( m_pCurr );
				pData->l = (u_char)(ntstd::Clamp( ((int)(rgba.X() * 255.5f)), 0, 0xff ));
				pData->a = (u_char)(ntstd::Clamp( ((int)(rgba.W() * 255.5f)), 0, 0xff ));
			}
			break;

		case GF_V8U8: // using rgba.X() && rgba.W() IS CORRECT FOR L8 format!
			{
				struct Pixel { u_char b,g; };
				Pixel* pData = reinterpret_cast<Pixel*>( m_pCurr );
				pData->b = (u_char)(ntstd::Clamp( ((int)(rgba.X() * 255.5f)), 0, 0xff ));
				pData->g = (u_char)(ntstd::Clamp( ((int)(rgba.Y() * 255.5f)), 0, 0xff ));
			}
			break;


		case GF_ABGR16F:
			{
				struct Pixel { D3DXFLOAT16 r,g,b,a; };
				Pixel* pData = reinterpret_cast<Pixel*>( m_pCurr );

				pData->r = rgba.X();
				pData->g = rgba.Y();
				pData->b = rgba.Z();
				pData->a = rgba.W();
			}
			break;

		case GF_ABGR32F:
			{
				struct Pixel { float r,g,b,a; };
				Pixel* pData = reinterpret_cast<Pixel*>( m_pCurr );

				pData->r = rgba.X();
				pData->g = rgba.Y();
				pData->b = rgba.Z();
				pData->a = rgba.W();
			}
			break;

		case GF_R16F:
			{
				struct Pixel { D3DXFLOAT16 r; };
				Pixel* pData = reinterpret_cast<Pixel*>( m_pCurr );
				pData->r = rgba.X();
			}
			break;

		case GF_R32F:
			{
				struct Pixel { float r; };
				Pixel* pData = reinterpret_cast<Pixel*>( m_pCurr );
				pData->r = rgba.X();
			}
			break;
		}
	}

private:
	void*		m_pStart;
	void*		m_pCurr;
	GFXFORMAT	m_fmt;
	int			m_iStride;
};

#endif // _TEX_READER_PC_H
