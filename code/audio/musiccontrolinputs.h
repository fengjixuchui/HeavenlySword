//--------------------------------------------------------------------------------------
//!	@file musiccontrolinputs.h
//!	@author Chip Bell (SCEE)
//!	@date 16.11.06
//!
//!	@brief Music control manager built-ins (inputs).
//--------------------------------------------------------------------------------------


#include "audio/musiccontrol.h"


#ifdef _MUSIC_CONTROL_DEBUG
#include "input/inputhardware.h"
#endif // _MUSIC_CONTROL_DEBUG




//--------------------------------------------------------------------------------------
//!	@class MusicControlInputConstant
//!	@brief Supplies a constant value.
//--------------------------------------------------------------------------------------
class MusicControlInputConstant : public MusicControlManager::MusicControlInput
{
public:
	MusicControlInputConstant(float fValue);
	virtual float GetValue(void);

private:
	float m_fValue;
};


//--------------------------------------------------------------------------------------
//!	@class MusicControlInputPlayerHealth
//!	@brief Music contol input supplying player's current health as a percentage.
//--------------------------------------------------------------------------------------
class MusicControlInputPlayerHealth : public MusicControlManager::MusicControlInput
{
public:
	virtual float GetValue(void);
};


//--------------------------------------------------------------------------------------
//!	@class MusicControlInputPlayerTotalKills
//!	@brief Music contol input supplying player's total kill total from the style manager.
//--------------------------------------------------------------------------------------
class MusicControlInputPlayerTotalKills : public MusicControlManager::MusicControlInput
{
public:
	virtual float GetValue(void);
};


//--------------------------------------------------------------------------------------
//!	@class MusicControlInputPlayerKills
//!	@brief Music contol input supplying player's current kill count from the style manager.
//--------------------------------------------------------------------------------------
class MusicControlInputPlayerKills : public MusicControlManager::MusicControlInput
{
public:
	virtual float GetValue(void);
};


//--------------------------------------------------------------------------------------
//!	@class MusicControlInputPlayerHitCount
//!	@brief Music contol input supplying player's current hit count from the style manager.
//--------------------------------------------------------------------------------------
class MusicControlInputPlayerHitCount : public MusicControlManager::MusicControlInput
{
public:
	virtual float GetValue(void);
};


//--------------------------------------------------------------------------------------
//!	@class MusicControlInputPlayerSuccessfulBlockCount
//!	@brief Music contol input supplying player's successful count from the style manager.
//--------------------------------------------------------------------------------------
class MusicControlInputPlayerSuccessfulBlockCount : public MusicControlManager::MusicControlInput
{
public:
	virtual float GetValue(void);
};


//--------------------------------------------------------------------------------------
//!	@class MusicControlInputPlayerUnsuccessfulBlockCount
//!	@brief Music contol input supplying player's successful count from the style manager.
//--------------------------------------------------------------------------------------
class MusicControlInputPlayerUnsuccessfulBlockCount : public MusicControlManager::MusicControlInput
{
public:
	virtual float GetValue(void);
};


//--------------------------------------------------------------------------------------
//!	@class MusicControlInputPlayerTotalBlockCount
//!	@brief Music contol input supplying player's total (successful and unsuccessful) count from the style manager.
//--------------------------------------------------------------------------------------
class MusicControlInputPlayerTotalBlockCount : public MusicControlManager::MusicControlInput
{
public:
	virtual float GetValue(void);
};


//--------------------------------------------------------------------------------------
//!	@class MusicControlInputPlayerCausedKoCount
//!	@brief Music contol input supplying number of KOs player has suffered.
//--------------------------------------------------------------------------------------
class MusicControlInputPlayerCausedKoCount : public MusicControlManager::MusicControlInput
{
public:
	virtual float GetValue(void);
};


//--------------------------------------------------------------------------------------
//!	@class MusicControlInputPlayerGotKoCount
//!	@brief Music contol input supplying number of KOs player has inflicted.
//--------------------------------------------------------------------------------------
class MusicControlInputPlayerGotKoCount : public MusicControlManager::MusicControlInput
{
public:
	virtual float GetValue(void);
};


//--------------------------------------------------------------------------------------
//!	@class MusicControlInputPlayerSuperStyle
//!	@brief Music contol input supplying 1 if superstyle active, 0 otherwise.
//--------------------------------------------------------------------------------------
class MusicControlInputPlayerSuperStyle : public MusicControlManager::MusicControlInput
{
public:
	virtual float GetValue(void);
};



//--------------------------------------------------------------------------------------
//!	@class MusicControlInputChatterBoxInput
//!	@brief Retrieves a ChatterBox trigger statistic.
//--------------------------------------------------------------------------------------
class MusicControlInputChatterBoxInput : public MusicControlManager::MusicControlInput
{
public:
	MusicControlInputChatterBoxInput(const char* pcStatName);
	virtual float GetValue(void);

private:
	const char* m_pcStatName;
};


//--------------------------------------------------------------------------------------
//!	@class MusicControlInputThreatCount
//!	@brief Retrieves a count of nearby enemies.
//--------------------------------------------------------------------------------------
class MusicControlInputThreatCount : public MusicControlManager::MusicControlInput
{
public:
	MusicControlInputThreatCount(float fDefProximity);
	virtual float GetValue(void);

	virtual void UpdateInternal(void);

	virtual bool SetParameter(const CHashedString& obParameter, float fValue);
	virtual bool GetParameter(const CHashedString& obParameter, float rfValue);

private:
	float m_fProximity;
	int m_iCount;
};




#ifdef _MUSIC_CONTROL_DEBUG


//--------------------------------------------------------------------------------------
//!	@class MusicControlInputPadButtonCount (debug only)
//!	@brief Music contol input supplying total number of times pad X has been pressed.
//--------------------------------------------------------------------------------------
class MusicControlInputPadButtonCount : public MusicControlManager::MusicControlInput
{
public:
	MusicControlInputPadButtonCount(PAD_NUMBER ePad, PAD_BUTTON eButton);
	virtual float GetValue(void);

protected:
	virtual void UpdateInternal();

private:
	PAD_NUMBER m_ePad;
	PAD_BUTTON m_eButton;
	int m_iTotal;
	bool m_bDown;
};


//--------------------------------------------------------------------------------------
//!	@class MusicControlInputLfo
//!	@brief A one second cycle LFO.
//--------------------------------------------------------------------------------------
class MusicControlInputLfo: public MusicControlManager::MusicControlInput
{
public:
	virtual float GetValue(void);
};


#endif // _MUSIC_CONTROL_DEBUG
