//--------------------------------------------------
//!
//!	\file game/entitycharacter.h
//!	Definition of the Character entity object
//!
//--------------------------------------------------

#ifndef	_WEAPONS_INC
#define	_WEAPONS_INC


//!----------------------------------------------------------------------------------------------
//! Forward Declarations
//!----------------------------------------------------------------------------------------------
class Character;
class Hero;
class Archer;
class CEntity;
class Spawnee_Def;

//!----------------------------------------------------------------------------------------------
//!
//! WeaponDef
//! Definition for a particular weapon
//!
//!----------------------------------------------------------------------------------------------
class WeaponDef
{
public:
	virtual ~WeaponDef() {;}
	
	virtual CEntity* Create(Character* pCharacter, ntstd::String sName) = 0;        // Standard Version
	virtual CEntity* Create(Character* pCharacter, CEntity* pWeapon) = 0;           // Spawning Version
	virtual CEntity* Create(ntstd::String sName, const Spawnee_Def* pSpawnDef) = 0; // Pre-create for Spawning

	ntstd::String m_sClump;
	ntstd::String m_sParentTransform;
	CPoint        m_ptPosition;
	CPoint        m_ptYPR;
	ntstd::String m_sSheathTransform;
	CPoint        m_ptSheathedPosition;
	CPoint        m_ptSheathedYPR;
};


//!----------------------------------------------------------------------------------------------
//!
//! ThrownWeaponDef
//! Definition for a particular thrown type weapon
//!
//!----------------------------------------------------------------------------------------------
class ThrownWeaponDef : public WeaponDef
{
public:

	virtual CEntity* Create(Character* pCharacter, ntstd::String sName);        // Standard Version
	virtual CEntity* Create(Character* pCharacter, CEntity* pWeapon);           // Spawning Version
	virtual CEntity* Create(ntstd::String sName, const Spawnee_Def* pSpawnDef); // Pre-create for Spawning

protected:
	HAS_INTERFACE(ThrownWeaponDef)

	ntstd::String m_sSharedAttributes;
};


//!----------------------------------------------------------------------------------------------
//!
//! RangedWeaponDef
//! Definition for a particular ranged type weapon
//!
//!----------------------------------------------------------------------------------------------
class RangedWeaponDef : public WeaponDef
{
public:
	virtual CEntity* Create(Character* pCharacter, ntstd::String sName);        // Standard Version
	virtual CEntity* Create(Character* pCharacter, CEntity* pWeapon);           // Spawning Version
	virtual CEntity* Create(ntstd::String sName, const Spawnee_Def* pSpawnDef); // Pre-create for Spawning

protected:
	HAS_INTERFACE(RangedWeaponDef)

	ntstd::String m_sSharedAttributes;
};


//!----------------------------------------------------------------------------------------------
//!
//! HeroWeaponDef
//! Definition for a particular hero type weapon
//!
//!----------------------------------------------------------------------------------------------
class HeroStandardWeaponDef : public WeaponDef
{
public:
	virtual CEntity* Create(Character* pCharacter, ntstd::String sName);                    // Standard Version
	virtual CEntity* Create(Character*, CEntity*)        {ntAssert(false); return 0;}       // Not used
	virtual CEntity* Create(ntstd::String, const Spawnee_Def*) {ntAssert(false); return 0;} // Not used

	CHashedString GetSheathTransform()  {return CHashedString(m_sSheathTransform);}
	CPoint GetSheathedPosition() {return m_ptSheathedPosition;}
	CPoint GetSheathedYPR()      {return m_ptSheathedYPR;}

protected:
	HAS_INTERFACE(HeroStandardWeaponDef);

	ntstd::String m_sClass;
	ntstd::String m_sSheathTransform;
	CPoint        m_ptSheathedPosition;
	CPoint        m_ptSheathedYPR;
};


//!----------------------------------------------------------------------------------------------
//!
//! HeroRangeHandleWeaponDef
//! Definition for a particular hero type weapon
//!
//!----------------------------------------------------------------------------------------------
class HeroRangedWeaponDef : public WeaponDef
{
public:
	virtual CEntity* Create(Character*, ntstd::String )                 {ntAssert(false); return 0;}; // Not used
	virtual CEntity* Create(Character* pCharacter, ntstd::String sName, bool bHandle);
	virtual CEntity* Create(Character*, CEntity*)                       {ntAssert(false); return 0;}  // Not used
	virtual CEntity* Create(ntstd::String, const Spawnee_Def*)          {ntAssert(false); return 0;}  // Not used

protected:
	HAS_INTERFACE(HeroRangedWeaponDef);

