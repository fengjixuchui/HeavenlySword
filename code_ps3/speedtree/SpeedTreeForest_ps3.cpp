///////////////////////////////////////////////////////////////////////	 
//	CSpeedTreeForest Class
//
//	(c)	2003 IDV, Inc.
//
//	This class is provided to illustrate one way to	incorporate
//	SpeedTreeRT	into an	OpenGL application.	 All of	the	SpeedTreeRT
//	calls that must	be made	on a per tree basis	are	done by	this class.
//	Calls that apply to	all	trees (i.e.	static SpeedTreeRT functions)
//	are	made in	the	functions in main.cpp.
//
//
//	***	INTERACTIVE	DATA VISUALIZATION (IDV) PROPRIETARY INFORMATION ***
//
//	This software is supplied under	the	terms of a license agreement or
//	nondisclosure agreement	with Interactive Data Visualization	and	may
//	not	be copied or disclosed except in accordance	with the terms of
//	that agreement.
//
//		Copyright (c) 2001-2003	IDV, Inc.
//		All	Rights Reserved.
//
//		IDV, Inc.
//		1233 Washington	St.	Suite 610
//		Columbia, SC 29201
//		Voice: (803) 799-1699
//		Fax:   (803) 931-0320
//		Web:   http://www.idvinc.com
//


///////////////////////////////////////////////////////////////////////	 
//	Include	Files

#include "speedtree/SpeedTreeRandom_ps3.h"
#include "speedtree/SpeedTreeForest_ps3.h"
#include "speedtree/SpeedTreeConfig_ps3.h"
#include "speedtree/SpeedTreeWrapper_ps3.h"
#include "speedtree/SpeedTreeRenderable_ps3.h"
#include "speedtree/SpeedTreeManager_ps3.h"
#include "speedtree/SpeedTreeDef_ps3.h"
#include "speedtree/speedtreebillboard.h"

#include "core/explicittemplate.h"
#include "core/exportstruct_clump.h"
#include "anim/hierarchy.h"
#include "gfx/renderer.h"	 // billboard buffers
#include "gfx/texturemanager.h"
#include "gfx/shader.h"
#include "objectdatabase/dataobject.h"
#include "area/arearesourcedb.h"

#include "core/gatso.h"

#include "../content_ps3/cg/SpeedTree_Defines.h"

#include "objectdatabase/objectdatabase.h"

#define SPEEDTREE_USE_SCRATCH_BUFFERS


namespace SpeedTreeBillboard
{


const unsigned int c_cellDimension = 2;
class CCellSystem
{

	//class CNewCellsQueue
	//{
	//	typedef ntstd::deque<CCell*> Container;



	//	Container	m_queue;
	//};

