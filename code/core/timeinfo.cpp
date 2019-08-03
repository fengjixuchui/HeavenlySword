#include "core/timeinfo.h"


TimeInfo::TimeInfo()
{
	m_mask.Set(TimeInfo::F_PAUSE);
}

const TimeInfo::Mask& TimeInfo::GetMask() const
{
	return m_mask;
}

TimeInfo::Mask& TimeInfo::GetMask()
{
	return m_mask;
}
