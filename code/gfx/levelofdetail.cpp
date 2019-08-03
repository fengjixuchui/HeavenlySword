/***************************************************************************************************
*
*	DESCRIPTION		Handles all things level-of-detail related.
*
*	NOTES
*
***************************************************************************************************/

#include "gfx/levelofdetail.h"
#include "game/renderablecomponent.h"
#include "anim/transform.h"
#include "game/entityinfo.h"
#include "gfx/camera.h"
#include "core/listtools.h"
#include "core/visualdebugger.h"
#include "editable/anystring.h"

// for visbility stuff only
#include "gfx/meshinstance.h"
#include "core/frustum.h"

// for movie capture
#include "game/capturesystem.h"

#include "objectdatabase/dataobject.h"

#include "core/visualdebugger.h"

#include "core/gatso.h"

START_CHUNKED_INTERFACE( CLODManagerDef, Mem::MC_GFX )
	PUBLISH_VAR_AS(	m_fLODBudget, LODBudget )
	PUBLISH_PTR_CONTAINER_AS ( m_presets, Presets );
	DECLARE_POSTCONSTRUCT_CALLBACK( PostConstruct )
END_STD_INTERFACE

START_CHUNKED_INTERFACE( CLODNode, Mem::MC_GFX )
	PUBLISH_VAR_AS(	m_fPercentCost,		PercentCost )
	PUBLISH_VAR_AS(	m_fDistanceValid,	DistanceValid )
	PUBLISH_PTR_CONTAINER_AS ( m_obMeshNames, MeshNames )
	PUBLISH_PTR_CONTAINER_AS ( m_obShadowMeshNames, ShadowMeshNames )
	DECLARE_POSTCONSTRUCT_CALLBACK( PostConstruct )
END_STD_INTERFACE

START_CHUNKED_INTERFACE( CLODComponentDef, Mem::MC_GFX )
	PUBLISH_PTR_CONTAINER_AS ( m_obLODNodes, LODNodes )
	PUBLISH_VAR_AS(	m_bDistanceLOD,	DistanceLOD )
END_STD_INTERFACE

START_CHUNKED_INTERFACE( LODPreset, Mem::MC_GFX )
	PUBLISH_VAR_AS(	m_clumpName,	FullClumpName )
	PUBLISH_PTR_AS(	m_pLODDef,		LODDefinition )
END_STD_INTERFACE

bool	CLODManager::m_bDebugRender = false;

/***************************************************************************************************
*
*	FUNCTION		ResolveBaseListToKeyStringType
*
*	DESCRIPTION		Function template for resolving lists of CBases to a given pointer type
*
***************************************************************************************************/
static void	ResolveBaseListToKeyStringType( const CLODNode::AnyStringList& obBases, CLODNode::HashedStringList& obResults )
{
	for (	CLODNode::AnyStringList::const_iterator obIt = obBases.begin();
			obIt != obBases.end(); ++obIt )
	{
#ifndef _RELEASE
		DataObject* pDO = ObjectDatabase::Get().GetDataObjectFromPointer( *obIt );
		ntError( pDO );
		ntError( stricmp( pDO->GetClassName(), "AnyString" ) == 0 );
#endif

		obResults.push_back( ntStr::GetString(( *obIt )->m_obString) );
	}
} 

/***************************************************************************************************
*
*	FUNCTION		CLODNode::CLODNode
*
*	DESCRIPTION		construct
*
***************************************************************************************************/
CLODNode::CLODNode( void )
{
	m_fPercentCost = 0.0f;
	m_fDistanceValid = 5.0f;
}

/***************************************************************************************************
*
*	FUNCTION		CLODNode::PostConstruct
*
*	DESCRIPTION		patch up the real lists
*
***************************************************************************************************/
void	CLODNode::PostConstruct( void )
{
	ResolveBaseListToKeyStringType( m_obMeshNames, m_obResolvedMeshNames );
	ResolveBaseListToKeyStringType( m_obShadowMeshNames, m_obResolvedShadowMeshNames );
}


