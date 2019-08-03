//! -------------------------------------------
//! aidiving.h
//!
//! Diving functionality for AIs avoiding bolts
//!
//! Author: Dario Sancho-Pradel (DSP)
//!
//!--------------------------------------------

#ifndef _AIDIVING_H
#define _AIDIVING_H

class CEntity;
class Object_Projectile;

// ===================================================================
// SDiversData
// ===================================================================

typedef struct _SDiversData
{
	_SDiversData () : pEntAI(NULL), pBolt(NULL), bDiscarded(false) {}

	_SDiversData ( CEntity* pEAI, Object_Projectile* pB )
	{
		pEntAI = pEAI;
		pBolt = pB;
		bDiscarded = false;
	}

	CEntity* pEntAI;
	Object_Projectile* pBolt;
	bool bDiscarded;

} SDiversData;

// ===================================================================
// SBoltData
// ===================================================================

typedef struct _SBoltData
{
	_SBoltData () : pBolt(NULL), fTime(0.0f), fDebugRadius(0.0f) {}

	_SBoltData ( Object_Projectile* pB )
	{
		pBolt = pB;
		fTime = 0.0f;
		fDebugRadius = 0.0f;
	}

	Object_Projectile* pBolt;
	float fTime;
	float fDebugRadius;	

} SBoltData;

// ===================================================================
// SBoltCone
// ===================================================================
typedef struct _SBoltCone
{
	_SBoltCone () : P0(CONSTRUCT_CLEAR), P1(CONSTRUCT_CLEAR), P2(CONSTRUCT_CLEAR), 
					Seg0(CONSTRUCT_CLEAR), Seg1(CONSTRUCT_CLEAR), Seg2(CONSTRUCT_CLEAR),
					Normal0(CONSTRUCT_CLEAR), Normal1(CONSTRUCT_CLEAR), Normal2(CONSTRUCT_CLEAR),
					P0_AI(CONSTRUCT_CLEAR), P1_AI(CONSTRUCT_CLEAR), P2_AI(CONSTRUCT_CLEAR) {}

	_SBoltCone ( const CPoint& AIPos, const CPoint & P0_in, const CPoint & P1_in, const CPoint & P2_in)
	{
		P0 = P0_in; P1 = P1_in; P2 = P2_in;
		Seg0 = CDirection(P1-P0);
		Seg1 = CDirection(P2-P1);
		Seg2 = CDirection(P0-P2);

		Normal0 = CDirection(Seg0.Z(),Seg0.Y(),-Seg0.X());
		Normal1 = CDirection(Seg1.Z(),Seg1.Y(),-Seg1.X());
		Normal2 = CDirection(Seg2.Z(),Seg2.Y(),-Seg2.X());

		P0_AI = CDirection( AIPos - P0 );
		P1_AI = CDirection( AIPos - P1 );
		P2_AI = CDirection( AIPos - P2 );
	}

	CPoint P0; /*       /P1 */
	CPoint P1; /*   P0 /	*/
	CPoint P2; /*      \	*/
	           /*       \P2	*/

	CDirection Seg0;
	CDirection Seg1;
	CDirection Seg2;

	CDirection Normal0;
	CDirection Normal1;
	CDirection Normal2;

	CDirection P0_AI;
	CDirection P1_AI;
	CDirection P2_AI;



} SBoltCone;

typedef ntstd::List<SDiversData*, Mem::MC_AI> ListDiversData;
typedef ntstd::List<SBoltData*, Mem::MC_AI> ListBoltsData;

// ===================================================================
// CAIDivingMan
// ===================================================================

class CAIDivingMan
{
	public:
		CAIDivingMan();
		~CAIDivingMan();

		void	AddBolt				(Object_Projectile* pBolt);
		void	RemoveBolt			(Object_Projectile* pBolt);
		bool	IsInTheDiversList	( CEntity*, bool* );
		void	RemoveDiver			( CEntity* );
		void	DisregardCurrentBolt( CEntity* );

		unsigned int GetDivingSideToAvoidBolt ( CEntity* ); 
		
		void	SetDebugDiveCone ( float fL, float fHR ) { m_fDebugConeLength = fL; m_fDebugConeHalfRadius = fHR; }
		void	DebugRender	( void );
		void	Update ( float );

	private:
		
		void ClearDiversDataWithBolt	( Object_Projectile* );
		void ManageDiversData			( CEntity*, Object_Projectile*, bool );

	private:
	
		ListBoltsData							m_listBoltsData;
		ntstd::Vector<SDiversData*, Mem::MC_AI>	m_vDiversData;
//		float									m_fTimer;
		SBoltCone								m_BoltCone;
		float									m_fDebugConeLength;
		float									m_fDebugConeHalfRadius;
};


#endif // _AIDIVING_H






