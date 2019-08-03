#ifndef _LIFEANDDEATH_H_
#define _LIFEANDDEATH_H_

#include "core/bitmask.h"
#include "core/valueandmax.h"
#include "core/timeinfo.h"

//--------------------------------------------------
//!
//!	time sequence
//!	regularly execute some function for each update
//!	Exciting class with lots of stuff to describe
//!
//--------------------------------------------------


class TimeSequence
{
public:
	typedef enum
	{
		F_RUNNING  = BITFLAG(0),
		F_PAUSE  = BITFLAG(1),
		F_REPEAT  = BITFLAG(2),
		F_MIRROR  = BITFLAG(3),
		F_PLAYWHENCREATED  = BITFLAG(4),
		F_MANUAL_MEM  = BITFLAG(5),
		F_BACKWARD  = BITFLAG(6),
		
		M_ALLOWED  = F_REPEAT | F_MIRROR | F_PLAYWHENCREATED | F_MANUAL_MEM,
	} State;	
	
	/// constructor
	TimeSequence(float fDuration, BitMask<TimeSequence::State>::Unsigned mask = static_cast<u_char>(0));
	
	/// constructor
	virtual ~TimeSequence();
	
	/// constructor
	void Set(BitMask<TimeSequence::State>::Unsigned mask);
	
	// set new duration time
	void SetDuration(float iDuration);
	
	// pause animation
	void Pause();
	
	// play animation
	void Play();
	
	// stop animation
	void Stop();
	
	// jump to a given location
	void JumpTo(float fTime);
	
	// update, return a killme value (true if the sequence is over)
	bool Update(const TimeInfo& time);
	
	// get progress
	float GetProgress() const;
	
	// get time elapsed since the beginning of the sequence
	float GetCurrent() const;
	
	// get sequence duration
	float GetDuration() const;

	// get mask
	const BitMask<TimeSequence::State>& GetMask() const;
	
	// true if the animation is running
	bool IsRunning();

	// get the number of step before the end
	float GetTimeBeforeEnd();
	
protected:	
	virtual void Begin()
	{
		// nothing
	}
	virtual void Next(const TimeInfo& time)
	{
		UNUSED(time);; // nothing
	}
	virtual void End()
	{
		// nothing
	}
	virtual void OneMore()
	{
		// nothing
	}
	
protected:
	// increment or decrement currnet position
	inline void IncOrDec(float fDelta)
	{
		m_progress.m_value += m_mask.CheckFlag(F_BACKWARD)?-fDelta:fDelta;
		
	}
	// increment or decrement currnet position
	inline bool IsInRange(float fCurrent)
	{
		return (fCurrent>=0.0f) && (fCurrent<=m_progress.m_max);
	}

	// duration of the sequence
	ValueAndMax<float> m_progress;
	
	// boolean array
	BitMask<TimeSequence::State> m_mask;	
};




//--------------------------------------------------
//!
//!	Short Class Description.
//!	Long Class Description
//!	Exciting class with lots of stuff to describe
//!
//--------------------------------------------------

class TimeSequenceManager
{
protected:	
	// little class to store sequence in order
	class TimeSequenceOrder
	{
	public:
		TimeSequence* m_pTimeSequence;
		
		TimeSequenceOrder(TimeSequence* pTimeSequence):
			m_pTimeSequence(pTimeSequence)
		{
			// nothing
		}
		
		TimeSequence* Get() {return m_pTimeSequence;}
		const TimeSequence* Get() const {return m_pTimeSequence;}
		
		bool operator< (const TimeSequenceOrder& t) const
		{
			return this->m_pTimeSequence->GetTimeBeforeEnd() < t.m_pTimeSequence->GetTimeBeforeEnd();
		}
	};
	
	// set of sequence type
	typedef ntstd::Set<TimeSequenceOrder> SequenceSet;

	// sequence set
	SequenceSet m_sequenceSet;

public:
	//! constructor
	TimeSequenceManager();
	
	//! constructor
	~TimeSequenceManager();
	
	// add a new sequence
	// if TimeSequence::F_MANUAL_MEM, the sequence don't take care about the destruction
	void AddNewSequence(TimeSequence* pTimeSequence);
	
	// return true is the sequence is currently active
	bool Has(TimeSequence* pTimeSequence);
	
	// update
	void Update(const TimeInfo& time);

}; // end of class TimeSequenceManager






#endif // end of _LIFEANDDEATH_H_
