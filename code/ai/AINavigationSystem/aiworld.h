//! -------------------------------------------
//! AIWorld.h
//!
//! World description for AIs
//!
//! Author: Dario Sancho-Pradel (DSP)
//!
//!--------------------------------------------

#ifndef _AIWOLRD_H
#define _AIWOLRD_H

#include "aiworldvolumes.h"

enum ENUM_DIVE_DIR
{
	E_DIVE_LEFT = 0,
	E_DIVE_RIGHT = 1,
};

enum ENUM_DIVE_FULL_DIRECTION
{
	E_DONT_DIVE			= 0,
	E_DIVE_LONG_LEFT	= 1,
	E_DIVE_LONG_RIGHT	= 1<<1,
	E_DIVE_SHORT_LEFT	= 1<<2,
	E_DIVE_SHORT_RIGHT	= 1<<3,
	E_DIVE_PANIC		= 1<<4,
	E_DIVE_CROUCH		= 1<<5,
};

class CEntity;

class CAIWorldMan
{
	public:
		
		CAIWorldMan();

		void	AddVolume			(CAIWorldVolume* pCV) { if (pCV) m_listpVolumes.push_back(pCV); }
		void	RemoveVolume		(CAIWorldVolume* pCV);
		void	AddGoAroundVolume	(CAIWorldVolume* pCV) { if (pCV) m_listpGoAroundVolumes.push_back(pCV); }
		void	AddVaultingVolume	(CAIWorldVolume* pCV) { if (pCV) m_listpVaultingVolumes.push_back(pCV); }
		void	AddContArea			(CAIWorldVolume* pCV) { if (pCV) m_listpContAreas.push_back(pCV); }
		
		bool	IntersectsAIVolume		( const CPoint&, const CPoint&, CPoint*, CDirection* ) const;
		bool	HasLineOfSight			( const CPoint&, const CPoint&, float = 0.0f ) const;
		bool	HasShootingLineOfSight	( const CPoint&, const CPoint&, float = 0.0f ) const;
		
		bool	HasLineOfSightExcludingGoAroundVolumes	( const CPoint&, const CPoint& ) const;
		bool	HasLineOfSightThroughGoAroundVolumes	( const CPoint&, const CPoint& ) const;
		
		CAIWorldVolume*			GetClosestGoAroundVolumeOnTheWay	( const CPoint&, const CPoint& ) const;
		const CAIWorldVolume*	IntersectsVaultingVolume			( const CPoint&, const CPoint&, CPoint* , CDirection* ) const;
		const CAIWorldVolume*	CloseToVaultingVolume				( const CPoint&, float fRadius ) const;

		bool					SetCentre							( CAIWorldVolume* pCV, const CPoint& obPos ) { return (pCV ? pCV->SetCentre(obPos) : false); }
		CAIWorldVolume*			GetVolumeByName						( const char* );

		void	DebugRender	( void );

		CEntity*		TestDive_AICollision		( CEntity* , ENUM_DIVE_FULL_DIRECTION );
		bool			TestDive_NoWallCollision	( CEntity* , ENUM_DIVE_FULL_DIRECTION );

	private:
	
		AIWorldVolumeList	m_listpVolumes;
		AIWorldVolumeList	m_listpContAreas;
		AIWorldVolumeList	m_listpGoAroundVolumes;
		AIWorldVolumeList	m_listpVaultingVolumes;

		SAIVolSegment		m_Seg[4];

};


#endif // _AIWOLRD_H



