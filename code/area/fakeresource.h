//--------------------------------------------------
//!
//!	\file fakeresource.h
//!	Place holder resource system used by AreaManager
//!
//--------------------------------------------------

#ifndef _FAKE_RESOURCE_H
#define _FAKE_RESOURCE_H

/*
#include "core/timer.h"

//--------------------------------------------------
//!
//!	ResourceStatus
//! status class simulates asynchronous loading of a resource
//!
//--------------------------------------------------
class ResourceStatus
{
public:
	ResourceStatus() { Reset(); }
	void Reset() { m_bLoadRequested = false; }

	//! is this resource loaded yet?
	bool ResourceLoaded()
	{
		ntError_p( m_bLoadRequested, ("This resource has not been requested") );
		float fDuration = CTimer::GetElapsedSystemTimeSinceHere( m_requestTime );
		return (fDuration > m_fDelayTime) ? true : false;
	}

	//! How long till this resource is loaded
	float EstimatedTimeOfArrival()
	{
		if (ResourceLoaded())
			return 0.0f;

		float fDuration = CTimer::GetElapsedSystemTimeSinceHere( m_requestTime );
		return (m_fDelayTime - fDuration);
	}

	//! request a fake resource load that takes fDelay seconds
	void RequestLoad( float fDelay )
	{
		ntError_p( !m_bLoadRequested, ("This resource has already been requested") );
		m_bLoadRequested = true;
		m_fDelayTime = fDelay;
		m_requestTime = CTimer::GetHWTimer();
	}

	//! cancel a fake load of a resource
	void LoadCancel()
	{
		ntError_p( m_bLoadRequested, ("This resource has not been requested") );
		m_bLoadRequested = false;
	}

	//! fake unload of a resource
	void Unload()
	{
		ntError_p( m_bLoadRequested, ("This resource has not been requested") );
		m_bLoadRequested = false;
	}

private:
	bool		m_bLoadRequested;
	float		m_fDelayTime;
	int64_t		m_requestTime;
};

//--------------------------------------------------
//!
//!	FakeResourceSystem
//! Fake resource system interface for future load / unload requests.
//!
//--------------------------------------------------
class FakeResourceSystem : public Singleton<FakeResourceSystem>
{
public:
	FakeResourceSystem() { Reset(); }

	void Reset()
	{
		for (int32_t i = 0; i < AreaSystem::NUM_AREAS; i++)
			m_aAreaResources[i].Reset();
	}

	// check to see if a load has finished
	bool Area_LoadFinished( int32_t iAreaNumber )
	{
		ntAssert_p( AreaSystem::IsValidAreaNumber(iAreaNumber), ("Invalid area number") );
		return m_aAreaResources[iAreaNumber-1].ResourceLoaded();
	}

	// check to see if a load has finished
	float Area_LoadETA( int32_t iAreaNumber )
	{
		ntAssert_p( AreaSystem::IsValidAreaNumber(iAreaNumber), ("Invalid area number") );
		return m_aAreaResources[iAreaNumber-1].EstimatedTimeOfArrival();
	}

	// request load of an area
	void Area_LoadRequest( int32_t iAreaNumber, float fDelay )
	{
		ntAssert_p( AreaSystem::IsValidAreaNumber(iAreaNumber), ("Invalid area number") );
		m_aAreaResources[iAreaNumber-1].RequestLoad( fDelay );
	}

	// cancel load of an area
	void Area_LoadCancel( int32_t iAreaNumber )
	{
		ntAssert_p( AreaSystem::IsValidAreaNumber(iAreaNumber), ("Invalid area number") );
		m_aAreaResources[iAreaNumber-1].LoadCancel();
	}

	// unload an area
	void Area_Unload( int32_t iAreaNumber )
	{
		ntAssert_p( AreaSystem::IsValidAreaNumber(iAreaNumber), ("Invalid area number") );
		m_aAreaResources[iAreaNumber-1].Unload();
	}

private:
	ResourceStatus	m_aAreaResources[AreaSystem::NUM_AREAS];
};
*/

#endif // _FAKE_RESOURCE_H

