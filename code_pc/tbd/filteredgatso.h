#ifndef _FILTEREDGATSO_H_
#define _FILTEREDGATSO_H_

#include "core/rotationnalindex.h"
#include "core/explicittemplate.h"
#include "core//bitmask.h"
#include "core/timer.h"


class StringFilter
{
public:
	virtual bool Match(const ntstd::String& str) const = 0;
	StringFilter() {};
	virtual ~StringFilter() {};
}; // end of class StringFilter

class StringFilterSubstring: StringFilter
{
public:
	ntstd::String m_substring;
public:
	virtual bool Match(const ntstd::String& str) const
	{
		int iRes = str.find(m_substring);
		return iRes>=0;
	}
	StringFilterSubstring(const ntstd::String& substring)
		:StringFilter()
		,m_substring(substring)
	{
		// nothing
	}
}; // end of class StringFilterSubstring

//--------------------------------------------------
//!
//!	performance class
//!	redundant with the one in gatso.h
//!
//--------------------------------------------------


class FilteredGatso
{
// type
private: 
	// filter size
	static const int Size = 30;

	// rotationnal index (just an easy way to use a modulo index on an array)
	typedef RotationnalIndex<int,Size> RotIndex;
	
	// record perf class
	class Record
	{
	public:
		class CheckPoint
		{
		public:
			ntstd::String m_name;
			LONGLONG m_lElapsed;
		public:
			CheckPoint(const ntstd::String& name, LONGLONG lElapsed)
				:m_name(name)
				,m_lElapsed(lElapsed)
			{}
			float GetTime(float fTimeCoef) const
			{
				return m_lElapsed * fTimeCoef;
			}
		}; // end of class CheckPoint
		
		typedef ntstd::List<CheckPoint> CheckPointList;
		typedef Array<float,Size> SampleArray;
	public:
		LONGLONG m_lAccum;
		LONGLONG m_lStart;
		SampleArray m_array;
		float m_fAverage;
		bool m_bFirstUpdate;
		int m_iNbTick;
		CheckPointList m_checkPointList;
	public:
		void Start();
		void Stop();
		void AddCheckPoint(const ntstd::String& checkName);
		void Update(const RotIndex& p, float fTimeCoef);
		Record();
	private:
		float RealAverage();  // do not use, debug purpose
	}; // end of class Record
	
	struct RecordCmp
	{
		bool operator()(const ntstd::String& s1, const ntstd::String& s2) const
		{
			return strcmp(s1.c_str(), s2.c_str()) < 0;
		}
	};	

	typedef ntstd::Map<ntstd::String, Record, RecordCmp> Map;
	typedef ntstd::List<Map::value_type*> TmpList;

	// sorting:
	struct AverageSort
	{
		bool operator()(const FilteredGatso::Map::value_type* p1, const FilteredGatso::Map::value_type* p2) const
		{
			return p1->second.m_fAverage > p2->second.m_fAverage;
		}
	};
	
	// sorting:
	struct NameSort
	{
		bool operator()(const FilteredGatso::Map::value_type* p1, const FilteredGatso::Map::value_type* p2) const
		{
			return strcmp(p1->first.c_str(), p2->first.c_str()) < 0;
		}
	};

	///////////////////////////////////////////////////
	// Group boolean
	typedef enum
	{
		F_ENABLE = BITFLAG(0),     // first draw, init me first
		F_AVERAGESORT = BITFLAG(1), // draw collision sphere
		F_NAMESORT = BITFLAG(1), // draw collision sphere
		F_CHECKPOINT = BITFLAG(2), // use checkpoint
	} State;

// member
private:	
	// origin on the screen
	Vec2 m_origin;
	
	// booleans
	BitMask<State> m_mask;
	
	// container of record
	Map m_map;
	
	// index for modulo array
	RotIndex m_rotIndex;
	
private:
	// draw
	void Draw();
	void Draw(const TmpList& list);
	void CreateTmpList(TmpList& list);
	
	// return the perod of one frame * 100
	static float GetTimeCoef()
	{
		return CTimer::GetHWTimerPeriodFrame() * 100.0f;;
	}
	
	CScopedPtr<const StringFilter> m_filter;

public:
	void Update();
	void Start(const ntstd::String &);
	void Stop(const ntstd::String &);
	void CheckPoint(const ntstd::String &, const ntstd::String &);
	void SetNameFilter(const StringFilter* pFilter) // can be zero for no filter
	{
		m_filter.Reset(pFilter);
	}
	void Scroll(float fY)
	{
		m_origin[1] += fY;
	}
	
	BitMask<State>& GetMask() {return m_mask;}
	const BitMask<State>& GetMask() const {return m_mask;}
	
	//! constructor
	FilteredGatso();
	
}; // end of class FilteredGatso


#endif // end of _FILTEREDGATSO_H_