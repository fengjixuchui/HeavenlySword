#ifndef _TIMEINFO_INL_
#define _TIMEINFO_INL_

#include "timeinfo.h"

template<uint16_t SIZE>
TemplateTimeInfo<SIZE>::TemplateTimeInfo(float fInitTime)
{
	ntAssert(SIZE>=3);
	m_times.assign(fInitTime);
}

template<uint16_t SIZE>
void TemplateTimeInfo<SIZE>::UpdateTimeChange(float fDelta)
{
	m_index++;
	m_times[m_index[SIZE-1]] = m_times[m_index[SIZE-2]] + fDelta;
}

template<uint16_t SIZE>
void TemplateTimeInfo<SIZE>::UpdateTime(float fTime)
{
	m_index++;
	m_times[m_index[SIZE-1]] = fTime;
}

template<uint16_t SIZE>
float TemplateTimeInfo<SIZE>::GetTime() const
{
	return m_times[m_index[SIZE-1]];
}

template<uint16_t SIZE>
float TemplateTimeInfo<SIZE>::GetTime(uint16_t iIndex) const
{
	return m_times[m_index[SIZE-1-iIndex]];
}

template<uint16_t SIZE>
float TemplateTimeInfo<SIZE>::GetTimeSpeed() const
{
	return m_times[m_index[SIZE-1]]-m_times[m_index[SIZE-2]];
}

template<uint16_t SIZE>
float TemplateTimeInfo<SIZE>::GetTimeAcceleration() const
{
	return m_times[m_index[SIZE-1]]-2.0f*m_times[m_index[SIZE-2]]+m_times[m_index[SIZE-3]];
}


#endif // end of _TIMEINFO_INL_