	typedef ntstd::Vector<CCell*>  NewCellsQueue;
	typedef ntstd::Vector<int>	   AvailableBuffersList;

public:
	CCellSystem(const CAABB& forestAABB, unsigned int numTrees, CSpeedTreeWrapper** trees)
	{
		if (!trees) return;

		const unsigned int c_maxTreesPerCell = 100;

		float size[c_cellDimension] = { forestAABB.Max().X() - forestAABB.Min().X(), forestAABB.Max().Z() - forestAABB.Min().Z() };

		unsigned int numCells = ComputeCellDimensions(size, numTrees, c_maxTreesPerCell, m_numCells);

		m_cells = NT_NEW_ARRAY_CHUNK(SPEEDTREE_MEMORY_CHUNK) CCell[m_numCells[0] * m_numCells[1]];

		float cellSide[2] = { size[0] / (float)m_numCells[0], size[1] / (float)m_numCells[1] };
		for (unsigned int cellZ = 0; cellZ < m_numCells[1]; ++ cellZ)
		{
			for (unsigned int cellX = 0; cellX < m_numCells[0]; ++ cellX)
			{
				m_cells[CC(cellX, cellZ)].SetPosition(
					forestAABB.Min().X() + (float)cellX * cellSide[0],
					forestAABB.Min().Y(),
					forestAABB.Min().Z() + (float)cellZ * cellSide[1],
					cellSide[0], forestAABB.Max().Y() - forestAABB.Min().Y(), cellSide[1]);
			}
		}

		// put trees into the appropriate cells  (for now ignore the tree radius)
		for (unsigned int tree = 0; tree < numTrees; ++ tree)
		{
			const float* pos =  trees[tree] -> GetPosition() ;

			int x = (int)((pos[0] - forestAABB.Min().X()) / cellSide[0]);
			int z = (int)((pos[2] - forestAABB.Min().Z()) / cellSide[1]);

			ntAssert(x >= 0 && z >= 0);
			m_cells[CC(x, z)].AddTree(trees[tree]);
		}

		// work out maximum number of trees that can be found in a sigle cell
		unsigned int maxTreesInCell = 0;
		for (unsigned int cellZ = 0; cellZ < m_numCells[1]; ++ cellZ)
		{
			for (unsigned int cellX = 0; cellX < m_numCells[0]; ++ cellX)
			{
				unsigned int numTreesInCell = m_cells[CC(cellX, cellZ)].GetNumTrees();
				//ntAssert(numTreesInCell > 0);

				if (0 == numTreesInCell)
				{
					CRenderable* renderable = m_cells[CC(cellX, cellZ)].GetRenderable();
					if (renderable)
					{
						renderable -> DisableRendering(true);
					}
				}

				if ( numTreesInCell > maxTreesInCell )
				{
					maxTreesInCell = numTreesInCell;
				}

			}
		}

		// now use the max number of trees per cell for size of vertex buffers
	   m_VBPool = NT_NEW_CHUNK(SPEEDTREE_MEMORY_CHUNK)  CSpeedTreeBillboardBuffers(maxTreesInCell * c_numVertsPerBillboard);

	   // Give all cells the address of the VB system
	   for (unsigned int cell = 0; cell < numCells; ++ cell)
	   {
		   m_cells[cell].SetVertexBuffers(m_VBPool);
	   }																						 

	   m_newCells.reserve(numCells);
	   m_availableBuffers.reserve(CSpeedTreeBillboardBuffers::c_numBuffers);

	   // place all buffers into "available" list
	   for (unsigned int bufferIndex = 0; bufferIndex < CSpeedTreeBillboardBuffers::c_numBuffers; ++ bufferIndex)
	   {
		   m_availableBuffers.push_back(bufferIndex);
	   }

	}

	void UpdateVisibility()
	{
		CGatso::Start( "SPEEDTREE CellSystem::UpdateVisibility" );

		for (unsigned int cell = 0; cell < CellsTotal(); ++ cell)
		{
			CCell& currCell = m_cells[cell];
			CRenderable* renderable = currCell.GetRenderable();

			if (renderable -> IsRendering() && (renderable -> m_FrameFlags & CRenderable::RFF_VISIBLE))
			{
				if (currCell.IsDisabled())
				{
					if (!currCell.IsEnqueued())
					{
						// place the cell in the queue waiting for allocation of a vertex buffer
						m_newCells.push_back(&currCell);
					}
				}
			}
			else
			{
				if (!currCell.IsDisabled())
				{
					int bufferIndex = currCell.GetBufferIndex();
					m_VBPool -> SetAvailable(bufferIndex);
					m_availableBuffers.push_back(bufferIndex);
					currCell.SetDisabled();
				}
			}
		
		}

		// sort the new cell queue by distance from camera
		// TODO:...

		//for (NewCellsQueue::reverse_iterator cellIter = m_newCells.rbegin(); cellIter != m_newCells.rend(); ++ cellIter)
		unsigned int queueLen = m_newCells.size();
		for (unsigned int cell = 0; cell < queueLen; ++ cell)
		{
			if (m_availableBuffers.empty())
			{
				break;
			}

			int availableIndex = m_availableBuffers.back();
			CCell* cellToActivate = m_newCells.back();

			cellToActivate -> SetBufferIndex(availableIndex);
			m_VBPool -> SetToCell(availableIndex, cellToActivate);

			m_availableBuffers.pop_back();
			m_newCells.pop_back();

		}
		CGatso::Stop( "SPEEDTREE CellSystem::UpdateVisibility" );
	}

