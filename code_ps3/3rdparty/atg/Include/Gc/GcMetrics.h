//--------------------------------------------------------------------------------------------------
/**
	@file		

	@brief		

	@note		(c) Copyright Sony Computer Entertainment 2004. All Rights Reserved.	
**/
//--------------------------------------------------------------------------------------------------

#ifndef GC_METRICS_H
#define GC_METRICS_H

#ifdef	ATG_PROFILE_ENABLED

//--------------------------------------------------------------------------------------------------
/**
	@class			GcMetrics
	
	@brief			Static management class for graphics related metrics data.
**/
//--------------------------------------------------------------------------------------------------

class	GcMetrics
{
public:

	// This class contains data associated with a single metrics item. 
	class	GcMetricsItem
	{
		friend class GcMetrics;

	public:	
		s64				GetValue( void )		{ return m_itemValue; }
		void			SetValue( s64 value )	{ m_itemValue = value; }
		void			AddValue( s64 value )	{ m_itemValue += value; }
		void			SetPersistency(bool p)	{ m_itemPersistency = p; }
		
	private:
		const char*		m_itemName;				///< Pointer to the metrics name (must be persistent)
		s64				m_itemValue;			///< Value associated with metrics
		bool			m_itemPersistency;		///< A persistency of true indicates the item value is not reset.
	};

	// These should not be called directly. Please use the macros below to access metrics data
	static	void			Reset( void );
	static	GcMetricsItem*	GetMetricsItem( const char* pName );

private:
	static	const int		kMaxMetricsItems = 32;

	static	int				ms_metricsCount;
	static	GcMetricsItem	ms_metricsItems[ kMaxMetricsItems ];
};
#endif	// ATG_PROFILE_ENABLED


//--------------------------------------------------------------------------------------------------
// The following macros are the only supported interface to the metrics system. While the calling
// style associated with the use of these macros may appear somewhat non-standard, the use of a
// scoped static pointer to the metrics item allows for rapid retrival/setting of metrics data on
// all but the first call. On non-profile builds, the macros melt away completely.

#ifdef	ATG_PROFILE_ENABLED

#define	GC_RESET_METRICS( )																	\
{																							\
	GcMetrics::Reset();																		\
}

#define	GC_GET_METRICS( name, var )															\
{																							\
	static	GcMetrics::GcMetricsItem*	pMetricsItem = GcMetrics::GetMetricsItem( name );	\
	var = pMetricsItem->GetValue();															\
}

#define	GC_SET_METRICS( name, value )														\
{																							\
	static	GcMetrics::GcMetricsItem*	pMetricsItem = GcMetrics::GetMetricsItem( name );	\
	pMetricsItem->SetValue( value );														\
}

#define	GC_ADD_METRICS( name, value )														\
{																							\
	static	GcMetrics::GcMetricsItem*	pMetricsItem = GcMetrics::GetMetricsItem( name );	\
	pMetricsItem->AddValue( value );														\
}

#define	GC_SET_METRICS_PERSISTENCY( name, p )												\
{																							\
	static	GcMetrics::GcMetricsItem*	pMetricsItem = GcMetrics::GetMetricsItem( name );	\
	pMetricsItem->SetPersistency( p );														\
}

#else

#define	GC_RESET_METRICS( )						{}
#define	GC_GET_METRICS( name, var )				{}
#define	GC_SET_METRICS( name, value )			{}
#define	GC_ADD_METRICS( name, value )			{}
#define	GC_SET_METRICS_PERSISTENCY( name, p )	{}

#endif	// ATG_PROFILE_ENABLED

#endif	// GC_METRICS_H
