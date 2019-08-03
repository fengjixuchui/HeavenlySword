//----------------------------------------------------------------------------------------
//! 
//! \filename exec\ppu\spuarmument_ps3.h
//! 
//----------------------------------------------------------------------------------------
#ifndef SPUARGUMENT_PS3_H_
#define SPUARGUMENT_PS3_H_

#include "exec/ppu/dmabuffer_ps3.h"

class Exec;

//
//	A single argument or parameter for an SPU program.
//
class SPUArgument
{
	public:
		//
		//	Enum for type of argument.
		//
		enum Mode
		{
			Mode_InputOnly		= 1,
			Mode_OutputOnly		= 2,
			Mode_InputAndOutput	= 4,

			Mode_Invalid		= 0xffff
		};

		enum Type							/* SPU usage;									*/
		{									/*												*/
			Type_DMABuffer		= 1,		/* Can be Input and Output (or both).			*/
			Type_U64			= 2,		/* InputOnly.									*/
			Type_U32			= 4,		/* InputOnly.									*/
			Type_Float			= 8,		/* InputOnly.									*/

			Type_Invalid		= 0xffff
		};

		static const uint32_t	InvalidBufferSet = 0xffffffff;

	public:
		//
		//	Accessors.
		//
		Mode					GetMode			()	const	{ return (Mode)( ( m_ModeAndType >> 16 ) & 0xffff ); }
		Type					GetType			()	const	{ return (Type)( m_ModeAndType & 0xffff ); }

		DMABuffer *				GetBuffer		()			{ ntError( GetType() == Type_DMABuffer ); return reinterpret_cast< DMABuffer * >( &( m_Buffer[ 0 ] ) ); }
		const DMABuffer *		GetBuffer		()	const	{ ntError( GetType() == Type_DMABuffer ); return reinterpret_cast< const DMABuffer * >( &( m_Buffer[ 0 ] ) ); }

		uint64_t				GetU64			()			{ ntError( GetType() == Type_U64 ); return m_U64; }
		uint32_t				GetU32			()			{ ntError( GetType() == Type_U32 ); return m_U32; }
		float					GetFloat		()			{ ntError( GetType() == Type_Float ); return m_Float; }

		uint32_t				GetBufferSetNum	()	const	{ return m_BufferSet; }

	public:
		//
		//	Ctors, dtor.
		//
		SPUArgument();
		explicit SPUArgument( Mode mode, const DMABuffer &buf );
		explicit SPUArgument( Mode mode /*= SPUArgument::Mode_InputOnly*/, uint64_t param64 );	// Sony says InputOnly.
		explicit SPUArgument( Mode mode /*= SPUArgument::Mode_InputOnly*/, uint32_t param32 );	// Sony says InputOnly.
		explicit SPUArgument( Mode mode /*= SPUArgument::Mode_InputOnly*/, float paramFloat );	// Sony says InputOnly.

	private:
		//
		//	Composited members.
		//
		friend class Exec;	// We'd like Exec to be able to bypass the accessors.

		union
		{
			uint8_t		m_Buffer[ sizeof( DMABuffer ) ];	// Storage space for a DMABuffer.
			uint64_t	m_U64;
			uint32_t	m_U32;
			float		m_Float;
		};

		uint32_t		m_ModeAndType;	// = ( ( mode & 0xffff ) << 16 ) | ( type & 0xffff )

		uint32_t		m_BufferSet;	// The index of the bufferset that this argument belongs to - only relevent for DMABuffers.

} __attribute__ ((aligned( 16 )));

//
//	A set of all the arguments/parameters for an SPU program.
//  These MUST ALWAYS be on DMA-able boundarys and DMAable size 
// (should be taken care automatically of with the aligned attribute)
//
class SPUArgumentList
{
	public:
		//
		//	Constants.
		//
		static const int16_t MaxNumArguments = 8;

	public:
		//
		//	Add argument.
		//
		void				Set	( const SPUArgument &arg, int16_t slot )
		{
			ntError( slot >= 0 && slot < MaxNumArguments );
			m_Arguments[ slot ] = arg;
		}