	void SetEnabled(bool isEnabled)
	{
		m_enabled = isEnabled;

		for (unsigned int cell = 0; cell < CellsTotal(); ++ cell)
		{
			CCell& currCell = m_cells[cell];
			CRenderable* renderable = currCell.GetRenderable();

			if (currCell.GetNumTrees() > 0)
			{
				if (isEnabled != renderable -> IsRendering())
				{
					renderable -> DisableRendering(!isEnabled);
				}
			}
		}

	}

	unsigned int CC(unsigned int x, unsigned int z) const
	{
		return x + z * m_numCells[0];
	}

	unsigned int CellsTotal() const
	{
		return m_numCells[0] * m_numCells[1];
	}

	~CCellSystem()
	{
		NT_DELETE_ARRAY_CHUNK(SPEEDTREE_MEMORY_CHUNK, m_cells);
		NT_DELETE_CHUNK( SPEEDTREE_MEMORY_CHUNK,m_VBPool);
	}

private:
	unsigned int					m_numCells[c_cellDimension]; // X, Z
	CCell*							m_cells;
	CSpeedTreeBillboardBuffers*		m_VBPool;

	// Auxialliary containers
	AvailableBuffersList			m_availableBuffers;
	NewCellsQueue					m_newCells;
	bool							m_enabled;
};

 }


///////////////////////////////////////////////////////////////////////	 
//	CSpeedTreeForest constructor

CSpeedTreeForest::CSpeedTreeForest( const SpeedTreeXmlForest * forestDef ) :
	 m_rockAngles(MAX_NUM_LEAF_ANGLES)
	,m_rustleAngles(MAX_NUM_LEAF_ANGLES)
	,m_fWindStrength(0.2f)
	,m_ulRenderBitVector(Speedtree_RenderAll)
	,m_pForestDef(forestDef)
	,m_bounds(CONSTRUCT_INFINITE_NEGATIVE)
	,m_cellSystem(NULL)
{
	//m_afForestExtents[0] = m_afForestExtents[1]	= m_afForestExtents[2] = FLT_MAX;
	//m_afForestExtents[3] = m_afForestExtents[4]	= m_afForestExtents[5] = -FLT_MAX;
}


///////////////////////////////////////////////////////////////////////	 
//	CSpeedTreeForest destructor
uint32_t CSpeedTreeForest::GetRenderBitVector()
{
	return m_ulRenderBitVector & SpeedTreeManager::Get().GetRenderBitVector();
}

CSpeedTreeForest::~CSpeedTreeForest( )
{
	for	(unsigned int i	= 0; i < m_treesInForest.size( ); ++i)
	{
		NT_DELETE_CHUNK( SPEEDTREE_MEMORY_CHUNK, m_treesInForest[i] );
	}
	NT_DELETE_CHUNK( SPEEDTREE_MEMORY_CHUNK, m_cellSystem);

	// to prevent speedtree's internal memory leak
	CSpeedTreeRT::ResetError();
}

///////////////////////////////////////////////////////////////////////	 
//	CSpeedTreeForest::Load
//
//	A (null) pFilename value forces	the	load routine to	generate the
//	trees procedurally instead of loading from an STF file

bool CSpeedTreeForest::Load()
{
	ntError_p(m_pForestDef, ("SPEEDTREE : No .spt file supplied for a speedtree forest\n"));

	static char acFileName[ MAX_PATH * 2 ];
	bool bSuccess =	false;
	Util::SetToPlatformResources();

	// setup a few parameters shared by	all	tree models
	CSpeedTreeRT::SetNumWindMatrices( SpeedTreeManager::Get().GetGlobalData().m_nNumWindMatrices);
	
	LoadFromStfFile();

	// setup speedwind
	bool exists = SpeedTreeManager::Get().GetResourcePath(ntStr::GetString(m_pForestDef->m_speedWindFilename), acFileName);
	user_warn_p(exists, ("SPEEDTREE : Failed to load speedwind definition in %s\n", acFileName));
	UNUSED(exists);

	m_cSpeedWind.Load(acFileName);
	m_cSpeedWind.SetQuantities(NUM_WIND_MATRICES, MAX_NUM_LEAF_ANGLES);
	m_cSpeedWind.SetWindStrengthAndDirection(1.f, 1.f, 0, 0);
	//SpeedTreeXmlWind sWind = m_cSpeedWind.GetAttributes();
	//sWind.m_uiNumMatrices =	SpeedTreeManager::Get().GetGlobalData().m_nNumWindMatrices;
	//m_cSpeedWind.SetAttributes(sWind);
	SetWindStrength(1.f);


	Util::SetToNeutralResources();
	

	return bSuccess;
}



