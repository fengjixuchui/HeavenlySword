//------------------------------------------------------
//!
//!	\file army/generaldef.cpp
//! This contains the setup and sector specific stuff for army chapter 1 section 1
//!
//------------------------------------------------------

#ifndef ARMY_CHAP5_3_H_
#define ARMY_CHAP5_3_H_

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

//------------------------------------------------------
//!
//! A class containing all the setup and updates for the
//! army section in Chapter 1 section 1
//!
//------------------------------------------------------
class ArmyChap5_3 : public ArmySection
{
public:
	// note these pointer may be deleted afterwards!
	ArmyChap5_3( const ArmyBattlefield* pArmyArena );
	virtual ~ArmyChap5_3();

	virtual void Update( float fTimeStep );
	virtual void Render();
	virtual void ProcessGlobalEvent( const CHashedString obGlobalEvent );

	static const int MAX_GRUNTS_ON_THE_BATTLEFIELD = 2000;

private:
	void SectionConstruct();

	bool m_bDoneConstruct;

	// simple battalion formation to get us thro alpha as fast as possible
	void CreateSmallSquare( const ArmyBattalion* pBattalionDef, 
						const BattlefieldHeader* pBattlefieldHeader, 
						Battalion* pBattalion, TempGruntVector& grunts );
	void CreateMediumSquare( const ArmyBattalion* pBattalionDef, 
						const BattlefieldHeader* pBattlefieldHeader, 
						Battalion* pBattalion, TempGruntVector& grunts );
	void CreateLargeSquare( const ArmyBattalion* pBattalionDef, 
							const BattlefieldHeader* pBattlefieldHeader, 
							Battalion* pBattalion, TempGruntVector& grunts );

	void CreateSmallRectangle( const ArmyBattalion* pBattalionDef, 
						const BattlefieldHeader* pBattlefieldHeader, 
						Battalion* pBattalion, TempGruntVector& grunts );
	void CreateMediumRectangle( const ArmyBattalion* pBattalionDef, 
						const BattlefieldHeader* pBattlefieldHeader, 
						Battalion* pBattalion, TempGruntVector& grunts );
	void CreateLargeRectangle( const ArmyBattalion* pBattalionDef, 
							const BattlefieldHeader* pBattlefieldHeader, 
							Battalion* pBattalion, TempGruntVector& grunts );

};
#endif	// !ARMY_CHAP5_3_H_