/***************************************************************************************************
*
*	FUNCTION		CLODComponent::CLODComponent
*
*	DESCRIPTION		construct
*
***************************************************************************************************/
CLODComponent::CLODComponent( CRenderableComponent *renderable_component, const CLODComponentDef* pobDef )
:	m_pobRenderableComponent( renderable_component )
{
	m_pobDef = pobDef;

	ntAssert(m_pobRenderableComponent);
	ntAssert(m_pobDef);

	CLODManager::Get().Register( this );

	// we're going to treat our LOD list as static.
	m_uiNumLOD = m_pobDef->m_obLODNodes.size();
	
	// build our sorted cost list.
	typedef ntstd::List< CCostIndex*, Mem::MC_GFX > CostIndexList;
	CostIndexList obTempList;

	u_int uiIndex = 0;
	for (	CLODComponentDef::LODNodeList::const_iterator obIt =  m_pobDef->m_obLODNodes.begin();
			obIt != m_pobDef->m_obLODNodes.end(); ++obIt, uiIndex++ )
	{
		CCostIndex* pobTempObj = NT_NEW_CHUNK( Mem::MC_GFX ) CCostIndex( (*obIt)->m_fPercentCost, uiIndex );
		obTempList.push_back( pobTempObj );
	}

	bubble_sort( obTempList.begin(), obTempList.end(), CComparator_CostIndex_GreaterThan() );

	// insert it into our table and cleanup
	m_paobCostTable = NT_NEW_ARRAY_CHUNK( Mem::MC_GFX ) CCostIndex[ GetNumLOD() ];

	uiIndex = GetNumLOD();
	while (!obTempList.empty())
	{
		--uiIndex;

		m_paobCostTable[uiIndex] = *obTempList.back();
		NT_DELETE_CHUNK( Mem::MC_GFX, obTempList.back() );

		obTempList.pop_back();
	}

	// default to the lowest LOD
	m_uiCurrLOD = GetNumLOD();
	m_iForcedLODOffset = 0;
	SetToLowestLOD();

	// now build our list of renderables for visibility checks
	for (	CLODComponentDef::LODNodeList::const_iterator obIt = m_pobDef->m_obLODNodes.begin();
			obIt != m_pobDef->m_obLODNodes.end(); ++obIt )
	{
		for (	CLODNode::HashedStringList::const_iterator obNames = (*obIt)->m_obResolvedMeshNames.begin();
				obNames != (*obIt)->m_obResolvedMeshNames.end(); ++obNames )
		{
			const CMeshInstance* pMesh = m_pobRenderableComponent->GetMeshByMeshName( (*obNames) );
			if  ( pMesh != NULL )
				m_obMeshList.push_back( pMesh );
		}
	}

	m_iLastReqTick = 0;
}

/***************************************************************************************************
*
*	FUNCTION		CLODComponent::~CLODComponent
*
*	DESCRIPTION		clean
*
***************************************************************************************************/
CLODComponent::~CLODComponent( void )
{
	CLODManager::Get().UnRegister( this );
	NT_DELETE_ARRAY_CHUNK( Mem::MC_GFX, m_paobCostTable );
}

/***************************************************************************************************
*
*	FUNCTION		CLODComponent::GetLODInfo
*
*	DESCRIPTION		retirive LOD def
*
***************************************************************************************************/
const CLODNode&	CLODComponent::GetLODInfo( u_int uiLOD ) const
{
	uiLOD = TranslateLODIndex( uiLOD );

	CLODComponentDef::LODNodeList::const_iterator obIt( m_pobDef->m_obLODNodes.begin() );
	ntstd::advance( obIt, uiLOD );

	return *(*obIt);
}

/***************************************************************************************************
*
*	FUNCTION		CLODComponent::RequestLODSwitch
*
*	DESCRIPTION		Called only for dynamic LODs, attempts to smooth transition...
*
***************************************************************************************************/
void	CLODComponent::RequestLODSwitch( u_int uiLOD )
{
	// we're overiding so switch imediately
	if (m_iForcedLODOffset)
	{
		SetCurrLOD(uiLOD);
	}
	else
	{
		uiLOD = (u_int)ntstd::Clamp( (int)uiLOD, 0, (int)(m_uiNumLOD-1) );

		// we're degrading in performance, so check to make sure enough time has elapsed
		if (uiLOD > m_uiCurrLOD)
		{
			m_iLastReqTick--;
			if (m_iLastReqTick > 0)
				return;

			static const uint iDegradeTicks = 120;
			m_iLastReqTick = iDegradeTicks;
		}
		
		SetCurrLOD(uiLOD);
	}	
}


