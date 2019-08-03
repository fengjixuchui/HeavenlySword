//--------------------------------------------------------------------------------------------------
/**
	@file		

	@brief		

	@note		(c) Copyright Sony Computer Entertainment 2005. All Rights Reserved.	
**/
//--------------------------------------------------------------------------------------------------

#ifndef GC_STREAM_FIELD_H
#define GC_STREAM_FIELD_H

#include	<Fw/FwStd/FwHashedString.h>
#include	<Gc/Gc.h>

//--------------------------------------------------------------------------------------------------
/**
	@class			GcStreamField
	
	@brief			
**/
//--------------------------------------------------------------------------------------------------

class GcStreamField
{
public:
	GcStreamField()
	{
		m_data.m_packedInfo	= 0;
	}


	GcStreamField( FwHashedString name, int offset, Gc::StreamType type, int size, bool normalised = false )
	{
		m_name						= name;
		m_data.m_state.m_offset		= ( u8 )offset;
		m_data.m_state.m_type		= ( u8 )type;
		m_data.m_state.m_size		= ( u8 )size;
		m_data.m_state.m_normalised	= ( u8 )normalised;
	}

	FwHashedString	GetName( void ) const		{ return m_name; }

	int				GetOffset( void ) const		{ return ( int )m_data.m_state.m_offset; }
	Gc::StreamType	GetType( void ) const		{ return ( Gc::StreamType )m_data.m_state.m_type; }
	int				GetSize( void ) const		{ return ( int )m_data.m_state.m_size; }
	bool			GetNormalised( void ) const	{ return ( bool )m_data.m_state.m_normalised; }

	// Used when writing out in binary structures..
	u32				GetPackedInfo( void ) const	{ return m_data.m_packedInfo; }

private:
	FwHashedString	m_name;

	union
	{
		u32			m_packedInfo;
		
		struct	FieldInfo
		{
			u8		m_offset;
			u8		m_type;
			u8		m_size;
			u8		m_normalised;
		} m_state;

	} m_data;
};

#endif	//GC_STREAM_FIELD_H
