/***************************************************************************************************
*
*	DESCRIPTION		Handles all things level-of-detail related.
*
*	NOTES
*
***************************************************************************************************/

#ifndef _LOD_H
#define _LOD_H

// Forward declarations
class CRenderableComponent;
class CCamera;
class CMeshInstance;
class AnyString;

/***************************************************************************************************
*
*	CLASS			CLODNode
*
*	DESCRIPTION		XML definition structure
*
***************************************************************************************************/
class CLODNode
{
public:
	CLODNode( void );
	void PostConstruct( void );

	float				m_fPercentCost;							// cost of this node in frame percent (0->1)
	float				m_fDistanceValid;						// distance this LOD is valid to

	typedef ntstd::List< AnyString*, Mem::MC_GFX > AnyStringList;
	typedef ntstd::List< CHashedString, Mem::MC_GFX > HashedStringList;

	AnyStringList		m_obMeshNames;					// 'any string' list 
	AnyStringList		m_obShadowMeshNames;			// 'any string' list 
	
	HashedStringList	m_obResolvedMeshNames;			// list of mesh names
	HashedStringList	m_obResolvedShadowMeshNames;	// list of shadow casting mesh names
};

/***************************************************************************************************
*
*	CLASS			CLODComponentDef
*
*	DESCRIPTION		This is either serialised or constructed from a lua script
*
***************************************************************************************************/
class CLODComponentDef
{
public:
	typedef ntstd::List< CLODNode*, Mem::MC_GFX > LODNodeList;
	LODNodeList					m_obLODNodes;
	bool						m_bDistanceLOD;
};

/***************************************************************************************************
*
*	CLASS			LODPreset
*
*	DESCRIPTION		Class used to associate a LOD with a clump name automatically
*
***************************************************************************************************/
class LODPreset
{
public:
	ntstd::String		m_clumpName;
	CLODComponentDef*	m_pLODDef;
};

/***************************************************************************************************
*
*	CLASS			CCostIndex
*
*	DESCRIPTION		glue struct to get around non sorted nature of LOD list
*
***************************************************************************************************/
struct	CCostIndex
{
	CCostIndex( void ) {};
	CCostIndex( float f, u_int i ) : fCost(f), uiIndex(i) {};
	float	fCost;
	u_int	uiIndex;
};

/***************************************************************************************************
*
*	CLASS			CLODComponent
*
*	DESCRIPTION		Entity component that handles LOD related shennaigans
*
*	NOTES			externally we present LOD 0 as the costliest, and LOD (GetNumLOD() - 1) as the cheapest
*
***************************************************************************************************/
class CLODComponent
{
public:
	CLODComponent( CRenderableComponent *renderable_component, const CLODComponentDef* pobDef );
	~CLODComponent( void );

	// return cost of this LOD
	float	GetLODCost( u_int uiIndex ) const
	{
		ntAssert( uiIndex < GetNumLOD() );
		return m_paobCostTable[uiIndex].fCost;
	}

	// return the info struct for this LOD
	const	CLODNode&	GetLODInfo( u_int uiLOD ) const;

	// get how may LOD's we have
	u_int	GetNumLOD( void ) const { return m_uiNumLOD; }
	u_int	GetCurrLOD( void ) const { return m_uiCurrLOD; }
	u_int	GetLODOnDistance( const CPoint& obPos ) const;

	// set what LOD we're using
	void	SetCurrLOD( u_int uiLOD );
	void	SetToLowestLOD( void )					{ SetCurrLOD( GetNumLOD() - 1 ); }
	void	SetToHighestLOD( void )					{ SetCurrLOD( 0 ); }
	void	SetOnDistance( const CPoint& obPos )	{ SetCurrLOD( GetLODOnDistance(obPos) ); }
	void	ForceLODOffset( int iNewOffset )		{ m_iForcedLODOffset = iNewOffset; }
	void	RequestLODSwitch( u_int uiLOD );

	// parent entity queries
	bool	DistanceLOD( void ) const { return m_pobDef->m_bDistanceLOD; }
	bool	Visible() const;
	CPoint	GetEntPos( void ) const;

	// LODs sorting stuff
	void	ComputeSortKey( CPoint& obWhat )		{ m_fSortKey = ( GetEntPos() - obWhat ).LengthSquared(); }
	float	GetSortKey( void ) const { return m_fSortKey; }

