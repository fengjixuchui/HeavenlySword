//--------------------------------------------------
//!
//!	\file areadata.cpp
//!	XML structs used in area system
//!
//--------------------------------------------------

#include "area/areasystem.h"
#include "area/areadata.h"

#include "objectdatabase/dataobject.h"
#include "core/visualdebugger.h"

using namespace AreaSystem;

//--------------------------------------------------
//!
//!	Interfaces
//!
//--------------------------------------------------
START_STD_INTERFACE	( SectorLoadTrigger )
	PUBLISH_VAR_WITH_DEFAULT_AS( m_position, CPoint(0.0f, 0.0f, 0.0f), Position )
	PUBLISH_VAR_WITH_DEFAULT_AS( m_orientation,	CQuat(0.0f, 0.0f, 0.0f, 1.0f), Orientation )
	PUBLISH_VAR_WITH_DEFAULT_AS( m_scale, CPoint(1.0f, 1.0f, 1.0f), Dimension )
	PUBLISH_VAR_WITH_DEFAULT_AS( m_iAreaToLoad, -1, SectorToLoad )
	DECLARE_POSTCONSTRUCT_CALLBACK(	PostConstruct )
	DECLARE_EDITORCHANGEVALUE_CALLBACK( EditorChangeValue )
	DECLARE_DEBUGRENDER_FROMNET_CALLBACK( DebugRender )
END_STD_INTERFACE

START_STD_INTERFACE	( SectorTransitionPortal )
	PUBLISH_VAR_WITH_DEFAULT_AS( m_position, CPoint(0.0f, 0.0f, 0.0f), Position )
	PUBLISH_VAR_WITH_DEFAULT_AS( m_orientation, CQuat(0.0f, 0.0f, 0.0f, 1.0f), Orientation )
	PUBLISH_VAR_WITH_DEFAULT_AS( m_scale, CPoint(1.0f, 1.0f, 1.0f), Dimension )
	PUBLISH_VAR_WITH_DEFAULT_AS( m_iAreaToActivate,	-1, SectorToActivate )
	DECLARE_POSTCONSTRUCT_CALLBACK(			PostConstruct )
	DECLARE_EDITORCHANGEVALUE_CALLBACK(		EditorChangeValue )
	DECLARE_DEBUGRENDER_FROMNET_CALLBACK(	DebugRender )
END_STD_INTERFACE

//--------------------------------------------------
//!
//!	SectorLoadTrigger::PostConstruct
//!
//--------------------------------------------------
void SectorLoadTrigger::PostConstruct()
{
	// we should already have defaults provided by serialiser
	// and MrED.

	AreaManager::Get().AddLoadTrigger( this );	

	// setup the actual volume
	SetVolume( m_position, m_orientation, m_scale );
}

//--------------------------------------------------
//!
//!	SectorLoadTrigger::EditorChangeValue
//!
//--------------------------------------------------
bool SectorLoadTrigger::EditorChangeValue( CallBackParameter, CallBackParameter )
{
	// setup the actual volume
	SetVolume( m_position, m_orientation, m_scale );
	return true;
}

//--------------------------------------------------
//!
//!	SectorLoadTrigger::EditorChangeValue
//!
//--------------------------------------------------
void SectorLoadTrigger::DebugRender()
{
#ifndef _GOLD_MASTER
	Render( 0xff0000ff, 0xff8080ff );

	DataObject* pDO = ObjectDatabase::Get().GetDataObjectFromPointer( this );
	ntAssert( pDO );

	const char* pName = ntStr::GetString(pDO->GetName());
	g_VisualDebug->Printf3D( m_position, DC_WHITE, 0, "%s: Loads area %d.", pName, m_iAreaToLoad );
#endif
}




//--------------------------------------------------
//!
//!	SectorTransitionPortal::PostConstruct
//!
//--------------------------------------------------
void SectorTransitionPortal::PostConstruct()
{
	// we should already have defaults provided by serialiser
	// and MrED.

	AreaManager::Get().AddTransitionPortal( this );	
	
	// setup the actual volume
	SetVolume( m_position, m_orientation, m_scale );
}

