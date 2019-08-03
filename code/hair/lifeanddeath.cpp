#include "lifeanddeath.h"














TimeSequence::~TimeSequence()
{
	// nothing
}

TimeSequence::TimeSequence(float fDuration, BitMask<TimeSequence::State>::Unsigned mask):
	m_progress(fDuration,0.0f),
	m_mask(mask)
{
	ntAssert(m_progress.m_max>=0.0f);
	if(m_mask.CheckFlag(F_PLAYWHENCREATED))
	{
		Play();
	}
	
	Set(mask);
}
	
/// constructor
void TimeSequence::Set(BitMask<TimeSequence::State>::Unsigned mask)
{
	m_mask.m_uMask |= (mask & M_ALLOWED);
}
	
// pause animation
void TimeSequence::Pause()
{
	m_mask.Toggle(F_PAUSE);
}

void TimeSequence::SetDuration(float fDuration)
{
	ntAssert(!m_mask.AllOfThem(F_RUNNING));
	ntAssert(fDuration>=0);
	m_progress.m_max=fDuration;
}

void TimeSequence::Play()
{
	m_mask.Set(F_RUNNING);
	m_mask.Unset(F_PAUSE);
	this->Begin();
}

void TimeSequence::Stop()
{
	m_mask.Unset(F_RUNNING);
	this->End();
	//m_progress.m_value = 0;
}

void TimeSequence::JumpTo(float fCurrent)
{
	ntAssert(IsInRange(fCurrent));
	m_progress.m_value = fCurrent;
}

// return a killme value
bool TimeSequence::Update(const TimeInfo& time)
{
	if(!m_mask.AllOfThem(F_RUNNING) || m_mask.AllOfThem(F_PAUSE))
	{
		// not running...
		return false;
	}

	this->Next(time);
	IncOrDec(time.GetTimeSpeed());
	
	if(!IsInRange(m_progress.m_value))
	{
		if(m_mask[F_MIRROR])
		{
			m_mask.Toggle(F_BACKWARD);
			if(m_progress.m_value<=0.0f)
			{
				m_progress.m_value = 0.0f;
			}
			else
			{
				m_progress.m_value = m_progress.m_max;
			}
		}
		else
		{
			if(m_progress.m_value<=0.0f)
			{
				m_progress.m_value = m_progress.m_max + m_progress.m_value;
			}
			else
			{
				m_progress.m_value = m_progress.m_value - m_progress.m_max;
			}
		}
		
		if(m_mask[F_REPEAT])
		{
			// end
			OneMore();
			return false;
		}
		else
		{
			// end but repeat
			Stop();
			return true;
		}
	}
	else
	{
		// normal
		return false;
	}
}

float TimeSequence::GetProgress() const
{
	return m_progress.m_value / m_progress.m_max;
}

float TimeSequence::GetCurrent() const
{
	return m_progress.m_value / m_progress.m_max;
}

float TimeSequence::GetDuration() const
{
	return m_progress.m_max;
}

const BitMask<TimeSequence::State>& TimeSequence::GetMask() const
{
	return m_mask;
}

bool TimeSequence::IsRunning()
{
	return m_mask.AllOfThem(F_RUNNING);
}

float TimeSequence::GetTimeBeforeEnd()
{
	return m_progress.m_max - m_progress.m_value;
}















//! constructor
TimeSequenceManager::TimeSequenceManager()
{
	// nothing
}

//! constructor
TimeSequenceManager::~TimeSequenceManager()
{
	for(SequenceSet::iterator it = m_sequenceSet.begin();it!=m_sequenceSet.end();it++)
	{
		if(!it->m_pTimeSequence->GetMask().CheckFlag(TimeSequence::F_MANUAL_MEM))
		{
			NT_DELETE_CHUNK( Mem::MC_PROCEDURAL, it->m_pTimeSequence );
		}
	}
}


// return true is the sequence is currently active
bool TimeSequenceManager::Has(TimeSequence* pTimeSequence)
{
	return m_sequenceSet.find(pTimeSequence)!=m_sequenceSet.end();
}

void TimeSequenceManager::AddNewSequence(TimeSequence* pTimeSequence)
{
	m_sequenceSet.insert(TimeSequenceOrder(pTimeSequence));
}

void TimeSequenceManager::Update(const TimeInfo& time)
{
	SequenceSet::iterator it = m_sequenceSet.begin();
	SequenceSet::iterator end = m_sequenceSet.end();
	for(;it!=end;it++)
	{
		bool bKillme = it->m_pTimeSequence->Update(time);
		if(bKillme)
		{
			SequenceSet::iterator itkill = it;
			it++;
			
			if(!itkill->m_pTimeSequence->GetMask().CheckFlag(TimeSequence::F_MANUAL_MEM))
			{
				NT_DELETE( itkill->m_pTimeSequence );
			}
			m_sequenceSet.erase(itkill);
		}
	}
}