	public:
		//
		//	Accessors
		//
		SPUArgument *		Get	( int16_t i )
		{
#if !defined( _RELEASE )
			if ( i < 0 || i >= MaxNumArguments )
				return NULL;

			if ( m_Arguments[ i ].GetMode() == SPUArgument::Mode_Invalid )
				return NULL;
#endif
			return &( m_Arguments[ i ] );
		}

		const SPUArgument *	Get	( int16_t i ) const
		{
#if !defined( _RELEASE )
			if ( i < 0 || i >= MaxNumArguments )
				return NULL;

			if ( m_Arguments[ i ].GetMode() == SPUArgument::Mode_Invalid )
				return NULL;
#endif
			return &( m_Arguments[ i ] );
		}

	private:
		//
		//	Composited members.
		//
		SPUArgument			m_Arguments[ MaxNumArguments ];

} __attribute__ ((aligned( 128 )));


//**************************************************************************************
//
// INLINEd basically cos its a pain in the arse to show cpp between ppu and spu 
//	
//**************************************************************************************


//**************************************************************************************
//	
//**************************************************************************************
inline SPUArgument::SPUArgument()
:	m_Buffer			()
,	m_ModeAndType		( ( SPUArgument::Mode_Invalid << 16 ) | SPUArgument::Type_Invalid )
,	m_BufferSet			( SPUArgument::InvalidBufferSet )
{}

//**************************************************************************************
//	
//**************************************************************************************
inline SPUArgument::SPUArgument( SPUArgument::Mode mode, const DMABuffer &buf )
:	m_ModeAndType		( ( mode << 16 ) | SPUArgument::Type_DMABuffer )
,	m_BufferSet			( SPUArgument::InvalidBufferSet )
{
	*reinterpret_cast< DMABuffer * >( &( m_Buffer[ 0 ] ) ) = buf;
}

//**************************************************************************************
//	
//**************************************************************************************
inline SPUArgument::SPUArgument( Mode mode, uint64_t param64 )
:	m_U64				( param64 )
,	m_ModeAndType		( ( mode << 16 ) | SPUArgument::Type_U64 )
,	m_BufferSet			( SPUArgument::InvalidBufferSet )
{
	//
	//	This is here because only DMABuffers can be outputs. Urgh.
	//	I'm sure it's possible to have these as outputs, but the sony libs don't seem to allow it,
	//	maybe it's slow or something?
	// 
	ntError_p( mode == SPUArgument::Mode_InputOnly, ("u64 params can only be inputs.") );
}

//**************************************************************************************
//	
//**************************************************************************************
inline SPUArgument::SPUArgument( Mode mode, uint32_t param32 )
:	m_U32				( param32 )
,	m_ModeAndType		( ( mode << 16 ) | SPUArgument::Type_U32 )
,	m_BufferSet			( SPUArgument::InvalidBufferSet )
{
	//
	//	This is here because only DMABuffers can be outputs. Urgh.
	//	I'm sure it's possible to have these as outputs, but the sony libs don't seem to allow it,
	//	maybe it's slow or something?
	// 
	ntError_p( mode == SPUArgument::Mode_InputOnly, ("u32 params can only be inputs.") );
}

//**************************************************************************************
//	
//**************************************************************************************
inline SPUArgument::SPUArgument( Mode mode, float paramFloat )
:	m_Float				( paramFloat )
,	m_ModeAndType		( ( mode << 16 ) | SPUArgument::Type_Float )
,	m_BufferSet			( SPUArgument::InvalidBufferSet )
{
	//
	//	This is here because only DMABuffers can be outputs. Urgh.
	//	I'm sure it's possible to have these as outputs, but the sony libs don't seem to allow it,
	//	maybe it's slow or something?
	// 
	ntError_p( mode == SPUArgument::Mode_InputOnly, ("float params can only be inputs.") );
}




#endif // !SPUARGUMENT_PS3_H_