//--------------------------------------------------
//!
//!	SectorTransitionPortal::EditorChangeValue
//!
//--------------------------------------------------
bool SectorTransitionPortal::EditorChangeValue( CallBackParameter, CallBackParameter )
{
	// setup the actual volume
	SetVolume( m_position, m_orientation, m_scale );
	return true;
}

//--------------------------------------------------
//!
//!	SectorTransitionPortal::EditorChangeValue
//!
//--------------------------------------------------
void SectorTransitionPortal::DebugRender()
{
#ifndef _GOLD_MASTER
	Render( 0xffff0000, 0xffff8080 );

	DataObject* pDO = ObjectDatabase::Get().GetDataObjectFromPointer( this );
	ntAssert( pDO );

	const char* pName = ntStr::GetString(pDO->GetName());
	g_VisualDebug->Printf3D( m_position, DC_WHITE, 0, "%s: Activates area %d.", pName, m_iAreaToActivate );
#endif
}




//--------------------------------------------------
//!
//!	BoxVolume::ctor
//! Simple box volume with an interior test
//!
//--------------------------------------------------
BoxVolume::BoxVolume() :
	m_bInvalid( true ),
	m_bDirty( true ),
	m_bLastInside( false )
{}

//--------------------------------------------------
//!
//!	BoxVolume::BoxVolume
//! setup our volume for test
//!
//--------------------------------------------------
void BoxVolume::SetVolume( const CPoint& pos, const CQuat& rot, const CPoint& size )
{
	m_bInvalid = false;
	m_bDirty = true;

	CMatrix mat = CMatrix( rot, pos );		
	CMatrix scale(	size.X() * 0.5f, 0.0f, 0.0f, 0.0f,
					0.0f, size.Y() * 0.5f, 0.0f, 0.0f,
					0.0f, 0.0f, size.Z() * 0.5f, 0.0f,
					0.0f, 0.0f, 0.0f, 1.0f);

	m_localToWorld = scale * mat;
}

//--------------------------------------------------
//!
//!	BoxVolume::Inside
//!	See if point is inside volume
//!
//--------------------------------------------------
bool BoxVolume::Inside( const CPoint& position )
{
	if ( m_bDirty )
	{
		m_worldToLocal = m_localToWorld.GetFullInverse();
		m_bDirty = false;
	}
	
	CPoint local = position * m_worldToLocal;

	if	(
		(local.X() < -1.0f) || (local.X() > 1.0f) ||
		(local.Y() < -1.0f) || (local.Y() > 1.0f) ||
		(local.Z() < -1.0f) || (local.Z() > 1.0f)
		)
	{
		m_bLastInside = false;
		return false;
	}

	m_bLastInside = true;
	return true;
}

//--------------------------------------------------
//!
//!	BoxVolume::Render
//!	
//!
//--------------------------------------------------
void BoxVolume::Render( uint32_t normalCol, uint32_t insideCol ) const
{
#ifndef _GOLD_MASTER
	CPoint aCorners[] = 
	{
		CPoint(-1.0f, -1.0f,  1.0f),
		CPoint(-1.0f,  1.0f,  1.0f),
		CPoint(1.0f,  1.0f,  1.0f),
		CPoint(1.0f, -1.0f,  1.0f),
		CPoint(-1.0f, -1.0f, -1.0f),
		CPoint(-1.0f,  1.0f, -1.0f),
		CPoint(1.0f,  1.0f, -1.0f),
		CPoint(1.0f, -1.0f, -1.0f)
	};

	for(int i = 0; i < 8; i++)
		aCorners[i] = aCorners[i] * m_localToWorld;

	uint32_t colour = m_bLastInside ? insideCol : normalCol;

	for(int i = 0; i < 4; i++)
	{
		int iWrapped = i + 1; if(iWrapped == 4) iWrapped = 0;

		g_VisualDebug->RenderLine( aCorners[i],		aCorners[iWrapped], colour, 0 );
		g_VisualDebug->RenderLine( aCorners[i+4],	aCorners[iWrapped+4], colour, 0 );
		g_VisualDebug->RenderLine( aCorners[i],		aCorners[i+4], colour, 0 );
	}
#endif
}
