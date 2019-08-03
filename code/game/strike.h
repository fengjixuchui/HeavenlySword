/***************************************************************************************************
*
*	DESCRIPTION		The strike class. Used to pass blow information between players.
*
*	NOTES
*
***************************************************************************************************/

#ifndef _STRIKE_H
#define _STRIKE_H

#include "editable/enumlist.h"

// Forward references
class	CEntity;
class	CAttackData;
class	Object_Projectile;

/***************************************************************************************************
*
*	CLASS			CStrike
*
*	DESCRIPTION		The strike class. Used to pass blow information between players.
*
*					If power is added to attack link as planned then a strike will have to be 
*					expanded in order to hold the extra information.
*
*					Prestrike will go soon because all strikes need to be sent as soon as they are
*					generated.
*
*					These should remain const data items that are passed about and interpretted but
*					never modified.
*
***************************************************************************************************/

class CStrike
{
public:

	// Construction
	CStrike(	const CEntity*		pobOriginator,			// Where did this strike come from?
				const CEntity*		pobTarget,				// Who is it aimed at
				const CAttackData*	pobAttackData,			// A pointer to the static attack information
				float				fAttackTimeScalar,		// How do the attack timings vary with those in m_pobAttackData?
				float				fAttackRange,			// What is the maximum range that this attack can reach?
				bool				bPreStrike,				// Is this a definate hit or a warning?
				bool				bCounter,				// Is this a specific counter
				bool				bIncidental,			// Is this something from a Strike2 Window?
				bool				bWithinExclusionDistance, // Is this strike occuring within exclusion distance?
				bool				bSyncronise,			// Should the struck party sync with the attack
				bool				bSpecificAttackVulnerabilityZone,// Was this attack created from a zone of specific vulnerability?
				CEntity*			pobProjectile,
				const CPoint&		obInitialPosition,      // The position of the originator - only really need if no originator
				int					iHitArea = -1           // HitArea on target character where strike hit, if it is known.
				);	// The position of the originator - only really need if no originator

	// Copy construction
	CStrike( const CStrike& obOther );

	// Destruction
	~CStrike();

	// Assignment
	CStrike& operator=( const CStrike& obOther );

	// Access to all the data - this section of interface should be const
	const CEntity*		GetOriginatorP() const		{ return m_pobOriginator; }
	const CEntity*		GetTargetP() const			{ return m_pobTarget; }
	const CAttackData*	GetAttackDataP() const		{ return m_pobAttackData; }
	float				GetAttackTimeScalar() const	{ return m_fAttackTimeScalar; }
	float				GetAttackRange() const		{ return m_fAttackRange; }
	bool				IsPreStrike() const			{ return m_bPreStrike; }
	bool				ShouldSync() const			{ return m_bSyncronise; }
	bool				IsCounter() const			{ return m_bCounter; }
	const CPoint&		GetInitialOriginatorPosition () const { return m_obInitialPosition; }
	float				GetAttackTime() const;
	float				GetAttackRecoverTime() const;
	bool				IsIncidental() const { return m_bIncidentalStrike; }
	bool				IsWithinExclusionDistance() const { return m_bWithinExclusionDistance; }
	bool				IsFromSpecificAttackVulnerabilityZone() const { return m_bSpecificAttackVulnerabilityZone; }
	const CEntity*		GetProjectile() const { return m_pobProjectile; };
	int					GetHitArea() const {return m_iHitArea; };
private:

	const CEntity*		m_pobOriginator;		// Where did this strike come from?
	const CEntity*		m_pobTarget;			// Who is it aimed at
	const CAttackData*	m_pobAttackData;		// A pointer to the static attack information
	float				m_fAttackTimeScalar;	// How do the attack timings vary with those in m_pobAttackData?
	float				m_fAttackRange;			// What is the maximum range that this attack can reach?
	bool				m_bPreStrike;			// Is this a definate hit or a warning?
	bool				m_bCounter;				// Is this attack a counter
	bool				m_bSyncronise;			// Should the struck guy sync with the attacker when he reacts
	CPoint				m_obInitialPosition;	// Initial position of the originator
	bool				m_bIncidentalStrike;
	bool				m_bWithinExclusionDistance;
	bool				m_bSpecificAttackVulnerabilityZone;
	CEntity*			m_pobProjectile;
	int					m_iHitArea;
};


#endif //_STRIKE_H
