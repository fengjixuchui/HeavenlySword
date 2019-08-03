#ifndef _TIMEINFO_H_
#define _TIMEINFO_H_

#include "core/bitmask.h"
#include "core/rotationnalindex.h"

//--------------------------------------------------
//!
//!	time class.
//!	remember a certain amount of time value
//!	can compute speed (time change) and acceleration
//!
//--------------------------------------------------

template<uint16_t SIZE>
class TemplateTimeInfo
{
private:
	Array<float,SIZE> m_times;
	RotationnalIndex<uint16_t,SIZE> m_index;
public:
	TemplateTimeInfo(float fInitTime = 0);
	
	void UpdateTimeChange(float fTimeStep);
	void UpdateTime(float fTime);
	float GetTime(uint16_t iIndex) const;
	
	float GetTime() const;
	float GetTimeSpeed() const;
	float GetTimeAcceleration() const;
};

//--------------------------------------------------
//!
//!	Time class containing the 3 last time value
//! it's size is 16 Byte
//!
//--------------------------------------------------
class TimeInfo: public TemplateTimeInfo<3>
{
public:
	typedef enum
	{
		F_PAUSE  = BITFLAG(0),
	} State;
protected:
	typedef BitMask<State,uint16_t> Mask;
	Mask m_mask;
public:
	TimeInfo();
	const Mask& GetMask() const;
	Mask& GetMask();
};


#endif // end of _TIMEINFO_H_
