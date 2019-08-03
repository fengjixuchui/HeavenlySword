//--------------------------------------------------------------------------------------
//!	@file musiccontrolinputs.cpp
//!	@author Chip Bell (SCEE)
//!	@date 19.10.06
//!
//!	@brief Music control manager built-ins (inputs).
//--------------------------------------------------------------------------------------


#include "audio/musiccontrolinputs.h"

#include "game/chatterboxman.h"
#include "game/combatstyle.h"
#include "game/entitymanager.h"
#include "game/query.h"




//--------------------------------------------------------------------------------------
// MusicControlInputConstant::MusicControlInputConstant
//--------------------------------------------------------------------------------------
MusicControlInputConstant::MusicControlInputConstant(float fValue)
:
	m_fValue(fValue)
{}

//--------------------------------------------------------------------------------------
// MusicControlInputConstant::GetValue
//--------------------------------------------------------------------------------------
float MusicControlInputConstant::GetValue(void)
{
	return m_fValue;
}


//--------------------------------------------------------------------------------------
// MusicControlInputPlayerHealth::GetValue
//--------------------------------------------------------------------------------------
float MusicControlInputPlayerHealth::GetValue(void)
{
	CEntity* pobEntity = CEntityManager::Get().GetPlayer();
	return (pobEntity && pobEntity->IsPlayer())
		? pobEntity->ToPlayer()->GetCurrHealth()/pobEntity->ToPlayer()->GetStartHealth()
		: 0.0f;
}


//--------------------------------------------------------------------------------------
// MusicControlInputPlayerTotalKills::GetValue
//--------------------------------------------------------------------------------------
float MusicControlInputPlayerTotalKills::GetValue(void)
{
	return (float)StyleManager::Get().GetStats().m_iTotalKills;
}


//--------------------------------------------------------------------------------------
// MusicControlInputPlayerKills::GetValue
//--------------------------------------------------------------------------------------
float MusicControlInputPlayerKills::GetValue(void)
{
	return (float)StyleManager::Get().GetStats().m_iKills;
}


//--------------------------------------------------------------------------------------
// MusicControlInputPlayerHitCount::GetValue
//--------------------------------------------------------------------------------------
float MusicControlInputPlayerHitCount::GetValue(void)
{
	HitCounter* pobHitCounter = StyleManager::Get().GetHitCounter();
	return (float)pobHitCounter->GetHits();
}


//--------------------------------------------------------------------------------------
// MusicControlInputPlayerSuccessfulBlockCount::GetValue
//--------------------------------------------------------------------------------------
float MusicControlInputPlayerSuccessfulBlockCount::GetValue(void)
{
	return (float)StyleManager::Get().GetStats().m_iSuccessfulBlocks;
}


//--------------------------------------------------------------------------------------
// MusicControlInputPlayerUnsuccessfulBlockCount::GetValue
//--------------------------------------------------------------------------------------
float MusicControlInputPlayerUnsuccessfulBlockCount::GetValue(void)
{
	return (float)StyleManager::Get().GetStats().m_iUnsuccessfulBlocks;
}


//--------------------------------------------------------------------------------------
// MusicControlInputPlayerTotalBlockCount::GetValue
//--------------------------------------------------------------------------------------
float MusicControlInputPlayerTotalBlockCount::GetValue(void)
{
	return (float)(StyleManager::Get().GetStats().m_iSuccessfulBlocks + StyleManager::Get().GetStats().m_iUnsuccessfulBlocks);
}


//--------------------------------------------------------------------------------------
// MusicControlInputPlayerCausedKoCount::GetValue
//--------------------------------------------------------------------------------------
float MusicControlInputPlayerCausedKoCount::GetValue(void)
{
	return (float)StyleManager::Get().GetStats().m_iCausedKOs;
}


//--------------------------------------------------------------------------------------
// MusicControlInputPlayerGotKoCount::GetValue
//--------------------------------------------------------------------------------------
float MusicControlInputPlayerGotKoCount::GetValue(void)
{
	return (float)StyleManager::Get().GetStats().m_iGotKOs;
}


//--------------------------------------------------------------------------------------
// MusicControlInputPlayerSuperStyle::GetValue
//--------------------------------------------------------------------------------------
float MusicControlInputPlayerSuperStyle::GetValue(void)
{
	return StyleManager::Get().GetStats().m_bSuperStyleActive ? 1.0f:0.0f;
}


//--------------------------------------------------------------------------------------
// MusicControlInputChatterBoxInput::MusicControlInputChatterBoxInput
//--------------------------------------------------------------------------------------
MusicControlInputChatterBoxInput::MusicControlInputChatterBoxInput(const char* pcStatName)
:
	m_pcStatName(pcStatName)
{}

//--------------------------------------------------------------------------------------
// MusicControlInputChatterBoxInput::GetValue
//--------------------------------------------------------------------------------------
float MusicControlInputChatterBoxInput::GetValue(void)
{
	bool bFound = false;
	int iResult = CChatterBoxMan::Get().GetCurrentStatistic(m_pcStatName, &bFound);
	return (float)iResult;
}


//--------------------------------------------------------------------------------------
// MusicControlInputThreatCount::MusicControlInputThreatCount
//--------------------------------------------------------------------------------------
MusicControlInputThreatCount::MusicControlInputThreatCount(float fDefProximity)
:
	m_fProximity(fDefProximity),
	m_iCount(0)
{}

