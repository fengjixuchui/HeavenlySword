/***************************************************************************************************
*
*	DESCRIPTION		aicharactersettings.h - classes containing parameters used by
*					AI state machines
*
*	NOTES
*
***************************************************************************************************/

#ifndef _AI_CHARACTERSETTINGS_H
#define _AI_CHARACTERSETTINGS_H

/***************************************************************************************************
*
*	CLASS			CAIMeleeSettings
*
*	DESCRIPTION		Settings to define the behaviour of melee combatant
*
	NOTES			
*
***************************************************************************************************/

class CAIMeleeSettings
{
public:
	CAIMeleeSettings();
	~CAIMeleeSettings();

	// elements editable in Welder
	float	m_fPatrolWalkTimeMin;
	float	m_fPatrolWalkTimeMax;
	float	m_fPatrolLookTimeMin;
	float	m_fPatrolLookTimeMax;
	float	m_fInvHesitationTime;
	float	m_fInvSoundTimeMin;
	float	m_fInvSoundTimeMax;
	float	m_fInvSoundTimeExtension;
	float	m_fInvSightTimeMin;
	float	m_fInvSightTimeMax;
	float	m_fInvSightTimeExtension;

private:

};

class CAISettingsManager : public Singleton<CAISettingsManager>
{
public:



private:
	CAIMeleeSettings	m_obSwordsmanSettings;
	CAIMeleeSettings	m_obAxemanSettings;
};

#endif // _AI_CHARACTERSETTINGS_H
