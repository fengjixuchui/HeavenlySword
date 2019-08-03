/***************************************************************************************************
*
*	FILE			timerbindings.cpp
*
*	DESCRIPTION		
*
***************************************************************************************************/


//-------------------------------------------------------------------------------------------------
// Includes                                                                                        
//-------------------------------------------------------------------------------------------------
#include "game/luaglobal.h"
#include "game/entitymanager.h"
#include "game/timerbindings.h"

#include "core/timer.h"

//-------------------------------------------------------------------------------------------------
// BINDFUNC:    Timer_Create(float time, callbackFunction) returns timerID
// DESCRIPTION: Create a new lua timer
//-------------------------------------------------------------------------------------------------
static int Timer_Create(NinjaLua::LuaState* state)
{
	NinjaLua::LuaStack args(state);
	
	// Check arguments are valid.
	ntAssert(args[1].IsNumber() && args[1].GetFloat() > 0.0f);
	ntAssert(args[2].IsFunction());

	//ntAssert( &CLuaGlobal::Get().GetState() != state );

	int iID = 0;
	
	if(!CLuaGlobal::Get().State().IsMine(args[1]))
	{
		NinjaLua::LuaObject obLocal = args[2];
		obLocal.Push();

		// Move the function from one stack to another. 
		state->XMove( CLuaGlobal::Get().State(), 1 );

		// Obtain a lua object handle to the function
		obLocal = NinjaLua::LuaStackObject( CLuaGlobal::Get().State(), -1 );
		
		// Add the time, callback function should be on correct stack
		iID = LuaTimerManager::Get().AddTimer(args[1].GetFloat(), obLocal, false);

		// Remove the item from the stack
		lua_pop(&(**state), 1);
	}
	else
	{
		NinjaLua::LuaObject callback( args[2] );
		iID = LuaTimerManager::Get().AddTimer(args[1].GetFloat(), callback, false);
	}
	

	state->Push(iID);
	return 1;
}


//-------------------------------------------------------------------------------------------------
// BINDFUNC:    Timer_Destroy(timerID)
// DESCRIPTION: Stop a lua timer
//-------------------------------------------------------------------------------------------------
static int Timer_Destroy(NinjaLua::LuaState* state)
{
	NinjaLua::LuaStack args(state);
	
	// Check arguments are valid.
	ntAssert(args[1].IsInteger());

	LuaTimerManager::LuaTimer* pTimer = LuaTimerManager::Get().GetTimer(args[1].GetInteger());

	if(pTimer)
		pTimer->Kill();

	return 0;
}


//------------------------------------------------------------------------------------------
//!                                                                                         
//!	LuaTimerManager::AddTimer                                                             
//!                                                                                         
//!                                                                                         
//------------------------------------------------------------------------------------------
int  LuaTimerManager::AddTimer(float fTime, NinjaLua::LuaObject& callback, bool bPersistent )
{ 
	// Find a free slot for a timer.
	for(int i = 0; i < MAX_TIMERS; i++)
	{
		if(!m_timers[i].m_bEnabled)
		{
			// Set the timers unique ID
			m_timers[i].m_iID = i + (m_timerPostfix++ << MAX_TIMERS_POW2);

			// Set other attributes
			m_timers[i].m_bEnabled = true;
			m_timers[i].m_bPersistent = bPersistent;
			m_timers[i].m_fTimeLeft = fTime;

			// Set up a callback if provided
			if(callback.IsFunction())
				m_timers[i].m_callback = callback;
			else
				m_timers[i].m_callback.AssignNil(CLuaGlobal::Get().State());

			m_timers[i].m_pEntity = CLuaGlobal::Get().GetTarg();
			//ntError_p( m_timers[i].m_pEntity, ( "No target associated with a lua timer" ) );

			// Return the unique identifier
			return m_timers[i].m_iID;
		}
	}

	//user_warn_msg( ("Warning - Out of Lua timers") );

	// There was no free slot...
	return -1;
}


//------------------------------------------------------------------------------------------
//!                                                                                         
//!	LuaTimerManager::GetTimer                                                              
//!                                                                                         
//!                                                                                         
//------------------------------------------------------------------------------------------
LuaTimerManager::LuaTimer* LuaTimerManager::GetTimer(int iTimer)
{
	if(iTimer > 0)
	{
		int iIdx = iTimer & TIMER_MASK;

		if(m_timers[iIdx].m_iID == iTimer)
			return &m_timers[iIdx];
	}
	else
	{
		ntAssert(false);
	}

	// This timer doesn't exist
	return 0;
}


//------------------------------------------------------------------------------------------
//!                                                                                         
//!	LuaTimerManager::UpdateTimers                                                             
//!                                                                                         
//!                                                                                         
//------------------------------------------------------------------------------------------
void LuaTimerManager::UpdateTimers()
{
	float fTimeDelta = CTimer::Get().GetGameTimeChange();

	for(int i = 0; i < MAX_TIMERS; i++)
	{
		if(m_timers[i].m_bEnabled && m_timers[i].m_fTimeLeft > 0.0f)
		{
			m_timers[i].m_fTimeLeft -= fTimeDelta;

			if(m_timers[i].m_fTimeLeft <= 0.0f)
			{
				CEntity* pOldEnt = CLuaGlobal::Get().GetTarg();
				CLuaGlobal::Get().SetTarg( m_timers[i].m_pEntity );

				if( m_timers[i].m_callback.IsFunction() )
				{
					NinjaLua::LuaFunction fn(m_timers[i].m_callback);
					fn();
				}

				CLuaGlobal::Get().SetTarg( pOldEnt );

				// Kill the timer if it's not persistent
				if(!m_timers[i].m_bPersistent)
					m_timers[i].Kill();
			}
		}
	}
}


//------------------------------------------------------------------------------------------
//!                                                                                         
//!	LuaTimerManager::ClearTimers                                                             
//!                                                                                         
//!                                                                                         
//------------------------------------------------------------------------------------------
void LuaTimerManager::ClearTimers()
{
	for(int i = 0; i < MAX_TIMERS; i++)
	{
		m_timers[i].m_bEnabled = false;

	}
}


//------------------------------------------------------------------------------------------
//!                                                                                         
//!	GetSystemTicks                                                 
//!                                                                                         
//!                                                                                         
//------------------------------------------------------------------------------------------
static int GetSystemTicks()
{
	return static_cast<int>( CTimer::Get().GetSystemTicks() );
}

static float GetSystemTime()
{
	return (float)CTimer::Get().GetSystemTime();
}


//------------------------------------------------------------------------------------------
//!                                                                                         
//!	TimerBindings::Register                                                                 
//!                                                                                         
//!                                                                                         
//------------------------------------------------------------------------------------------
void TimerBindings::Register()
{
	NinjaLua::LuaObject obGlobals = CLuaGlobal::Get().State().GetGlobals();

	NT_NEW LuaTimerManager;

	obGlobals.RegisterRaw("Timer_Create",				Timer_Create);
	obGlobals.RegisterRaw("Timer_Destroy",				Timer_Destroy);

	obGlobals.Register("GetSystemTicks",				GetSystemTicks);
	obGlobals.Register("GetSystemTime",					GetSystemTime);
}