/***************************************************************************************************
*
*	FUNCTION		CLODComponent::SetCurrLOD
*
*	DESCRIPTION		set our current LOD
*
***************************************************************************************************/
void	CLODComponent::SetCurrLOD( u_int uiLOD )
{
	uiLOD = (u_int)ntstd::Clamp( (int)uiLOD + m_iForcedLODOffset, 0, (int)(m_uiNumLOD-1) );

	if (uiLOD == m_uiCurrLOD) 
		return;

	if (m_uiCurrLOD == GetNumLOD())
	{
		// turn off all the current meshes
		m_pobRenderableComponent->DisableAllRenderables();
	}
	else
	{
		// turn off all the old LOD meshes
		const CLODNode*	pobDef = &GetLODInfo( m_uiCurrLOD );

		for (	CLODNode::HashedStringList::const_iterator obIt = pobDef->m_obResolvedMeshNames.begin();
				obIt != pobDef->m_obResolvedMeshNames.end(); ++obIt )
		{
			m_pobRenderableComponent->EnableRenderByMeshName( (*obIt), false );
		}

		for (	CLODNode::HashedStringList::const_iterator obIt = pobDef->m_obResolvedShadowMeshNames.begin();
				obIt != pobDef->m_obResolvedShadowMeshNames.end(); ++obIt )
		{
			m_pobRenderableComponent->EnableShadowCastByMeshName( (*obIt), false );
		}
	}

	// turn on all the new LOD meshes
	m_uiCurrLOD = uiLOD;

	const CLODNode*	pobDef = &GetLODInfo( m_uiCurrLOD );
	
	// turn on all the current ones we want for this LOD
	for (	CLODNode::HashedStringList::const_iterator obIt = pobDef->m_obResolvedMeshNames.begin();
			obIt != pobDef->m_obResolvedMeshNames.end(); ++obIt )
	{
		m_pobRenderableComponent->EnableRenderByMeshName( (*obIt), true );
	}

	for (	CLODNode::HashedStringList::const_iterator obIt = pobDef->m_obResolvedShadowMeshNames.begin();
			obIt != pobDef->m_obResolvedShadowMeshNames.end(); ++obIt )
	{
		m_pobRenderableComponent->EnableShadowCastByMeshName( (*obIt), true );
	}
}

/***************************************************************************************************
*
*	FUNCTION		CLODComponent::GetLODOnDistance
*
*	DESCRIPTION		Get recomneded LOD for our distance
*
***************************************************************************************************/
u_int	CLODComponent::GetLODOnDistance( const CPoint& obPos ) const
{
	float fDistance = (obPos - GetEntPos()).Length();
	u_int uiBest = GetNumLOD() - 1;
	float fBest = MAX_POS_FLOAT;

	for (u_int i = 0; i < GetNumLOD(); i++)
	{
		float fDistValid = GetLODInfo(i).m_fDistanceValid;
		if ((fDistance < fDistValid) && (fDistValid < fBest))
		{
			uiBest = i;
			fBest = fDistValid;
		}
	}
	return uiBest;
}

/***************************************************************************************************
*
*	FUNCTION		CLODComponent::Visible
*
*	DESCRIPTION		visit our renderables to see if we're visible within this frustrum
*
***************************************************************************************************/
bool	CLODComponent::Visible() const
{
	if (m_pobRenderableComponent && !m_pobRenderableComponent -> IsSuspended())
	{
		for (	MeshInstanceList::const_iterator obIt = m_obMeshList.begin();
				obIt != m_obMeshList.end(); ++obIt )
		{
			if	(
				(*obIt)->IsRendering() &&
				((*obIt)->m_FrameFlags & CRenderable::RFF_VISIBLE )
				)
				return true;
		}
	}
	return false;
}

/***************************************************************************************************
*
*	FUNCTION		CLODComponent::GetEntPos
*
*	DESCRIPTION		retrieve the position of our parent
*
***************************************************************************************************/
CPoint	CLODComponent::GetEntPos( void ) const
{
	return m_pobRenderableComponent->GetPosition();
}

