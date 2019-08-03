//------------------------------------------------------
//!
//! \file army/armyrenderpool.h
//! holds a bunch of renderables to be dished out to the
//! approipate grunt for maximum visible fidelity
//!
//------------------------------------------------------

#ifndef ARMYRENDERPOOL_H
#define ARMYRENDERPOOL_H

class ArmyImpostors;

class ArmyRenderPool
{
public:
	ArmyRenderPool( const Battlefield* pBattlefield, const ArmyUnitParameters* pParams, uint32_t iPoolSize, uint32_t iMaxImposters );
	~ArmyRenderPool();

	ArmyRenderable* Allocate();
	void Deallocate( ArmyRenderable* pRenderable );
	bool AnySpareRenderables(); 
    
	void AddBombs( const CKeyString& bombName );

	void ResetSprites();
	void SetSprite( const CPoint& pos, const uint32_t gruntID, const uint8_t iMajorState );

private:
	const Battlefield*			m_pBattlefield;
	const ArmyUnitParameters*	m_pParams;
	const uint32_t				m_iPoolSize;

	ArmyRenderable* m_pArmyRenderables;
	ArmyRenderable* m_pBombRenderables;

	typedef ntstd::List< ArmyRenderable* > FreeList;

	FreeList m_FreeList;

	float*			m_SpriteSeeds;

	ArmyImpostors*	m_pImposters[ MAX_GRUNT_ANIM ];
	float			m_halfHeight;
};

#endif //ARMYRENDERPOOL_H