void CSpeedTreeForest::LoadWindMatrix(Shader* pMaterial)
{
	// CAHCE DATA (fixme_frank) to be optimised, not per frame
	uint32_t iNumElem = GetWind()->GetNumWindMatrices();
	CScopedArray<float> pTmp( NT_NEW_CHUNK(SPEEDTREE_MEMORY_CHUNK)  float[16*iNumElem]);
	for(uint32_t uiLocation = 0 ; uiLocation < iNumElem ; ++uiLocation )
	{
		NT_MEMCPY(&pTmp[16*uiLocation],GetWind()->GetWindMatrix(uiLocation),16*sizeof(float));
	}
	
	// load it
	uint32_t uiIndex = pMaterial->GetVertexHandle()->GetConstantIndex( FwHashedString("g_amWindMatrices") );
	ntAssert_p(uiIndex<pMaterial->GetVertexHandle()->GetConstantCount(), ("g_amWindMatrices does not exsit"));
	pMaterial->SetVSConstant(uiIndex,pTmp.Get(),4*iNumElem);
}

void CSpeedTreeForest::LoadLeafMatrix(Shader* pMaterial)
{
	// CAHCE DATA (fixme_frank) to be optimised, not per frame
	//uint32_t iNumElem = GetWind()->GetNumLeafAngles();
	//CScopedArray<float> pTmp( NT_NEW_CHUNK(SPEEDTREE_MEMORY_CHUNK)  float[16*iNumElem]);
	//for(uint32_t uiLocation = 0 ; uiLocation < iNumElem ; ++uiLocation )
	//{
	//	NT_MEMCPY(&pTmp[16*uiLocation],GetWind()->GetLeafAngleMatrix(uiLocation),16*sizeof(float));
	//}

	//// load it
	//uint32_t uiIndex = pMaterial->GetVertexHandle()->GetConstantIndex( FwHashedString("g_amLeafAngleMatrices") );
	//ntAssert_p(uiIndex<pMaterial->GetVertexHandle()->GetConstantCount(), ("g_amLeafAngleMatrices does not exsit"));
	//pMaterial->SetVSConstant(uiIndex,pTmp.Get(),4*iNumElem);
}

				   

///////////////////////////////////////////////////////////////////////	 
//	CSpeedTreeForest::AdjustExtents

//void CSpeedTreeForest::AdjustExtents(float x, float	y, float z)
//{
//	if (m_treesInForest.size() == 1)
//	{
//		NT_MEMCPY(m_afForestExtents, m_treesInForest[0]->GetBoundingBox( ), 6 * sizeof(float));
//	}
//	else
//	{
//		// min
//		m_afForestExtents[0] = __min(m_afForestExtents[0], x); 
//		m_afForestExtents[1] = __min(m_afForestExtents[1], y); 
//		m_afForestExtents[2] = __min(m_afForestExtents[2], z); 
//
//		// max
//		m_afForestExtents[3] = __max(m_afForestExtents[3], x); 
//		m_afForestExtents[4] = __max(m_afForestExtents[4], y); 
//		m_afForestExtents[5] = __max(m_afForestExtents[5], z);
//	}
//}

///////////////////////////////////////////////////////////////////////	 
//	CSpeedTreeForest::LoadFromStfFile