/***************************************************************************************************
*
*	FUNCTION		CLODComponent::DebugRender
*
*	DESCRIPTION		Show what LOD we're on visibly
*
***************************************************************************************************/
void	CLODComponent::DebugRender( const CCamera* ) const
{
#ifndef _GOLD_MASTER
	CPoint obPos = m_pobRenderableComponent->GetPosition() + CPoint( 0.0f, 2.0f, 0.0f );

	switch ( m_uiCurrLOD )
	{
	case 0:	g_VisualDebug->Printf3D( obPos, NTCOLOUR_ARGB(0xff,0xff,0xff,0xff), 0, "0" ); break;
	case 1:	g_VisualDebug->Printf3D( obPos, NTCOLOUR_ARGB(0xff,0xff,0x00,0x00), 0, "1" ); break;
	case 2:	g_VisualDebug->Printf3D( obPos, NTCOLOUR_ARGB(0xff,0x00,0xff,0x00), 0, "2" ); break;
	case 3:	g_VisualDebug->Printf3D( obPos, NTCOLOUR_ARGB(0xff,0x00,0x00,0xff), 0, "3" ); break;
	}
#endif
}





/***************************************************************************************************
*
*	FUNCTION		CLODManagerDef::CLODManagerDef
*
*	DESCRIPTION		construct
*
***************************************************************************************************/
void CLODManagerDef::PostConstruct( void )
{
	CLODManager::Get().SetLODBudget( m_fLODBudget );

	for (	LODPresetList::iterator it = m_presets.begin();
			it != m_presets.end(); ++it )
	{
		if ((*it)->m_pLODDef)
			CLODManager::Get().AddLODPreset( ntStr::GetString((*it)->m_clumpName), (*it)->m_pLODDef );
	}
}




/***************************************************************************************************
*
*	FUNCTION		CLODManager::CLODManager
*
*	DESCRIPTION		construct
*
***************************************************************************************************/
CLODManager::CLODManager( void ) :
	m_obVisibleSet( 200 ),
	m_obToSortSet( 200 ),
	m_obDynamicLODS( 200 )
{
	m_fTotalLODBudget = 1.0f;
	m_fTotalUsedBudget = 0.0f;
	m_bDebugRender = false;
	m_iForcedLODOffset = 0;
}

/***************************************************************************************************
*
*	FUNCTION		CLODManager::GetKeyFromString
*
*	DESCRIPTION		util function
*
***************************************************************************************************/
u_int CLODManager::GetKeyFromString( const char* pString )
{
	static char aString[ MAX_PATH ];
	static char aFinal[ MAX_PATH ];
	strcpy( aString, pString );

	// set to lower case
	ntstd::transform( aString, aString + strlen( aString ), aString, &ntstd::Tolower );
	strcpy( aFinal, aString );

	// strip out any path type slashes
	char* pNext = strstr( aFinal, "\\" );
	while (pNext)
	{
		char* pAfter = pNext+1;
		strcpy( aString, pAfter );
		strcpy( pNext, aString );
		pNext = strstr( pNext, "\\" );
	}

	pNext = strstr( aFinal, "/" );
	while (pNext)
	{
		char* pAfter = pNext+1;
		strcpy( aString, pAfter );
		strcpy( pNext, aString );
		pNext = strstr( pNext, "/" );
	}

	CHashedString key(aFinal);
	return key.GetValue();
}

/***************************************************************************************************
*
*	FUNCTION		CLODManager::AddLODPreset
*
*	DESCRIPTION		cache a LOD component def (only accepts XML defined LOD defs)
*
***************************************************************************************************/
void CLODManager::AddLODPreset( const char* pFullClumpName, const CLODComponentDef* pDef )
{
	ntAssert(pFullClumpName);
	ntAssert(pDef);

	DataObject* pDataObject = ObjectDatabase::Get().GetDataObjectFromPointer( pDef );
	if ((pDataObject) && (strcmp( pDataObject->GetClassName(), "CLODComponentDef" ) == 0))
	{
		m_LODPresets[ GetKeyFromString(pFullClumpName) ] = pDef;
	}
}

