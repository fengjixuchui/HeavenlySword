#ifndef _INPUT_SEQUENCE_H
#define _INPUT_SEQUENCE_H

#include "editable/enumlist.h"

// Forward declarations.
struct CProcessedInput;


//--------------------------------------------------
//
//! An InputSequence represents a series of button presses and moves that can be performed with a joypad.
//! It is used to hijack input into an Entity's InputComponent, enabling us to replay combo chains, etc. 
//
//--------------------------------------------------
class CInputSequence
{
public:
	enum ATTACK
	{
		ATK_SPEED_FAST,
		ATK_SPEED_MEDIUM,
		ATK_POWER_FAST,
		ATK_POWER_MEDIUM,
		ATK_RANGE_FAST,
		ATK_RANGE_MEDIUM
	};

	static const float DURATION_DEFAULT;
	static const float GAP_DEFAULT;
	static const float GAP_SIMULTANEOUS;

	void Push( ATTACK_MOVE_TYPE eMove, float fRelativeTime = GAP_DEFAULT, float fDuration = DURATION_DEFAULT );
	void Push( VIRTUAL_BUTTON_TYPE eButton, float fRelativeTime = GAP_DEFAULT, float fDuration = DURATION_DEFAULT );
	void Mash( VIRTUAL_BUTTON_TYPE eButton, float fRelativeTime = GAP_DEFAULT );
	
	float GetDuration() const;
	bool Update( float fTimeChange );
	void Evaluate( CProcessedInput& obOutProcessedInput ) const;
	void Clear();
	void Play();
	void Stop();
	void Seek( float fAbsoluteTime );
	void Rewind();

private:
	enum CONTROLLER_COMPONENT
	{
		CC_L_ANALOG_STICK,
		CC_R_ANALOG_STICK,
		CC_BUTTONS_ACTUAL,
		CC_BUTTONS_VIRTUAL
	};

	typedef ntstd::Map< float, unsigned short > ButtonTimeLine;

	static const CDirection DIRECTION_NONE;

	inline u_int EvaluatePressed( CONTROLLER_COMPONENT eComponent ) const;
	inline u_int EvaluateHeld( CONTROLLER_COMPONENT eComponent ) const;
	inline u_int EvaluateReleased( CONTROLLER_COMPONENT eComponent ) const;
	inline CDirection EvaluateDirection( float fTime, CONTROLLER_COMPONENT eComponent ) const;
	inline float EvaluateSpeed( float fTime, CONTROLLER_COMPONENT eComponent ) const;
	inline bool EvaluateWiggled( float fTime ) const;
	inline int EvaluateWiggleSide( float fTime ) const;
	inline u_int EvaluateButtonState( float fTime, CONTROLLER_COMPONENT eComponent ) const;

	void PressButton( VIRTUAL_BUTTON_TYPE eButton, float fAbsoluteTime );
	void ReleaseButton( VIRTUAL_BUTTON_TYPE eButton, float fAbsoluteTime );
	u_int ButtonStateAtTime( float fTime, const ButtonTimeLine& obTimeLine ) const;
	void UpdateDuration();
	void UpdateDuration( float fAbsoluteTime );

	ButtonTimeLine m_obButtonTimeLine;
	ButtonTimeLine m_obVButtonTimeLine;
	float m_fInsertTime;
	float m_fPosition;
	float m_fDuration;
	bool m_bPlaying;

	mutable u_int m_uiButtonState;
	mutable u_int m_uiVButtonState;
	mutable u_int m_uiLastFrameButtonState;
	mutable u_int m_uiLastFrameVButtonState;
};


#endif // _INPUT_SEQUENCE_H