inline void SetTreePosition(CSpeedTreeWrapper* tree, const CPoint& pos)
{
	//const float* bounds = tree->GetBoundingBox();
	float heightOffset = 0;//(bounds[4] - bounds[1]) * 0.7f;
	//tree->GetSpeedTree()->SetTreePosition(pos[0] + pivot[0], pos[1] + pivot[1], pos[2] + pivot[2]);
	tree->GetSpeedTree()->SetTreePosition(pos[0], pos[1] - heightOffset, pos[2]);

}

void CSpeedTreeForest::RegisterWithAreaSystem()
{
	ntAssert(m_pForestDef);
	const char* name = ntStr::GetString(ObjectDatabase::Get().GetNameFromPointer(m_pForestDef));
	AreaResourceDB::Get().AddAreaResource( name, AreaResource::SPEEDTREE, m_pForestDef -> m_sectorBits );

	for (InstanceContainer::iterator iter = m_treesInForest.begin(); iter != m_treesInForest.end(); ++ iter)
	{
		(*iter) -> RegisterTextures(m_pForestDef -> m_sectorBits);
	}
}

void CSpeedTreeForest::Enable(bool enable)
{
	m_cellSystem -> SetEnabled(enable);

	for (InstanceContainer::iterator iter = m_treesInForest.begin(); iter != m_treesInForest.end(); ++ iter)
	{
		(*iter) -> Enable(enable);
	}
}

void CSpeedTreeForest::OnAreaLoadStart()
{

}

void CSpeedTreeForest::OnAreaLoadEnd()
{
	//int tree = 0;
	//for(SpeedTreeXmlForest::List::const_iterator treeIter = m_pForestDef->m_list.begin();
	//	treeIter != m_pForestDef->m_list.end();
	//	++treeIter, ++tree)
	for (InstanceContainer::iterator iter = m_treesInForest.begin(); iter != m_treesInForest.end(); ++ iter)
	{
		//const SpeedTreeXmlTree* pTreeDef = *treeIter;

		CSpeedTreeWrapper* pTreeClone = *iter;

		pTreeClone -> OnAreaLoadEnd(NULL);
	}

}

inline float SetTreeScale(CSpeedTreeWrapper* tree, const SpeedTreeXmlTree* treeDef)
{
	float size = tree -> GetSize();
	const float minSize = 0.1f;
	float varianceTerm = GetRandom( -treeDef -> m_fVariance, treeDef -> m_fVariance);
	float instanceSize = size + varianceTerm;
	instanceSize = instanceSize > minSize ? instanceSize : minSize;
	float scale = instanceSize / size;
	//return tree -> SetScale(scale, treeDef -> m_fNearLodDistance, treeDef -> m_fFarLodDistance);
	return tree -> SetScale(scale, tree -> m_minLODDistance, tree -> m_maxLODDistance);
}

