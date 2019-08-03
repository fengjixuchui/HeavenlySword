/***************************************************************************************************
*
*	DESCRIPTION		Collection of classes for tweaking renderer properties during a special
*
*	NOTES		
*
***************************************************************************************************/

#ifndef _EFFECT_SPECIAL_H
#define _EFFECT_SPECIAL_H

class CEffectSpecialDef;
class CEffectSlowdownSpecial;
class CEffectKillingSpecial;
class CEntity;
class CEntityQuery;

enum EFFECT_SPECIAL_TYPE
{
	SLOWDOWN_SPECIAL,
	KILLING_SPECIAL,
};

/***************************************************************************************************
*
*	CLASS			CEffectSpecial
*
*	DESCRIPTION		pure virtual base class that differing types of specials are based on.
*
***************************************************************************************************/
class CEffectSpecial
{
public:
	CEffectSpecial( const CEntity* pobParent );
	virtual ~CEffectSpecial( void );
	void Cleanup( void );

	virtual bool Update( float fTimeStep, bool bActive ) = 0;
	static CEffectSpecial* CreateNewSpecial( const CEntity* pobParent, EFFECT_SPECIAL_TYPE eType = KILLING_SPECIAL );

	// A Static list for reseting - required for debud rendering settings
	static ntstd::List< CEffectSpecial*, Mem::MC_EFFECTS >* m_pobRegisteredSpecials;

	// Resets all the special effects held in the static list
	static void Clear( void );
	static void Build( void );

protected:
	
	// add clase result
	void AddResultToProdded(CEntityQuery& eq);
	
	// Construct the list of bodies to prod
	void	BuildProddedEntities( void );

	static	void			SetPerEntityEvilness( CEntity* pobVictim );
	static	void			ClearPerEntityEvilness( CEntity* );
	static	void			Lerp4Floats( float* pobResult, const float* pobSrc, const CVector& obDest, float fLerp );

	void	UpdatePerEntityEvilness( CEntity* pobVictim, float fIntensity, EFFECT_SPECIAL_TYPE eType );

	const CEntity*				m_pobParent;
	const CEffectSpecialDef*	m_pobDef;
	bool						m_bOwnsDef;

	bool						m_bInvalid;
	bool						m_bWasActive;
	float						m_fTimeInEffect;
	float						m_fCurrLerpValue;
	float						m_fLerpAtSwitch;

	ntstd::List<CEntity*>				m_obProddedEntities;
};

/***************************************************************************************************
*
*	CLASS			CEffectSlowdownSpecial
*
*	DESCRIPTION		special effect dedicated to slowdown special move...
*
***************************************************************************************************/
class CEffectSlowdownSpecial : public CEffectSpecial
{
public:
	CEffectSlowdownSpecial( const CEntity* pobParent ) : CEffectSpecial(pobParent) {};
	virtual bool Update( float fTimeStep, bool bActive );
};

/***************************************************************************************************
*
*	CLASS			CEffectKillingSpecial
*
*	DESCRIPTION		special effect dedicated to killing special move...
*
***************************************************************************************************/
class CEffectKillingSpecial : public CEffectSpecial
{
public:
	CEffectKillingSpecial( const CEntity* pobParent ) : CEffectSpecial(pobParent) {};
	virtual bool Update( float fTimeStep, bool bActive );
};

/***************************************************************************************************
*
*	CLASS			CEffectSpecialDef
*
*	DESCRIPTION		simple static XML def for colour values in special move overide.
*
***************************************************************************************************/
class CEffectSpecialDef
{
public:
	CEffectSpecialDef( void );

	bool CalculateDesitnationProperty(	CVector& obResult, const float* pfSrc,
										EFFECT_SPECIAL_TYPE eType, int iProperty ) const;

	void GetBloomParameters( float& fRange, float& fPower, EFFECT_SPECIAL_TYPE eType ) const;
	float GetKeyValParameters( EFFECT_SPECIAL_TYPE eType ) const;
	
	float	m_fKillKeyValScalar;
	float	m_fKillBloomRange;
	float	m_fKillBloomPower;
	float	m_fKillIntroLen;
	float	m_fKillOutroLen;
	CVector	m_obKillSig_DiffuseMod;
	CVector	m_obKillSig_SpecularMod;
	CVector	m_obKillSig_ReflectMod;

	float	m_fSlowKeyValScalar;
	float	m_fSlowBloomRange;
	float	m_fSlowBloomPower;
	float	m_fSlowIntroLen;
	float	m_fSlowOutroLen;
	CVector	m_obSlowSig_DiffuseMod;
	CVector	m_obSlowSig_SpecularMod;
	CVector	m_obSlowSig_ReflectMod;
};

#endif // _EFFECT_SPECIAL_H
