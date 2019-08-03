//------------------------------------------------------
//!
//!	\file army/generaldef.cpp
//! This contains the setup and sector specific stuff for army chapter 1 section 1
//!
//------------------------------------------------------

#ifndef ARMY_CHAP5_4_H_
#define ARMY_CHAP5_4_H_

//------------------------------------------------------
//	Includes files.
//------------------------------------------------------
#include "army/army_section.h"

//------------------------------------------------------
//	Forward declarations.
//------------------------------------------------------
struct General;
struct Battlefield;
struct BattlefieldHeader;
struct Battalion;
struct Grunt;
struct Unit;
struct HeightfieldHeader;
class ArmyBattlefield;
class ArmyBattalion;
class CEntity;

//------------------------------------------------------
//!
//! A class containing all the setup and updates for the
//! army section in Chapter 5 section 4
//!
//------------------------------------------------------
class ArmyChap5_4 : public ArmySection
{
public:
	// note these pointer may be deleted afterwards!
	ArmyChap5_4( const ArmyBattlefield* pArmyArena );
	virtual ~ArmyChap5_4();

	virtual void Update( float fTimeStep );
	virtual void Render();
	virtual void ProcessGlobalEvent( const CHashedString obGlobalEvent );

	static const int MAX_GRUNTS_ON_THE_BATTLEFIELD = 2000;

private:
	void SectionConstruct();

	bool m_bDoneConstruct;

	// the bulk are single unit squares
	void Create_fodder_square_36( const ArmyBattalion* pBattalionDef, 
						const BattlefieldHeader* pBattlefieldHeader, 
						Battalion* pBattalion, TempGruntVector& grunts );
	void Create_shieldsmen_square_36( const ArmyBattalion* pBattalionDef, 
						const BattlefieldHeader* pBattlefieldHeader, 
						Battalion* pBattalion, TempGruntVector& grunts );
	void Create_axemen_square_36( const ArmyBattalion* pBattalionDef, 
						const BattlefieldHeader* pBattlefieldHeader, 
						Battalion* pBattalion, TempGruntVector& grunts );

	void Create_fodder_square_36_Axemen( const ArmyBattalion* pBattalionDef, 
						const BattlefieldHeader* pBattlefieldHeader, 
						Battalion* pBattalion, TempGruntVector& grunts );
	void Create_shieldsmen_square_36_Axemen( const ArmyBattalion* pBattalionDef, 
						const BattlefieldHeader* pBattlefieldHeader, 
						Battalion* pBattalion, TempGruntVector& grunts );

	void Create_fodder_square_36_DoubleAxemen( const ArmyBattalion* pBattalionDef, 
						const BattlefieldHeader* pBattlefieldHeader, 
						Battalion* pBattalion, TempGruntVector& grunts );
	void Create_shieldsmen_square_36_DoubleAxemen( const ArmyBattalion* pBattalionDef, 
						const BattlefieldHeader* pBattlefieldHeader, 
						Battalion* pBattalion, TempGruntVector& grunts );


	void Create_mixed_rectangle_40( const ArmyBattalion* pBattalionDef, 
						const BattlefieldHeader* pBattlefieldHeader, 
						Battalion* pBattalion, TempGruntVector& grunts );
	void Create_shieldman_square_64( const ArmyBattalion* pBattalionDef, 
						const BattlefieldHeader* pBattlefieldHeader, 
						Battalion* pBattalion, TempGruntVector& grunts );
	void Create_lillys_big_one(  const ArmyBattalion* pBattalionDef, 
						const BattlefieldHeader* pBattlefieldHeader, 
						Battalion* pBattalion, TempGruntVector& grunts );
	void Create_small_flag_square(  const ArmyBattalion* pBattalionDef, 
						const BattlefieldHeader* pBattlefieldHeader, 
						Battalion* pBattalion, TempGruntVector& grunts );

	// bespoke code stuff
	CEntity* m_pCatapult[3];
	int32_t	m_iCatapultObstacleIndex[3];

	bool m_bCharge;
	bool m_bCatapultTriggered;
	float m_fGameTimeInSecs;

	CDirection			m_vBoomOffset;

};
#endif	// !ARMY_CHAP5_4_H_