bool CSpeedTreeForest::LoadFromStfFile()
{
	static char acFileName[ MAX_PATH * 2 ];

	// get speedtree path, todo: per-level
	ntstd::String speedTreePath = SpeedTreeManager::Get().GetDataPath();

	// go through the base trees
	for(SpeedTreeXmlForest::List::const_iterator it = m_pForestDef->m_list.begin();
		it != m_pForestDef->m_list.end();
		++it)
	{
		//clone def
		const SpeedTreeXmlTree* pTreeDef = *it;

		if (!pTreeDef)
		{
			user_warn_msg(("SPEEDTREE : A tree definition referenced in by the forest is missing in the level file\n"));
			continue;
		}

		const char* templateFileName = pTreeDef->m_templateName.c_str();
		if (ntStr::IsNull(templateFileName))
		{
			user_warn_msg(("SPEEDTREE : Template file name (.spt) is missing from the tree definition. Tree ignored\n"));
			continue;
		}

		bool exists = SpeedTreeManager::Get().GetResourcePath(templateFileName, acFileName);
		if (!exists)
		{
			user_warn_msg(("SPEEDTREE : Can not find speedtree template file %s. Tree ignored\n", acFileName));
			continue;
		}
		
		// create clone
		// SpeedTreeRt has its own new operator			
		CSpeedTreeWrapper*	registeredTree = SpeedTreeManager::Get().GetUniqueTree(pTreeDef);

		CSpeedTreeWrapper* pTreeClone = NULL;
		// Check if we have not encountered this type of tree before
		if (!registeredTree)
		{
			pTreeClone = NT_NEW_CHUNK(SPEEDTREE_MEMORY_CHUNK)  CSpeedTreeWrapper( new CSpeedTreeRT(), this);
			bool bRes=pTreeClone->LoadTree(acFileName, pTreeDef);
			pTreeClone -> OnAreaLoadStart(pTreeDef);

			UNUSED(bRes);
			ntError_p(bRes, ("SPEEDTREE : Cannot load tree %s while loading %s\n", acFileName, ntStr::GetString(ObjectDatabase::Get().GetNameFromPointer(m_pForestDef))));
			SpeedTreeManager::Get().AddUniqueTree(pTreeClone, pTreeDef);
		}
		else
		{
			pTreeClone = registeredTree -> MakeInstance();
		}

		CPoint pos(pTreeDef->m_position); 
		float offset = SetTreeScale(pTreeClone, pTreeDef);
		pos.Y() -= offset;

		pTreeClone -> SetWindOffset(GetRandom(0.f, 100.f));

		m_treesInForest.push_back(pTreeClone);

		SetTreePosition(pTreeClone, pos);
		pTreeClone -> SetRotation(pTreeDef -> m_rotation);

		if (pTreeDef -> m_needsCollision)
		{
			pTreeClone->CreatePhysicsRep();
		}

		// create instances
		for(SpeedTreeXmlTree::List::const_iterator it = pTreeDef->m_list.begin();
			it != pTreeDef->m_list.end();
			++it)
		{
			const SpeedTreeXmlTreeInstance* pTreeDefInstance = *it;
			//if (!pTreeDefInstance)
			//{
			//	CKeyString str = ObjectDatabase::Get().GetNameFromPointer(pTreeDef);
			//	ntPrintf("!!!%s\n", ntStr::GetString(str));
			//}
			ntError_p(pTreeDefInstance, ("No instance definition found"));
			CSpeedTreeWrapper* pInstance = pTreeClone->MakeInstance();

			CPoint pos(pTreeDefInstance->m_position); 

			float offset = SetTreeScale(pInstance, pTreeDef);
			pos.Y() -= offset;
			SetTreePosition(pInstance, pos);

			pInstance -> SetRotation(pTreeDefInstance -> m_rotation);

			if (pTreeDefInstance -> m_needsCollision)
			{
				pInstance->CreatePhysicsRep();
			}
			m_treesInForest.push_back(pInstance);

			pInstance -> SetWindOffset(GetRandom(0.f, 100.f));
		}

	}

	// create tree
	for(uint32_t	iTree =	0 ;	iTree <	m_treesInForest.size() ; ++iTree )
	{
		Transform* transform = m_treesInForest[iTree]->GetTransform(); 
		CHierarchy::GetWorld()->GetRootTransform()->AddChild(transform);
		m_treesInForest[iTree]->SetRenderable(m_pForestDef -> m_sectorBits == 0);

		const float* bounds = m_treesInForest[iTree]->GetBoundingBox();
		m_bounds.Union(CAABB(CPoint(bounds) + CPoint(m_treesInForest[iTree]->GetSpeedTree() -> GetTreePosition()), CPoint(bounds + 3) + CPoint(m_treesInForest[iTree]->GetSpeedTree() -> GetTreePosition())));
	}

	if (m_treesInForest.size() > 0)
	{
		m_cellSystem = NT_NEW_CHUNK(SPEEDTREE_MEMORY_CHUNK)  SpeedTreeBillboard::CCellSystem(m_bounds, m_treesInForest.size(), &m_treesInForest[0]);	
	}

	
	user_warn_p(m_treesInForest.size()>0,("SPEEDTREE : No trees in forest!\n"));

	return true;
}



///////////////////////////////////////////////////////////////////////	 
//	CSpeedTreeForest::SetWindStrength