	ntstd::String m_sClass;
	ntstd::String m_sHandleClump;
};


//!----------------------------------------------------------------------------------------------
//!
//! HeroRangeHandleWeaponDef
//! Definition for a particular hero type weapon
//!
//!----------------------------------------------------------------------------------------------
class ArcherWeaponDef : public WeaponDef
{
public:
	virtual CEntity* Create(Character* pCharacter, ntstd::String sName);
	virtual CEntity* Create(Character*, CEntity*)                       {ntAssert(false); return 0;} // Not used
	virtual CEntity* Create(ntstd::String, const Spawnee_Def*)          {ntAssert(false); return 0;} // Not used

protected:
	HAS_INTERFACE(ArcherWeaponDef);

	ntstd::String m_sAnimContainer;
	ntstd::String m_sSharedAttributes;
};


//!----------------------------------------------------------------------------------------------
//!
//! WeaponSetDef
//! Definition for a characters weapons
//!
//!----------------------------------------------------------------------------------------------
class WeaponSetDef
{
public:
	virtual ~WeaponSetDef() {;}

	virtual bool CreateWeapons(Character* pCharacter) const  = 0;                                                   // Standard Version
	virtual bool CreateWeapons(Character* pCharacter, const ntstd::Vector<CEntity*>& vecWeapons) const  = 0;        // Spawning Version
	virtual ntstd::Vector<CEntity*> PreCreateWeapons(ntstd::String sName, const Spawnee_Def* pSpawnDef) const  = 0; // Pre-create for spawning

	virtual bool IsBasicWeaponSet() const {return false;}
};


//!----------------------------------------------------------------------------------------------
//!
//! BasicWeaponSetDef
//! Definition for a characters weapons
//!
//!----------------------------------------------------------------------------------------------
class BasicWeaponSetDef : public WeaponSetDef
{
public:
	//BasicWeaponsSetDef() : m_pLeftWeapon(0), m_pRightWeapon(0), m_pRangedWeapon(0) {;}
	virtual ~BasicWeaponSetDef() {;}
	
	virtual bool CreateWeapons(Character* pCharacter) const;                                                   // Standard Version
	virtual bool CreateWeapons(Character* pCharacter, const ntstd::Vector<CEntity*>& vecWeapons) const;        // Spawning Version
	virtual ntstd::Vector<CEntity*> PreCreateWeapons(ntstd::String sName, const Spawnee_Def* pSpawnDef) const; // Pre-create for spawning

	virtual bool IsBasicWeaponSet() const {return true;}

	HAS_INTERFACE(BasicWeaponSetDef)
	WeaponDef* m_pLeftWeapon;
	WeaponDef* m_pRightWeapon;
	WeaponDef* m_pRangedWeapon;
};

//!----------------------------------------------------------------------------------------------
//!
//! HeroWeaponSetDef
//! Specialised version for the Hero
//!
//!----------------------------------------------------------------------------------------------
class HeroWeaponSetDef : public WeaponSetDef
{
public:
	HeroWeaponSetDef() : m_pTechniqueLeft(0), m_pTechniqueRight(0), m_pPower(0), m_pRangeLeft(0), m_pRangeRight(0), m_pBasic(0) {;}
	virtual ~HeroWeaponSetDef() {;}

	virtual bool CreateWeapons(Character* pCharacter) const;
	virtual bool CreateWeapons(Character*, const ntstd::Vector<CEntity*>&) const {ntAssert(false); return false;} // We don't support spawning heros
	virtual ntstd::Vector<CEntity*> PreCreateWeapons(ntstd::String, const Spawnee_Def*) const {ntAssert(false); return ntstd::Vector<CEntity*>();} // We don't support spawning heros

private:
	static void Create_SwordChainEffect(CEntity* pBlade, Hero* pHero, CHashedString transform);

private:
	HAS_INTERFACE(HeroWeaponSetDef)
	HeroStandardWeaponDef* m_pTechniqueLeft;
	HeroStandardWeaponDef* m_pTechniqueRight;
	HeroStandardWeaponDef* m_pPower;
	HeroRangedWeaponDef*   m_pRangeLeft;
	HeroRangedWeaponDef*   m_pRangeRight;
	HeroStandardWeaponDef* m_pBasic;
};

#endif //_WEAPONS_INC
