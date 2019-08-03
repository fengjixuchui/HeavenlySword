#include "tbd/filteredgatso.h"
#include "core/visualdebugger.h"
#include "input/inputhardware.h"

void FilteredGatso::Record::Start()
{
	m_lStart = CTimer::GetHWTimer();
	m_iNbTick++;
}
void FilteredGatso::Record::Stop()
{
	LONGLONG lElapsed = CTimer::GetHWTimer() - m_lStart;
	m_lAccum+=lElapsed;
}

void FilteredGatso::Record::AddCheckPoint(const ntstd::String& checkName)
{
	LONGLONG lNow = CTimer::GetHWTimer();
	LONGLONG lElapsed = lNow - m_lStart;
	m_lStart = lNow;
	m_lAccum+=lElapsed;
	m_checkPointList.push_back(CheckPoint(checkName,lElapsed));
}

void FilteredGatso::Record::Update(const RotIndex& p, float fTimeCoef)
{
	if(m_bFirstUpdate)
	{
		m_fAverage = static_cast<float>(m_lAccum) * fTimeCoef;
		m_array.assign(m_fAverage);
	}
	else
	{
		float fWeight = 1.0f / static_cast<float>(Size);
		m_fAverage -= m_array[p[Size-1]] * fWeight;
		m_array[p[Size-1]] =  static_cast<float>(m_lAccum) * fTimeCoef;
		m_fAverage += m_array[p[Size-1]] * fWeight;
	}
	m_iNbTick=0;
	m_lAccum=0;
	m_bFirstUpdate=false;
	m_checkPointList.clear();
}

FilteredGatso::Record::Record():
	m_fAverage(0.0f),
	m_lAccum(0),
	m_lStart(0),
	m_bFirstUpdate(true)
{
	m_array.assign(0.0f);
}

float FilteredGatso::Record::RealAverage()
{
	float fAverage = 0.0f;
	for(int iElem = 0 ; iElem < m_array.size() ; iElem++ )
	{
		fAverage+=m_array[iElem];
	}
	return fAverage / static_cast<float>(Size);
}








FilteredGatso::FilteredGatso():
	m_origin(10.0f, 10.0f)
{
	// nothing
}
	
 
void FilteredGatso::CreateTmpList(TmpList& list)
{
	for(Map::iterator it = m_map.begin();
		it != m_map.end();
		it++)
	{
		if(m_filter && m_filter->Match(it->first))
		{
			list.push_back(&(*it));
		}
	}
}


void FilteredGatso::Draw()
{
	if(!m_mask[F_ENABLE])
	{
		return;
	}
	
	TmpList list;
	CreateTmpList(list);
	
	if(m_mask[F_AVERAGESORT])
	{
		list.sort(AverageSort());
	}
	else if(m_mask[F_NAMESORT])
	{
		list.sort(NameSort());
	}
	
	Draw(list);
}

void FilteredGatso::Draw(const TmpList& list)
{
	Vec2 position = m_origin;
	char tmp[256];
	float fTimeCoef = GetTimeCoef();
	
	
	sprintf(tmp,"%40.40s   %8.3s %8.3s","name","cur","ave");
	g_VisualDebug->Printf2D( position[0], position[1], 0xffff00ff, 0, tmp);
	position[1] += 12.0f;
	
	for(TmpList::const_iterator itaux = list.begin();
		itaux != list.end();
		itaux++)
	{
		// main value
		const Map::value_type* it = *itaux;
		sprintf(tmp,"%40.40s : %8.3f %8.3f",
			it->first, it->second.m_array[m_rotIndex[Size-2]], it->second.m_fAverage);
		g_VisualDebug->Printf2D( position[0], position[1], 0xffffffff, 0, tmp);
		position[1] += 12.0f;
		
		// check point
		for(Record::CheckPointList::const_iterator it2 = it->second.m_checkPointList.begin();
			it2 != it->second.m_checkPointList.end();
			++it2)
		{
			const Record::CheckPoint& checkPoint = *it2;
			sprintf(tmp,"  * %40.40s : %8.3f", checkPoint.m_name.c_str(), checkPoint.GetTime(fTimeCoef));
			position[1] += 12.0f;
		}
	}
}



void FilteredGatso::Update()
{
	// TODO Deano how did this ever compile?
//	KeyEvent();
		
	m_rotIndex++;
	for(Map::iterator it = m_map.begin();
		it != m_map.end();
		it++)
	{
		it->second.Update(m_rotIndex, GetTimeCoef());
	}
	
	Draw();
}

void FilteredGatso::Start(const ntstd::String& name)
{
	m_map[name].Start();
}

void FilteredGatso::Stop(const ntstd::String& name)
{
	m_map[name].Stop();
}

void FilteredGatso::CheckPoint(const ntstd::String& name, const ntstd::String& checkName)
{
	if(m_mask[F_CHECKPOINT])
	{
		m_map[name].AddCheckPoint(checkName);
	}
}


