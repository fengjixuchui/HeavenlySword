//------------------------------------------------------------------------------------------
//!                                                                                         
//!	\file airepulsion.h
//!                                                                                         
//------------------------------------------------------------------------------------------

#ifndef _AIREPULSION_H
#define _AIREPULSION_H

class CEntity;
class AvoidPoint;
class CSphereBound;

//------------------------------------------------------------------------------------------
//!                                                                                         
//!	AIObjectAvoidance::AIObjectAvoidance
//!                                                                                         
//! handles inter-object repulsion forces to reduce the	possibility of AIs
//! colliding with dynamic objects
//!                                                                                         
//------------------------------------------------------------------------------------------

static const int NUM_REPELLERS = 256;

class AIObjectAvoidance
{
public:
	AIObjectAvoidance() : m_iNumEnts(0), m_iNumPoints(0)	{ for (int i = 0; i < NUM_REPELLERS; i++) { m_bStatic[i] = false; m_bDontAvoid[i] = false; } }
	~AIObjectAvoidance()							{};

	void	FirstFrameInit(); 
	void	Update( float fTimeStep ); 

	int		AddEntity( const CEntity* pobEntity, float radius = 1.0f );
	void	RemoveEntity( const CEntity* pobEntity );
	int		AddAvoidPoint( const AvoidPoint* point );
	void	Clear()													{ m_iNumEnts = 0; m_iNumPoints = 0; }

	CPoint	MakeForce( const CEntity* pobAI, const int iNum, const CPoint& obPos, const CPoint& obHeading, float fTimeStep );
	int		GetNumRepellers() const;
	CPoint	GetPos( const int iNum ) const;

	bool	IsObstructed( const CPoint& obPos, const float fRadius, CSphereBound* pResult = 0 ) const;
	bool	IsObstructed( const CPoint& from, const CPoint& to, const float fRadius, CSphereBound* pResult = 0 ) const;

	void	SetStatic( const int iNum, bool bStatic )		{ m_bStatic[iNum] = bStatic; }

private:
	int					m_iNumEnts;
	int					m_iNumPoints;
	const CEntity*		m_pobEntities[NUM_REPELLERS];
	const AvoidPoint*	m_pobPoints[NUM_REPELLERS];
	float				m_fRadii[NUM_REPELLERS];
	bool				m_bStatic[NUM_REPELLERS];
	bool				m_bDontAvoid[NUM_REPELLERS];

	float		GetAngle( const CPoint& obVec1, const CPoint& obVec2 );
	float		GetAngle( const CDirection& obVec1, const CPoint& obVec2 );
};

#endif
