/***************************************************************************************************
*
*	FILE			timerbindings.h
*
*	DESCRIPTION		
*
***************************************************************************************************/

#ifndef _TIMERBINDINGS_H
#define _TIMERBINDINGS_H

class CEntity;


//------------------------------------------------------------------------------------------
//!
//!	LuaTimerManager
//!	Manages a collection of timer objects for use with lua
//!
//------------------------------------------------------------------------------------------
class LuaTimerManager : public Singleton<LuaTimerManager>
{
// Timer Sub Class
public:
	class LuaTimer
	{
	private:
		LuaTimer() {m_bEnabled = false;m_pEntity=0;}

	public:
		void Kill()                {m_bEnabled = false;}
		bool HasFinished()         {return m_fTimeLeft < 0.0f;}
		void SetTime(float fTime)  {m_fTimeLeft = fTime;}

	private:
		int       m_iID; // This ID is unique (effectively) to prevent misidentifications
		bool      m_bEnabled;
		bool      m_bPersistent;
		float     m_fTimeLeft;
		CEntity*  m_pEntity;
		NinjaLua::LuaObject m_callback;

		friend class LuaTimerManager;
	};

// Methods
public:
	LuaTimerManager() {m_timerPostfix = 1;}

	int  AddTimer(float fTime, NinjaLua::LuaObject& callback, bool bPersistent);
	LuaTimerManager::LuaTimer* GetTimer(int iID);

	void UpdateTimers();
	void ClearTimers();

// Members;
private:
	static const int MAX_TIMERS_POW2 = 7;
	static const int MAX_TIMERS = 1 << MAX_TIMERS_POW2;
	static const int TIMER_MASK = MAX_TIMERS - 1;

	LuaTimer m_timers[MAX_TIMERS];
	int      m_timerPostfix;
};


/***************************************************************************************************
*
*	CLASS			TimerBindings
*
*	DESCRIPTION		
*
***************************************************************************************************/
class TimerBindings : public Singleton<TimerBindings>
{
public:
	static void Register();
};


#endif // _TIMERBINDINGS_H
