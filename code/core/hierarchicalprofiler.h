#ifndef CORE_HIERARCHICALPROFILER_H
#define CORE_HIERARCHICALPROFILER_H

#ifdef _PROFILING

#ifdef _HIERARCHICAL_PROFILER_TESTING
	// This hackery enables me to work on the profiler outside of the game code,
	// significantly decreasing the compile-link-run time between iterations. /Mats
	typedef long int64_t;
	typedef unsigned int uint32_t;
	typedef int LogFile;
	
	#include <list>
	#include <vector>
	#include <algorithm>

	namespace ntstd
	{
		using std::sort;
		template<typename T> inline T Max(T const& a, T const& b) { return (a > b) ? a : b; }
	}
#else
	#include "core/nt_std.h"
#endif


//--------------------------------------------------
//
//! Hierarchical Profiler - Traces flagged function calls and outputs call time statistics.
//! Use the LOAD_PROFILE() macros below so we don't leave a lot of junk in the _MASTER build.
//!
//! Hierarchical profiling induces a slight (imperceptable) overhead when sampling, as we
//! have to do some rummaging around in lists and whatnot to trap recursive function calls
//! already on the call stack. While this overhead is surprisingly slight (barely measurable,
//! in fact), you probably shouldn't use it inside that tiny function you decided to inline.
//!
//! It's really useful for load-time profiling, though. Here's a quick guide on how to use it:
//!
//! Each Profiler sample is identified by a pointer to a constant C string. The LOAD_PROFILE()
//! macro creates this string for you. Since the compiler by default only allocates memory for
//! identical string constants once, we can perform pointer comparisons instead of comparing
//! the actual characters inside the strings.
//!
//! Start profiling like this:
//! 
//!		LOAD_PROFILE( UniqueLabel )
//!
//! You don't have to END a profile. The Sample is stopped when the HierarchicalProfiler is
//! destroyed at the end of the scope:
//!
//!		bool TextureManager::LoadTexture( const char* pTexture )
//!		{
//!			LOAD_PROFILE( TextureManager_LoadTexture )
//!			... gah, get your texture loading on!
//!			return bSuccess;
//!		}
//! 
//! ...but you can also stop it explicitly.
//! This if useful when you've got several operations in the same scope:
//!
//!		LOAD_PROFILE( ConvertPixelFormat )
//!		... pixel-format conversion action!
//!		LOAD_PROFILE_END( ConvertPixelFormat )
//!
//! Once you're ready to output the sampled data, just put this at the end:
//!
//!		LOAD_PROFILE_OUTPUT_RESULTS
//! 
//! This prints the profiling data to the console, and disables any further profiling.
//
//--------------------------------------------------
class HierarchicalProfiler
{
private:
	class Sample;

public:
#ifdef _HIERARCHICAL_PROFILER_TESTING
		typedef std::vector< const Sample* > SampleList;
#else
		typedef ntstd::Vector< const Sample* > SampleList;
#endif

	HierarchicalProfiler( const char* pSampleLabel );
	~HierarchicalProfiler();
	void Stop();

	static void OutputResults();

private:
	//--------------------------------------------------
	//
	//! Represents a node in the hierarchical function call tree.
	//
	//--------------------------------------------------
	class Sample
	{
	public:
		//--------------------------------------------------
		//
		//! Functor for sorting Sample pointers.
		//! Sorts the Samples in descending order based on their accumulated time.
		//
		//--------------------------------------------------
		struct PointerSorter
		{
			bool operator()( const Sample* pobFirst, const Sample* pobSecond ) const;
		};

		Sample( const char* pLabel, Sample* pParent );
		bool operator<( const Sample& obOther ) const;

		void Start( int64_t iStartTime );
		void Stop( int64_t iEndTime );
		inline void IncreaseTime( float fSeconds ) { m_iAccumulatedTime += AsTicks( fSeconds ); }
		inline void IncreaseCallCount( uint32_t uiCalls = 1 ) { m_uiCallCount += uiCalls; }
		
		inline bool IsRunning() const { return m_iStartTime != 0; }
		float AccumulatedTime( int64_t iCurrentTime ) const;
		uint32_t CallCount() const;
		uint32_t ParentCount() const;
		Sample* Parent();
		const Sample* Parent() const;
		Sample& Child( const char* pLabel );
		SampleList SortedChildren() const;
		Sample* FindSampleAbove( const char* pLabel );
		void GroupSamplesInto( Sample& obReceivingSample ) const;
		
		uint32_t LabelWidth() const;
		uint32_t IndentedLabelWidth() const;
		uint32_t WidestLabelColumnWidth( bool bIndented = false ) const;
		void Output( uint32_t uiLabelColumnWidth, bool bIndented = false ) const;
		void OutputTree( uint32_t uiLabelColumnWidth ) const;
		
	private:
#ifdef _HIERARCHICAL_PROFILER_TESTING
		typedef std::list< Sample > ChildList;
#else
		typedef ntstd::List< Sample > ChildList;
#endif

		static float AsSeconds( int64_t uiTicks );
		static int64_t AsTicks( float fSeconds );

		ChildList m_obChildren;
		int64_t m_iStartTime;
		int64_t m_iAccumulatedTime;
		const char* m_pLabel;
		uint32_t m_uiCallCount;
		Sample* m_pobParent;
	};

	static void AllocateSampleTree();
	static void FreeSampleTree();
	
	static void OutputString( const char* pString );
	static void OutputGroupedSamples();
	
	Sample* m_pobSample;
	
	static Sample* s_pobSampleTree;
	static Sample* s_pobCurrentContext;
	static LogFile* s_pobLogFile;
	static bool s_bEnabled;
};

#endif // _PROFILING

#if defined( _PROFILING ) && defined( _LOAD_PROFILING )

// Use these when profiling load times - they will evaluate to nothing in a _MASTER build.
#define LOAD_PROFILE( sample_id ) HierarchicalProfiler HierarchicalProfiler_Sample_##sample_id( #sample_id );
#define LOAD_PROFILE_END( sample_id ) HierarchicalProfiler_Sample_##sample_id .Stop();
#define LOAD_PROFILE_OUTPUT_RESULTS HierarchicalProfiler::OutputResults();

#else

#define LOAD_PROFILE( sample_id )
#define LOAD_PROFILE_END( sample_id )
#define LOAD_PROFILE_OUTPUT_RESULTS

#endif // _PROFILING

#endif // CORE_HIERARCHICALPROFILER_H
