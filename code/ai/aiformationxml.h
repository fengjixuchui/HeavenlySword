//------------------------------------------------------------------------------------------
//!
//!	\file aiformationxml.h
//!
//------------------------------------------------------------------------------------------

#ifndef _AIFORMATIONXML_H
#define _AIFORMATIONXML_H


//------------------------------------------------------------------------------------------
// Headers
//------------------------------------------------------------------------------------------
#include "editable/enums_ai.h"
#include "game/randmanager.h"

class CAIAttackRoot;
class CGromboState;
class CGromboAttack;
class CGromboEntity;
class CGromboInstance;
class CGromboAttackPattern;
class CGromboAttackList;


//------------------------------------------------------------------------------------------
//!
//! CGromboAttack
//! 
//!
//------------------------------------------------------------------------------------------
class CGromboAttack
{
public:
	CGromboAttack();

	float			m_fRange;
	float			m_fTolerance;
	float			m_fWeight;
	ntstd::String	m_obAnimName;
	CGromboState*	m_pobNextState;
};
typedef ntstd::List<CGromboAttack*, Mem::MC_AI> GromboAttackList;


//------------------------------------------------------------------------------------------
//!
//! CGromboState
//! 
//!
//------------------------------------------------------------------------------------------
class CGromboState
{
public:
	CGromboState();

	// Check whether the grombo state and any below it have a 
	// one on one state enabled. 
	bool  HasEnter1On1Recursive(void);

	CGromboState*	m_pobNextStateIfKoed;
	CGromboState*	m_pobNextStateIfBlocked;
	CGromboState*	m_pobNextStateIfRecoiled;
	CGromboState*	m_pobNextState;

	bool			m_bUntilKoed;
	bool			m_bUntilRecoiling;
	bool			m_bEnter1on1;
	bool			m_bFaceTarget;

	float			m_fUntilDist;
	float			m_fTimeout;
	float			m_fEarlyAttackRange;
	float			m_fDelay;

	ntstd::String	m_obSendMsg;
	ntstd::String	m_obSimpleAnim;

	GromboAttackList m_obAttacks;

	CAIAttackRoot*	m_pNewAIAttackTree;
};


//------------------------------------------------------------------------------------------
//!
//! CGromboEntity
//! 
//!
//------------------------------------------------------------------------------------------
class CGromboEntity
{
public:

	CGromboEntity();
	void OnPostConstruct(void);

	float			m_fFormationPositionOffset;
	float			m_fTimeNotInGrombo;
	float			m_fDelay;
	float			m_fDistanceToTarget;
	int				m_iAngle;
	int				m_iCamera;
	int				m_iRelative;
	bool			m_bHas1On1;
	CGromboState*	m_pobStartState;
	ntstd::String	m_obType;
};


//------------------------------------------------------------------------------------------
//!
//! CGromboInstance
//! 
//!
//------------------------------------------------------------------------------------------
class CGromboInstance
{
public:

	CGromboInstance();


	float			m_fWeight;
	float			m_fPriority;
	float			m_fInterruptible;
	bool			m_bSimultaneousAllowed;
	bool			m_bIncidental;
	ntstd::String	m_obPlayerCombo;
	ntstd::String	m_obOneOnOneAttacker;
	ntstd::String	m_obTargetType;

	// Metadata in the form of keystrings. 
	ntstd::String	m_obMetadata;

	ntstd::List<CGromboEntity*, Mem::MC_AI> m_obEntities;
};


//------------------------------------------------------------------------------------------
//!
//! GromboAttackPatternMetaTag
//! 
//!
//------------------------------------------------------------------------------------------
class GromboAttackPatternMetaTag
{
public:
	CHashedString m_sID;
};


//------------------------------------------------------------------------------------------
//!
//! CGromboAttackPattern
//! 
//!
//------------------------------------------------------------------------------------------
class CGromboAttackPattern
{
public:

	CGromboAttackPattern();


	float				m_fStartDelay;
	float				m_fEndDelay;
	ntstd::String		m_obOnComplete;
	ntstd::String		m_obSquads;
	CGromboInstance*	m_pGromboInstance;
};

typedef ntstd::List<CGromboAttackPattern*, Mem::MC_AI> GromboAttackPatternList;


//------------------------------------------------------------------------------------------
//!
//! CGromboAttackList
//! 
//!
//------------------------------------------------------------------------------------------
class CGromboAttackList
{
public:
	typedef GromboAttackPatternList::iterator Iterator;

	// List of the attacks
	GromboAttackPatternList m_obAttacks;

	CGromboAttackList();
};




#endif //_AIFORMATIONXML_H