//--------------------------------------------------------------------------------------
// MusicControlInputThreatCount::UpdateInternal
//--------------------------------------------------------------------------------------
void MusicControlInputThreatCount::UpdateInternal(void)
{
	if (!CEntityManager::Exists() || !CEntityManager::Get().GetPlayer())
	{
		m_iCount = 0;
		return;
	}

	// Query for appropriate entities
	CEntityQuery obQuery;

	// Enemies
	CEQCIsEnemy obEnemy;
	obQuery.AddClause(obEnemy);

	// Alive
	CEQCHealthLTE obHealth(0.0f);
	obQuery.AddUnClause(obHealth);

	// Nearby
	CEQCProximitySphere obSphere;
	obSphere.Set(CEntityManager::Get().GetPlayer()->GetPosition(), m_fProximity);
	obQuery.AddClause(obSphere);

	// Find matching AI entities
	CEntityManager::Get().FindEntitiesByType(obQuery, CEntity::EntType_AI);

	// Done
	m_iCount = (int)obQuery.GetResults().size();
}

//--------------------------------------------------------------------------------------
// MusicControlInputThreatCount::GetValue
//--------------------------------------------------------------------------------------
float MusicControlInputThreatCount::GetValue(void)
{
	return (float)m_iCount;
}

//--------------------------------------------------------------------------------------
// MusicControlInputThreatCount::SetParameter
//--------------------------------------------------------------------------------------
bool MusicControlInputThreatCount::SetParameter(const CHashedString& obParameter, float fValue)
{
	if (obParameter == "proximity")
	{
		m_fProximity = fValue;
		return true;
	}

	return false;
}

//--------------------------------------------------------------------------------------
// MusicControlInputThreatCount::GetParameter
//--------------------------------------------------------------------------------------
bool MusicControlInputThreatCount::GetParameter(const CHashedString& obParameter, float rfValue)
{
	if (obParameter == "proximity")
	{
		rfValue = m_fProximity;
		return true;
	}

	return false;
}




#ifdef _MUSIC_CONTROL_DEBUG


//--------------------------------------------------------------------------------------
// MusicControlInputPadButtonCount::MusicControlInputPadButtonCount
//--------------------------------------------------------------------------------------
MusicControlInputPadButtonCount::MusicControlInputPadButtonCount(PAD_NUMBER ePad, PAD_BUTTON eButton)
:
	m_ePad(ePad),
	m_eButton(eButton),
	m_iTotal(0),
	m_bDown(false)
{}

//--------------------------------------------------------------------------------------
// MusicControlInputPadButtonCount::GetValue
//--------------------------------------------------------------------------------------
float MusicControlInputPadButtonCount::GetValue(void)
{
	return (float)m_iTotal;
}

//--------------------------------------------------------------------------------------
// MusicControlInputPadButtonCount::UpdateInternal
//--------------------------------------------------------------------------------------
void MusicControlInputPadButtonCount::UpdateInternal()
{
	if (m_bDown)
	{
		if (CInputHardware::Get().GetPad(m_ePad).GetPressed()&m_eButton)
			return;
		m_bDown = false;
	}
	else if (CInputHardware::Get().GetPad(m_ePad).GetPressed()&m_eButton)
	{
		m_bDown = true;
		++m_iTotal;
	}
}


//--------------------------------------------------------------------------------------
// MusicControlInputLfo::GetValue
//--------------------------------------------------------------------------------------
float MusicControlInputLfo::GetValue(void)
{
	float fInt = 0.0;
	float fFrac = modf((float)(CTimer::Get().GetSystemTime()/(double)TWO_PI), &fInt);
	return fsinf(fFrac*TWO_PI);
}


#endif // _MUSIC_CONTROL_DEBUG




//--------------------------------------------------------------------------------------
//	RegisterBuiltinInputs
//!	Registers a instance of each of the built-in music control inputs with the music
//!	control manager.
//--------------------------------------------------------------------------------------
void MusicControlManager::RegisterBuiltinInputs(void)
{
	RegisterInput(NT_NEW MusicControlInputConstant(1.0f), "one");
	RegisterInput(NT_NEW MusicControlInputConstant(0.0f), "zero");
	RegisterInput(NT_NEW MusicControlInputPlayerHealth(), "playerHealth");
	RegisterInput(NT_NEW MusicControlInputPlayerTotalKills(), "playerTotalKills");
	RegisterInput(NT_NEW MusicControlInputPlayerKills(), "playerKills");
	RegisterInput(NT_NEW MusicControlInputPlayerHitCount(), "playerHitCount");
	RegisterInput(NT_NEW MusicControlInputPlayerSuccessfulBlockCount(), "playerSuccessfulBlocks");
	RegisterInput(NT_NEW MusicControlInputPlayerUnsuccessfulBlockCount, "playerUnsuccessfulBlocks");
	RegisterInput(NT_NEW MusicControlInputPlayerTotalBlockCount, "playerTotalBlocks");
	RegisterInput(NT_NEW MusicControlInputPlayerCausedKoCount, "playerCausedKOs");
	RegisterInput(NT_NEW MusicControlInputPlayerGotKoCount, "playerGotKOs");
	RegisterInput(NT_NEW MusicControlInputPlayerSuperStyle, "playerSuperStyle");
	RegisterInput(NT_NEW MusicControlInputChatterBoxInput("EnemyKO"), "cbEnemyKO");
	RegisterInput(NT_NEW MusicControlInputChatterBoxInput("PlayerKO"), "cbPlayerKO");
	RegisterInput(NT_NEW MusicControlInputChatterBoxInput("EnemyDeath"), "cbEnemyDeath");
	RegisterInput(NT_NEW MusicControlInputThreatCount(10.0f), "playerThreatCount");

#ifdef _MUSIC_CONTROL_DEBUG
	RegisterInput(NT_NEW MusicControlInputPadButtonCount(PAD_0, PAD_FACE_3), "padButtonCross");
	RegisterInput(NT_NEW MusicControlInputLfo(), "lfo");
#endif // _MUSIC_CONTROL_DEBUG
}
