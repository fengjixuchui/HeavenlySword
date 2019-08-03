//--------------------------------------------------
//!
//!	\file sectordata.h
//!	XML definitions used in area system
//!
//--------------------------------------------------

#ifndef AREA_DATA_H
#define AREA_DATA_H

namespace AreaSystem
{
	//--------------------------------------------------
	//!
	//!	BoxVolume
	//! Simple box volume with an interior test
	//!
	//--------------------------------------------------
	class BoxVolume
	{
	public:
		bool Inside( const CPoint& position );

	protected:
		BoxVolume();

		void SetVolume( const CPoint& pos, const CQuat& rot, const CPoint& size );
		void Render( uint32_t normalCol, uint32_t insideCol ) const;

	private:
		bool m_bInvalid;
		bool m_bDirty;
		bool m_bLastInside;

		CMatrix	m_localToWorld;
		CMatrix	m_worldToLocal;
	};
};

//--------------------------------------------------
//!
//!	SectorLoadTrigger
//! Serialised class that represents a load trigger
//!
//--------------------------------------------------
class SectorLoadTrigger : public AreaSystem::BoxVolume
{
public:
	void PostConstruct();
	bool EditorChangeValue( CallBackParameter, CallBackParameter );
	void DebugRender();

	CPoint		m_position;
	CQuat		m_orientation;
	CPoint		m_scale;
	int32_t		m_iAreaToLoad;
	CHashedString	m_unused;
};

//--------------------------------------------------
//!
//!	SectorTransitionPortal
//! Serialised class that represents a sector portal
//!
//--------------------------------------------------
class SectorTransitionPortal : public AreaSystem::BoxVolume
{
public:
	void PostConstruct();
	bool EditorChangeValue( CallBackParameter, CallBackParameter );
	void DebugRender();

	CPoint		m_position;
	CQuat		m_orientation;
	CPoint		m_scale;
	int32_t		m_iAreaToActivate;
	CHashedString	m_unused;
};

#endif