void CSpeedTreeForest::SetWindStrength(float fStrength)
{
	m_fWindStrength	= fStrength;

	static float afZeroes[6] = { 0.0f };
	for	(unsigned int i	= 0; i < m_treesInForest.size(	); ++i)
	{
		m_treesInForest[i]->GetSpeedTree( )->SetWindStrengthAndLeafAngles(m_fWindStrength, afZeroes, afZeroes, 6);
	}
}


///////////////////////////////////////////////////////////////////////	 
//	CSpeedTreeForest::SetupWindMatrices

void CSpeedTreeForest::AdvanceWind(float fTimeInSecs)
{
	Vec3 windDirection = SpeedTreeManager::Get().GetGlobalData().m_afWindDirection;
	//m_cSpeedWind.Advance(fTimeInSecs, m_fWindStrength, windDirection[0], windDirection[1], windDirection[2]);
	m_cSpeedWind.Advance(fTimeInSecs, true, windDirection[0], windDirection[1], windDirection[2]);
	m_cSpeedWind.GetRockAngles(m_rockAngles);
	m_cSpeedWind.GetRustleAngles(m_rustleAngles);

	ntAssert(m_rockAngles.size() == MAX_NUM_LEAF_ANGLES);
	ntAssert(m_rustleAngles.size() == MAX_NUM_LEAF_ANGLES);
}


///////////////////////////////////////////////////////////////////////	 
//	CSpeedTreeForest::SetLodLimits

void CSpeedTreeForest::SetLodLimits(void)
{
	// find	tallest	tree
	float fTallest = -1.0f;
	for	(uint32_t i	= 0; i < m_treesInForest.size( ); ++i)
	{
		float fHeight =	m_treesInForest[i]->GetBoundingBox(	)[4] - m_treesInForest[i]->GetBoundingBox( )[1];
		fTallest = max(fHeight, fTallest);
	}

	// assign all trees	based on tallest
	for	(uint32_t i = 0;	i <	m_treesInForest.size( ); ++i)
	{
		m_treesInForest[i]->GetSpeedTree( )->SetLodLimits(
			fTallest * SpeedTreeManager::Get().GetGlobalData().m_fNearLodFactor,
			fTallest * SpeedTreeManager::Get().GetGlobalData().m_fFarLodFactor);
	}
}

void CSpeedTreeForest::PerFrameUpdate(float fElapsedTime)
{
	UNUSED(fElapsedTime);
	// wind
	AdvanceWind(SpeedTreeManager::Get().GetAccumTime());
}

void CSpeedTreeForest::PerViewportUpdate()
{
	if (m_cellSystem)
	{
		m_cellSystem -> UpdateVisibility();
	}
	// update lod
 	for	(unsigned int i	= 0; i < m_treesInForest.size(	); ++i)
	{
		m_treesInForest[i]->UpdateLod( );
	}
}

void CSpeedTreeForest::UpdateForLight()
{
	//CDirection dir = SpeedTreeManager::Get().GetObserver(). m_lightDir;
	//float cdir[3] = { dir.X(), dir.Y(), dir.Z() };
	//this->BuildLeafAngleMatrices(cdir);
}


void CSpeedTreeForest::UpdateForCamera()
{
	//CDirection dir = SpeedTreeManager::Get().GetObserver().m_camDir;
	//float cdir[3] = { dir.X(), dir.Y(), dir.Z() };
	//this->BuildLeafAngleMatrices(cdir);
}




SpeedTreeStat CSpeedTreeForest::GetStat()
{
	SpeedTreeStat res;
	for(ntstd::Vector<CSpeedTreeWrapper*>::iterator it	= m_treesInForest.begin();
		it != m_treesInForest.end();
		++it)
	{
		res+=(*it)->GetStat();
	}

	return res;
}

#ifdef SPEEDTREE_COLLECT_STATS
CSpeedTreeWrapper*	CSpeedTreeForest::DebugGetTree(const SpeedTreeXmlTree* treeDef)
{
	for (InstanceContainer::iterator iter = m_treesInForest.begin(); iter != m_treesInForest.end(); ++ iter)
	{
		if ((*iter) -> GetXMLDef() == treeDef)
		{
			return *iter;
		}
	}

	return NULL;
}
#endif