/***************************************************************************************************
*
*	FUNCTION		CLODManager::GetLODPreset
*
*	DESCRIPTION		retrieve a LOD component def
*
***************************************************************************************************/
const CLODComponentDef* CLODManager::GetLODPreset( const char* pFullClumpName )
{
	LODPresetMap::iterator mapIt = m_LODPresets.find( GetKeyFromString(pFullClumpName) );
	if ( mapIt != m_LODPresets.end() )
		return mapIt->second;
	return 0;
}

/***************************************************************************************************
*
*	FUNCTION		CLODManager::ForceLODOffset
*
*	DESCRIPTION		DEBUG: allow us to overide what LOD we're really at
*
***************************************************************************************************/
void	CLODManager::ForceLODOffset( int iNewOffset )
{
	m_iForcedLODOffset = iNewOffset;
	for (	LOCComponentList::iterator obIt = m_obLODComponents.begin();
			obIt != m_obLODComponents.end(); ++obIt )
	{
		(*obIt)->ForceLODOffset(m_iForcedLODOffset);
	}	
}

/***************************************************************************************************
*
*	FUNCTION		CLODManager::SetLODs
*
*	DESCRIPTION		set the LOD's of all registerd componets for the next frame based on this frames
*					visible renderable set and our current LOD budget.
*
***************************************************************************************************/
void	CLODManager::SetLODs( const CCamera* pobCamera )
{
	ntAssert(pobCamera);

	bool bForceHighest = false;

	if (CaptureSystem::Exists() && CaptureSystem::Get().IsCapturing())
		bForceHighest = true;

	if (bForceHighest)
	{
		for (	LOCComponentList::iterator obIt = m_obLODComponents.begin();
				obIt != m_obLODComponents.end(); ++obIt )
		{
			(*obIt)->SetToHighestLOD();
		}
		return;
	}

	CGatso::Start("CLODManager::SetLODs");

	// get the visible set of LODable entities
	//m_obVisibleSet.clear();
	m_obVisibleSet.resize(0);

	for (	LOCComponentList::iterator obIt = m_obLODComponents.begin();
			obIt != m_obLODComponents.end(); ++obIt )
	{
		if ((*obIt)->Visible())
			m_obVisibleSet.push_back( *obIt );
		else
			(*obIt)->SetToLowestLOD();
	}

	CGatso::Stop("CLODManager::SetLODs");		
	
	CPoint obCamPos = pobCamera->GetViewTransform()->GetLocalTranslation();

	CGatso::Start("CLODManager::SortLODs");		
	// right, simplest algorithm is to ignore our budgets and just set to a particular LOD on distance
	//m_obToSortSet.clear();
	m_obToSortSet.resize(0);

	for ( unsigned int i = 0; i < m_obVisibleSet.size(); i++ )
	{
		if ( m_obVisibleSet[i]->DistanceLOD() )
		{
			m_obVisibleSet[i]->SetOnDistance( obCamPos );

			if (m_bDebugRender)
				m_obVisibleSet[i]->DebugRender( pobCamera );
		}
		else
		{
			m_obToSortSet.push_back( m_obVisibleSet[i] );
		}
	}
	
	if (m_obToSortSet.empty() )
	{
		CGatso::Stop("CLODManager::SortLODs");		
		return;
	}



	// we precompute the sort key before sorting this vector (it improves sorting speed a lot!)
	unsigned int uiVectorSize = m_obToSortSet.size();
	for ( unsigned int i = 0; i < uiVectorSize; i++ )
		m_obToSortSet[i]->ComputeSortKey( obCamPos );

	// sort the remainder to be closest first
	ntstd::sort(	&m_obToSortSet[0], &m_obToSortSet[0] + m_obToSortSet.size(),
				CComparator_LODComponent_CloserThan() );

	CGatso::Stop("CLODManager::SortLODs");		

	// now we assign default LOD values based on distance, and either improve or degrade untill our budget is used up.
	float	fTotalCost = 0.0f;

	//m_obDynamicLODS.clear();
	m_obDynamicLODS.resize(0);

	for ( unsigned int i = 0; i < m_obToSortSet.size(); i++ )
	{
		u_int uiLOD = m_obToSortSet[i]->GetLODOnDistance( obCamPos );

		m_obDynamicLODS.push_back( uiLOD );
		fTotalCost += m_obToSortSet[i]->GetLODCost( uiLOD );
	}

	ntAssert( m_obDynamicLODS.size() == m_obToSortSet.size() );

	bool bFinished = false;
	bool bImproveOrDegrade = (fTotalCost < m_fTotalLODBudget) ? true : false;

	while (!bFinished)
	{
		if (bImproveOrDegrade)
		{
			// we're improving our LOD's upwards untill we run out of budget or
			// theyre all top notch.
			// we start with the closest and work outwards

			bool bAllBest = true;

			for ( unsigned int i = 0; i < m_obToSortSet.size(); i++ )
			{
				ntError( i >= 0 && i < m_obToSortSet.size() );
				u_int uiLOD = m_obDynamicLODS[i];

				if (uiLOD > 0)	// can we be improved?
				{
					// yep, improve LOD and recalc cost
					bAllBest = false;
					
					fTotalCost -= m_obToSortSet[i]->GetLODCost( uiLOD );
					uiLOD--;

					fTotalCost += m_obToSortSet[i]->GetLODCost( uiLOD );
					m_obDynamicLODS[i] = uiLOD;
				}

				if (fTotalCost > m_fTotalLODBudget)
				{
					bFinished = true;
					break;
				}	
			}

			if (bAllBest)
				bFinished = true;
		}
		else
		{
			// we're degrading our LOD's downwards untill we enter our budget or
			// theyre all as cheap as poss.
			// we start with the furthest and work inwards
			bool bAllWorst = true;

			for ( int i = m_obToSortSet.size() - 1; i >= 0; i-- )
			{
				ntError( i >= 0 && i < (int)m_obToSortSet.size() );
				u_int uiLOD = m_obDynamicLODS[i];

				if (uiLOD < (m_obToSortSet[i]->GetNumLOD()-1))	// can we be degraded?
				{
					// yep, degrade LOD and recalc cost
					bAllWorst = false;
					
					fTotalCost -= m_obToSortSet[i]->GetLODCost( uiLOD );
					uiLOD++;

					fTotalCost += m_obToSortSet[i]->GetLODCost( uiLOD );
					m_obDynamicLODS[i] = uiLOD;
				}

				if (fTotalCost < m_fTotalLODBudget)
				{
					bFinished = true;
					break;
				}	
			}

			if (bAllWorst)
				bFinished = true;
		}
	}

	m_fTotalUsedBudget = fTotalCost;

	// now all we need to do is set from our final LOD list
	for ( unsigned int i = 0; i < m_obToSortSet.size(); i++ )
	{
		m_obToSortSet[i]->RequestLODSwitch( m_obDynamicLODS[i] );

		if (m_bDebugRender)
			m_obToSortSet[i]->DebugRender( pobCamera );
	}

}

/***************************************************************************************************
*
*	FUNCTION		CLODManager::Register
*
*	DESCRIPTION		register component with manager
*
***************************************************************************************************/
void	CLODManager::Register( CLODComponent* pobComp )
{
	ntAssert( pobComp );

#ifndef _RELEASE
	for (	LOCComponentList::iterator obIt = m_obLODComponents.begin();
			obIt != m_obLODComponents.end(); ++obIt )
	{
		if ((*obIt) == pobComp)
		{
			ntAssert_p(0,("Component not registerd with manager"));
		}
	}
#endif

	m_obLODComponents.push_back( pobComp );
}

/***************************************************************************************************
*
*	FUNCTION		CLODManager::UnRegister
*
*	DESCRIPTION		unregister component with manager
*
***************************************************************************************************/
void	CLODManager::UnRegister( CLODComponent* pobComp )
{
	ntAssert( pobComp );

	for (	LOCComponentList::iterator obIt = m_obLODComponents.begin();
			obIt != m_obLODComponents.end(); ++obIt )
	{
		if ((*obIt) == pobComp)
		{
			m_obLODComponents.erase( obIt );
			return;
		}
	}

	ntAssert_p(0,("Component not registerd with manager"));
}