	void	DebugRender( const CCamera* pobCamera ) const;

private:
	// turn external LOD level into real list index 
	u_int	TranslateLODIndex( u_int uiIndex ) const
	{
		ntAssert( uiIndex < GetNumLOD() );
		return m_paobCostTable[uiIndex].uiIndex;
	}

	CRenderableComponent *		m_pobRenderableComponent;
	const	CLODComponentDef*	m_pobDef;
	
	u_int	m_uiCurrLOD;
	u_int	m_uiNumLOD;
	int		m_iForcedLODOffset;
	int		m_iLastReqTick;

	float	m_fSortKey;

	class CComparator_CostIndex_GreaterThan
	{
	public:
		bool operator()( const CCostIndex* pobFirst, const CCostIndex* pobSecond ) const
		{
			return pobFirst->fCost > pobSecond->fCost;
		}
	};

	CCostIndex*	m_paobCostTable;

	// List of all meshes. Unfortunate, but nessecary for visibility
	typedef ntstd::List< const CMeshInstance*, Mem::MC_GFX > MeshInstanceList;
	MeshInstanceList m_obMeshList;
};

/***************************************************************************************************
*
*	CLASS			CLODManagerDef
*
*	DESCRIPTION		This is either serialised or constructed from a lua script
*
***************************************************************************************************/
class CLODManagerDef
{
public:
	void PostConstruct( void );

	float					m_fLODBudget;
	typedef ntstd::List<LODPreset*, Mem::MC_GFX> LODPresetList;
	LODPresetList	m_presets;
};

/***************************************************************************************************
*
*	CLASS			CLODManager
*
*	DESCRIPTION		Singleton that manages the LOD budget for a given level.
*
*	NOTES			Though the actual LOD budget could vary over a given level, for simplicities sake
*					it is currently fixed.
*
***************************************************************************************************/
class CLODManager : public Singleton<CLODManager>
{
public:
	CLODManager( void );

	void	SetLODs( const CCamera* pobCamera );

	void	Register( CLODComponent* pobComp );
	void	UnRegister( CLODComponent* pobComp );

	void	SetLODBudget( float fBudget )
	{
		ntAssert( fBudget >= 0.0f );
		m_fTotalLODBudget = fBudget;
	}

	int		GetNumLastVisible( void ) const { return m_obVisibleSet.size(); }
	float	GetLODBudget( void ) const { return m_fTotalLODBudget; }
	float	GetUsedBudget( void ) const { return m_fTotalUsedBudget; }

	static bool	m_bDebugRender;

	// debug overide
	void	ForceLODOffset( int iNewOffset );
	int		GetForcedLODOffset() { return m_iForcedLODOffset; }

	// LOD preset structures
	void AddLODPreset( const char* pFullClumpName, const CLODComponentDef* pDef );
	const CLODComponentDef* GetLODPreset( const char* pFullClumpName );

private:
	static u_int GetKeyFromString( const char* pString );

	float	m_fTotalLODBudget;	// this is expressed in percentage of a frame (0 -> 1)
	float	m_fTotalUsedBudget;	// this is expressed in percentage of a frame (0 -> 1)
	typedef ntstd::List< CLODComponent*, Mem::MC_GFX > LOCComponentList;
	LOCComponentList m_obLODComponents; // all ents registered for LODing

	// now we sort the visible set
	class CComparator_LODComponent_CloserThan
	{
	public:
// there's no need to compute our sort key in the comparator anymore cause we're computing it before sorting our vectors (it improves performance)
//		explicit CComparator_LODComponent_CloserThan( const CPoint& obWhat ) : m_obWhat( obWhat ) {}
		explicit CComparator_LODComponent_CloserThan() {}

		bool operator()( const CLODComponent* pobFirst, const CLODComponent* pobSecond ) const
		{
			return	(  pobFirst->GetSortKey() < pobSecond->GetSortKey() );
		}

	private:
//		CPoint const& m_obWhat;
	};

	typedef ntstd::Vector<CLODComponent*, Mem::MC_GFX> LODComponentVector;
	typedef ntstd::Vector<u_int, Mem::MC_GFX> LODUIntVector;
	LODComponentVector		m_obVisibleSet;
	LODComponentVector		m_obToSortSet;
	LODUIntVector			m_obDynamicLODS;
	int						m_iForcedLODOffset;

	typedef ntstd::Map<u_int, const CLODComponentDef*> LODPresetMap;
	LODPresetMap m_LODPresets;
};

#endif // _LOD_H
