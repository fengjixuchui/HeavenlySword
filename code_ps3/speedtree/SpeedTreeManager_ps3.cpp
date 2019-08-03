#include <speedtreeallocator.h>
#include "speedtree/SpeedTreeManager_ps3.h"
#include "speedtree/SpeedTreeForest_ps3.h"
#include "speedtree/SpeedTreeWrapper_ps3.h"
#include "speedtree/SpeedTreeShaders_ps3.h"
#include "speedtree/speedtreerenderable_ps3.h"
#include "speedtreedef_ps3.h"
#include "speedtreeconfig_ps3.h"

#include "core/visualdebugger.h"
#include "core/osddisplay.h"
#include "core/gatso.h"
#include "core/io.h"
#include "input/inputhardware.h"		// for debug control only
#include "objectdatabase/dataobject.h"
#include "game/keybinder.h"
#include "gfx/rendercontext.h"
#include "camera/camman_public.h"
#include "anim/hierarchy.h"

#include "gfx/shader.h"

#include "uniqueptrcontainer.h"

extern void CreateSpeedTreeShaders();
extern void DestroySpeedTreeShaders();
extern void CreateSpeedGrassShaders();
extern void DestroySpeedGrassShaders();


class CCustomSpeedTreeAllocator : public CSpeedTreeAllocator
{
public:
        void* Alloc(size_t blockSize, size_t)
        {
			void* block = (void*)NT_ALLOC_CHUNK(SPEEDTREE_MEMORY_CHUNK, blockSize);
			ntAssert(block);

            return block;
        }

        void Free(void* block)
        {
            if (block)
                NT_FREE_CHUNK(SPEEDTREE_MEMORY_CHUNK, (uintptr_t)block);
        }
};

class CUniqueTreeElement
{
public:
	explicit CUniqueTreeElement(const SpeedTreeXmlTree* xmlDef)
	{
		ntAssert(xmlDef);
		size_t	templateNameLen = ntStr::GetLength(xmlDef -> m_templateName);
		size_t	hashStringLen = templateNameLen;// + sizeof(xmlDef -> m_fSize);// + sizeof(xmlDef -> m_seed);
		char* hashString = (char*)alloca(hashStringLen);
		NT_MEMCPY(hashString, ntStr::GetString(xmlDef -> m_templateName), templateNameLen);
		//ntAssert(sizeof(xmlDef -> m_fSize) == sizeof(unsigned int));
		//*((unsigned int*)(hashString + templateNameLen)) = *((unsigned int*)&xmlDef -> m_fSize);
		//*((unsigned int*)&hashString[templateNameLen + sizeof(xmlDef -> m_fSize)]) = xmlDef -> m_seed;

		hash_ = FwHashedString(hashString, hashStringLen);

	}

	const SpeedTreeXmlTree*	def_;
	FwHashedString			hash_;

};

struct CUniqueTreePredicate : public ntstd::binary_function<const CUniqueTreeElement&, const CUniqueTreeElement&, bool>
{

	bool operator() (const CUniqueTreeElement& lhs, const CUniqueTreeElement& rhs)   const
	{
		return lhs.hash_ < rhs.hash_;
	}
};

class CTreeRegister
{
public:
	typedef ntstd::Map<CUniqueTreeElement, CSpeedTreeWrapper*, CUniqueTreePredicate>	UniqueTreeContainer;

	UniqueTreeContainer	cont_;
};

namespace
{
	CCustomSpeedTreeAllocator	g_SpeedTreeAllocator;
}


void SpeedTreeManager::LoadXmlFile(const ntstd::String& xmlFilename)
{
	char acFileName[256];
	Util::GetFiosFilePath( xmlFilename.c_str(), acFileName );
	FileBuffer obFile( acFileName, true );
	if ( !ObjectDatabase::Get().LoadDataObject( &obFile, acFileName ) )
	{
		ntError_p( false, ( "Failed to parse XML file '%s'", acFileName ) );
	}
}


//! constructor
SpeedTreeManager::SpeedTreeManager()
	:m_fAccumTime(0.0f)
	,m_ulRenderBitVector(Speedtree_RenderAll)
	,m_speedTreeFolder("data/speedtree/")
	,m_speedtreeToggle(SPEEDTREE_M_TOGGLE_ALL_ON)
	,m_treeRegister(NT_NEW_CHUNK(SPEEDTREE_MEMORY_CHUNK) CTreeRegister)
{
	CreateSpeedTreeShaders();
	CreateSpeedGrassShaders();

	//CreateStream();
	RegisterKey();

	CSpeedTreeRT::SetAllocator(&g_SpeedTreeAllocator);
}

void SpeedTreeManager::RegisterKey()
{
	// Register some commands
	CommandBaseInput<const int&>* pRenderToggle = CommandManager::Get().CreateCommand("Speedtree", this, &SpeedTreeManager::CommandToggleRender, "Toggle rendering of speedtree renderable number n");
	KeyBindManager::Get().RegisterKey("rendering", pRenderToggle, "Toggle Speedtree Leaf rendering", int(SPEEDTREE_LEAF), KEYS_PRESSED, KEYC_A, KEYM_ALT);
	KeyBindManager::Get().RegisterKey("rendering", pRenderToggle, "Toggle Speedtree Frond rendering", int(SPEEDTREE_FROND), KEYS_PRESSED, KEYC_S, KEYM_ALT);
	KeyBindManager::Get().RegisterKey("rendering", pRenderToggle, "Toggle Speedtree Branch rendering", int(SPEEDTREE_BRANCH), KEYS_PRESSED, KEYC_D, KEYM_ALT);
	KeyBindManager::Get().RegisterKey("rendering", pRenderToggle, "Toggle Speedtree Billboard rendering", int(SPEEDTREE_BILLBOARD), KEYS_PRESSED, KEYC_F, KEYM_ALT);

	// Register some commands
	CommandBaseInput<const int&>* pMainToggle = CommandManager::Get().CreateCommand("Speedtree", this, &SpeedTreeManager::CommandToggleSpeedtree, "Toggle speedtree updates n");
	KeyBindManager::Get().RegisterKey("rendering", pMainToggle, "Toggle Speedtree Leaf rendering", int(SPEEDTREE_TOGGLE_RENDER), KEYS_PRESSED, KEYC_Z, KEYM_ALT);
	KeyBindManager::Get().RegisterKey("rendering", pMainToggle, "Toggle Speedtree Game update", int(SPEEDTREE_TOGGLE_GAME_UPDATE), KEYS_PRESSED, KEYC_X, KEYM_ALT);
	KeyBindManager::Get().RegisterKey("rendering", pMainToggle, "Toggle Speedtree Viewport Update", int(SPEEDTREE_TOGGLE_VIEWPORT_UPDATE), KEYS_PRESSED, KEYC_C, KEYM_ALT);
	KeyBindManager::Get().RegisterKey("rendering", pMainToggle, "Toggle Speedtree Debug rendering", int(SPEEDTREE_TOGGLE_DEBUG_RENDER), KEYS_PRESSED, KEYC_V, KEYM_ALT);

#ifdef SPEEDTREE_COLLECT_STATS
	// Dump stats
	CommandBaseNoInput* dumpStatsCommand = CommandManager::Get().CreateCommandNoInput("DumpSpeedTreeStats", this, &SpeedTreeManager::DumpStats, "Dump SpeedTree stats");
	KeyBindManager::Get().RegisterKeyNoInput("game", dumpStatsCommand, "Dump SpeedTree stats", KEYS_PRESSED, KEYC_T, KEYM_SHIFT | KEYM_CTRL | KEYM_ALT );
#endif

}

// add remove forest
void SpeedTreeManager::AddForest(const SpeedTreeXmlForest * pDef)
{
	ntAssert(pDef);
	ntAssert_p(m_forests.find(pDef)==m_forests.end(), ("pDef already registerd"));
	//const Mem::MemStats& stats =  Mem::GetMemStats();
	//int chunk = Mem::MC_PROCEDURAL;
	//ntPrintf("speedtre mem usage : %d overflow : %d\n", stats.sChunks[chunk].iNumBytesInUse, stats.sChunks[chunk].iNumBytesOverflowInUse );
	CSpeedTreeForest*pForest = NT_NEW_CHUNK(SPEEDTREE_MEMORY_CHUNK)  CSpeedTreeForest(pDef);
	//ntPrintf("speedtre mem usage : %d overflow : %d\n", stats.sChunks[chunk].iNumBytesInUse, stats.sChunks[chunk].iNumBytesOverflowInUse );
	pForest->Load();
	//ntPrintf("speedtre mem usage : %d overflow : %d\n", stats.sChunks[chunk].iNumBytesInUse, stats.sChunks[chunk].iNumBytesOverflowInUse );
	//if (0 == pDef -> m_sectorBits)
	pForest->OnAreaLoadStart();
		//ntPrintf("speedtre mem usage : %d overflow : %d\n", stats.sChunks[chunk].iNumBytesInUse, stats.sChunks[chunk].iNumBytesOverflowInUse );
		//ntPrintf("speedtre mem usage : %d overflow : %d\n", stats.sChunks[chunk].iNumBytesInUse, stats.sChunks[chunk].iNumBytesOverflowInUse );

	if (0 != pDef -> m_sectorBits)
	{
		pForest -> RegisterWithAreaSystem();
		pForest -> Enable(false);
	}
	else
	{
		pForest->OnAreaLoadEnd();
	}

	m_forests.insert( Forests::value_type(pDef,pForest) );
}
void SpeedTreeManager::RemoveForest(const SpeedTreeXmlForest * pDef)
{
	Forests::iterator	forestIter = m_forests.find(pDef);
	if (forestIter != m_forests.end())
	{
		NT_DELETE_CHUNK( SPEEDTREE_MEMORY_CHUNK, forestIter->second );
		m_forests.erase(forestIter);
	}
	else
	{
		ntAssert(0); // forest not found
	}
}

//! destructor
SpeedTreeManager::~SpeedTreeManager()
{
	for(Forests::iterator it = m_forests.begin();
		it != m_forests.end();
		++it)
	{
		NT_DELETE_CHUNK( SPEEDTREE_MEMORY_CHUNK, it->second );
	}

	NT_DELETE_CHUNK( SPEEDTREE_MEMORY_CHUNK, m_treeRegister );

	DestroySpeedGrassShaders();
	DestroySpeedTreeShaders();
}

// toggle speedtree renderable
COMMAND_RESULT SpeedTreeManager::CommandToggleSpeedtree(const int& kind)
{
	m_speedtreeToggle.Toggle(Speedtree_toggle(kind));
	OSD::Add(OSD::DEBUG_CHAN, 0xffffffff, "Render[%s] Animate[%s] Viewport[%s] Debug[%s]" ,
		(m_speedtreeToggle[SPEEDTREE_TOGGLE_RENDER]?"x":" "),
		(m_speedtreeToggle[SPEEDTREE_TOGGLE_GAME_UPDATE]?"x":" "),
		(m_speedtreeToggle[SPEEDTREE_TOGGLE_VIEWPORT_UPDATE]?"x":" "),
		(m_speedtreeToggle[SPEEDTREE_TOGGLE_DEBUG_RENDER]?"x":" ") );
	return CR_SUCCESS;
}

// toggle speedtree renderable
COMMAND_RESULT SpeedTreeManager::CommandToggleRender(const int& kind)
{
	m_ulRenderBitVector ^= (1<<ntstd::Clamp(kind,0,4));
	OSD::Add(OSD::DEBUG_CHAN, 0xffffffff, "Leaf[%s] Frond[%s] Branch[%s] Billboard[%s] " ,
		((GetRenderBitVector()&Speedtree_RenderLeaves)?"x":" "),
		((GetRenderBitVector()&Speedtree_RenderFronds)?"x":" "),
		((GetRenderBitVector()&Speedtree_RenderBranches)?"x":" "),
		((GetRenderBitVector()&Speedtree_RenderBillboards)?"x":" ") );
	return CR_SUCCESS;
}

// get state
SpeedTreeStat SpeedTreeManager::GetStat()
{
	SpeedTreeStat res;
	for(Forests::iterator it = m_forests.begin();
		it != m_forests.end();
		++it)
	{
		res+=it->second->GetStat();
	}
	return res;
}


void SpeedTreeManager::DebugRender()
{
		//SpeedTreeStat stat = GetStat();
		//Vec2 position(10,10);
		//g_VisualDebug->Printf2D(position[0],position[1],DC_PURPLE,0,"ALL = %i", stat.GetTotal());
		//position[1]+=10;
		//g_VisualDebug->Printf2D(position[0],position[1],DC_PURPLE,0,"Branch count = %i", stat.m_BranchTriangleCount);
		//position[1]+=10;
		//g_VisualDebug->Printf2D(position[0],position[1],DC_PURPLE,0,"Frond count = %i", stat.m_FrondTriangleCount);
		//position[1]+=10;
		//g_VisualDebug->Printf2D(position[0],position[1],DC_PURPLE,0,"Leaf count = %i", stat.m_leafTriangleCount);
		//position[1]+=10;
		//g_VisualDebug->Printf2D(position[0],position[1],DC_PURPLE,0,"Billboard count = %i", stat.m_BillboardTriangleCount);
	}

void SpeedTreeManager::SetSpeedtreeCamera(const CPoint& pos, const CDirection& dir)
{
	float cpos[3] = { pos.X(), pos.Y(), pos.Z() };
	float cdir[3] = { dir.X(), dir.Y(), dir.Z() };
	CSpeedTreeRT::SetCamera(cpos,cdir);
}

void SpeedTreeManager::SetSpeedtreeCameraToCamera(bool bForBillboard)
{
	//if(bForBillboard)
	//{
	SetSpeedtreeCamera(m_speedtreeObserver.m_camPos, m_speedtreeObserver.m_camDir);
	//}
	//else
	//{
	//	SetSpeedtreeCamera(m_speedtreeObserver.m_camPos,CDirection(1.0f, 0.0f, 0.0f));
	//}
}
void SpeedTreeManager::SetSpeedtreeCameraToLight(bool bForBillboard)
{
	//if(bForBillboard)
	if (1)
	{
		SetSpeedtreeCamera(m_speedtreeObserver.m_lightPos, m_speedtreeObserver.m_lightDir);
	}
	else
	{
		SetSpeedtreeCamera(m_speedtreeObserver.m_lightPos,CDirection(1.0f, 0.0f, 0.0f));
	}
}

void SpeedTreeManager::UpdateForLight()
{
	CGatso::Start( "SpeedTreeManager::UpdateLight" );
	SetSpeedtreeCameraToLight();
	for(Forests::iterator it = m_forests.begin();
		it != m_forests.end();
		++it)
	{
		it->second->UpdateForLight();;
	}
	CGatso::Stop( "SpeedTreeManager::UpdateLight" );
}
void SpeedTreeManager::UpdateForCamera()
{
	CGatso::Start( "SpeedTreeManager::UpdateCamera" );
	SetSpeedtreeCameraToCamera();
	for(Forests::iterator it = m_forests.begin();
		it != m_forests.end();
		++it)
	{
		it->second->UpdateForCamera();;
	}
	CGatso::Stop( "SpeedTreeManager::UpdateCamera" );
}

void SpeedTreeManager::PerFrameUpdate(float fElapsedTime)
{
	if(m_speedtreeToggle[SPEEDTREE_TOGGLE_DEBUG_RENDER])
	{
		DebugRender();
	}
	if(!m_speedtreeToggle[SPEEDTREE_TOGGLE_GAME_UPDATE])
	{
		return;
	}

//	CGatso::Start( "SpeedTreeManager::PerFrameUpdate" );

		static float fLastFrameTime = 0.0f;

		// compute time
		m_fAccumTime += fElapsedTime;

		// advance wind
		CSpeedTreeRT::SetTime(m_fAccumTime);
		fLastFrameTime = m_fAccumTime;

	// update each tree
	for(Forests::iterator it = m_forests.begin();
		it != m_forests.end();
		++it)
	{
		it->second->PerFrameUpdate(fElapsedTime);
	}

//	CGatso::Stop( "SpeedTreeManager::PerFrameUpdate" );
}

void SpeedTreeManager::PerViewportUpdate()
{
	CGatso::Start( "SpeedTreeManager::PerViewportUpdate" );

	if(!m_speedtreeToggle[SPEEDTREE_TOGGLE_VIEWPORT_UPDATE])
	{
		return;
	}

	// get light
	//CDirection keyDirection = -RenderingContext::Get()->m_keyDirection;
	CDirection keyDirection = -RenderingContext::Get()->m_shadowDirection;
	for(int i = 0 ; i < 3 ; ++i ) m_speedtreeObserver.m_lightDir[i] = keyDirection[i];
	for(int i = 0 ; i < 3 ; ++i ) m_speedtreeObserver.m_lightPos[i] = -1000*keyDirection[i];

	// get camera FIXME_frank, get it from the right place
	//const CCamera* pCamera = CamMan_Public::GetP();
	//CMatrix invWorldToEye = pCamera->GetViewTransform()->GetWorldMatrix();
	CMatrix invWorldToEye = RenderingContext::Get()->m_worldToView.GetAffineInverse();
	CDirection worldCamDirection = CDirection(invWorldToEye[2]);
	CPoint worldCamPosition = invWorldToEye.GetTranslation();
	for(int i = 0 ; i < 3 ; ++i ) m_speedtreeObserver.m_camDir[i] = worldCamDirection[i];
	for(int i = 0 ; i < 3 ; ++i ) m_speedtreeObserver.m_camPos[i] = worldCamPosition[i];

	// update camera and lod
	SetSpeedtreeCameraToCamera();
	for(Forests::iterator it = m_forests.begin();
		it != m_forests.end();
		++it)
	{
		it->second->PerViewportUpdate();
	}
	CGatso::Start( "SpeedTreeManager::PerViewportUpdate" );
}

// get render bit
uint32_t SpeedTreeManager::GetRenderBitVector()
{
	if(m_speedtreeToggle[SPEEDTREE_TOGGLE_RENDER])
	{
		return m_ulRenderBitVector;
	}
	else
	{
		return m_ulRenderBitVector & (~Speedtree_RenderAll);
	}
}

bool SpeedTreeManager::GetResourcePath(const char* originalName, char* speedTreeName)
{
 	Util::SetToPlatformResources();
	const char* baseName = Util::BaseName(originalName);

	ntstd::String speedTreePath = GetDataPath();
	speedTreePath += baseName;
	Util::GetFullGameDataFilePath( ntStr::GetString(speedTreePath), speedTreeName );
	Util::SetToNeutralResources();

	return CellFsFile::Exists( speedTreeName );
}

void SpeedTreeManager::ResetRendering()
{
	RenderableSpeedTree::ResetBatch();
}

void SpeedTreeManager::FinishRendering()
{
	RenderableSpeedTree::EndBatch();
}

void SpeedTreeManager::OnLevelUnload()
{
	CSpeedTreeBillboardBuffers::Destroy();

	ntAssert(m_treeRegister);

	m_treeRegister -> cont_.clear();
}

void SpeedTreeManager::OnAreaLoad(CHashedString resHash)
{
	const SpeedTreeXmlForest* def = ObjectDatabase::Get().GetPointerFromName<SpeedTreeXmlForest*>(resHash);
	Forests::iterator iter = m_forests.find(def);
	if (iter != m_forests.end())
	{
		CSpeedTreeForest*	forest = iter -> second;
		//forest -> OnAreaLoadStart();
		forest -> OnAreaLoadEnd();
		forest -> Enable(true);
	}
	else
	{
		ntError_p(0, ("SPEEDTREE : Forest not found by the area system\n"));
	}
}

void SpeedTreeManager::OnAreaUnload(CHashedString resHash)
{
	const SpeedTreeXmlForest* def = ObjectDatabase::Get().GetPointerFromName<SpeedTreeXmlForest*>(resHash);
	Forests::iterator iter = m_forests.find(def);
	if (iter != m_forests.end())
	{
		CSpeedTreeForest*	forest = iter -> second;
		forest -> Enable(false);
	}
	else
	{
		ntError_p(0, ("SPEEDTREE : Forest not found by the area system\n"));
	}

}



CSpeedTreeWrapper*  SpeedTreeManager::GetUniqueTree(const SpeedTreeXmlTree* treeDef)
{
	ntAssert(m_treeRegister);

	CUniqueTreeElement	newElement(treeDef);

	CTreeRegister::UniqueTreeContainer::iterator iter = m_treeRegister -> cont_.find(newElement);

	if (iter != m_treeRegister -> cont_.end())
	{
		return iter -> second;
	}
	else
	{
		return NULL;
	}


}
void				SpeedTreeManager::AddUniqueTree(CSpeedTreeWrapper* tree, const SpeedTreeXmlTree* treeDef)
{
	ntAssert(m_treeRegister);

	CUniqueTreeElement	newElement(treeDef);

	typedef ntstd::pair<CTreeRegister::UniqueTreeContainer::iterator, bool> InsertResultType;

	InsertResultType result = m_treeRegister -> cont_.insert(CTreeRegister::UniqueTreeContainer::value_type(newElement, tree));

	ntAssert(result.second)

	UNUSED(result);

}

unsigned int SpeedTreeManager::GetVRAMFootprint(unsigned int& vertexSize, unsigned int& indexSize, unsigned int& textureSize)
{
	CUniquePtrContainer	uniqueTextures;

	vertexSize	= 0;
	indexSize	= 0;
	textureSize	= 0;

	if (m_treeRegister)
	{
		for ( CTreeRegister::UniqueTreeContainer::iterator iter = m_treeRegister -> cont_.begin(); iter != m_treeRegister -> cont_.end(); ++ iter)
		{
			CSpeedTreeWrapper*	tree = iter -> second;
			
			vertexSize += tree -> GetVertexFootprint();
			indexSize += tree -> GetIndexFootprint();
			textureSize += tree -> GetTextureFootprint(uniqueTextures);
		}
	}

	return vertexSize + indexSize + textureSize;
}


#ifdef SPEEDTREE_COLLECT_STATS

struct SpeedTreePredicate : public ntstd::binary_function<const SpeedTreeXmlTree*, const SpeedTreeXmlTree*, bool>
{
	bool operator() (const SpeedTreeXmlTree* lhs, const SpeedTreeXmlTree* rhs)
	{
		return lhs -> m_templateName.compare(rhs->m_templateName);
	}
};

COMMAND_RESULT SpeedTreeManager::DumpStats()
{
	ntPrintf("====================SPEEDTREE=====================\n");

	ntPrintf("Forest : UniqueTrees : TotalTrees : Unique Template\n");

	unsigned int totalUnique = 0;
	unsigned int totalInstances = 0;

	typedef ntstd::Set<SpeedTreeXmlTree*, SpeedTreePredicate>	UniqueTemplateList;
	typedef ntstd::pair<UniqueTemplateList::iterator, bool>		InsertResult;

	UniqueTemplateList	uniqueTemplatePerLevel;

	for (Forests::iterator iter = m_forests.begin(); iter != m_forests.end(); ++ iter)
	{
		const SpeedTreeXmlForest*	xmlDef = iter -> first;
//		const CSpeedTreeForest*		forest	= iter -> second;

		const char* name = ntStr::GetString(ObjectDatabase::Get().GetNameFromPointer(xmlDef));
		unsigned int	numUniqueTrees = xmlDef -> m_list.size();
		unsigned int	numInstances = 0;

		UniqueTemplateList	uniqueTemplatePerForest;

		for (SpeedTreeXmlForest::List::const_iterator treeIter = xmlDef -> m_list.begin(); treeIter != xmlDef -> m_list.end(); ++ treeIter)
		{
			numInstances += (*treeIter) -> 	m_list.size();

			uniqueTemplatePerForest.insert(*treeIter);
			uniqueTemplatePerLevel.insert(*treeIter);
		}

		ntPrintf("%s : %d : %d : %d : %d\n", name, numUniqueTrees, numInstances, uniqueTemplatePerForest.size(), uniqueTemplatePerLevel.size());

		totalUnique += numUniqueTrees;
		totalInstances += numInstances;
	}

	ntPrintf("Template name : Vertex footprint : Index footprint : Total\n");
	for (UniqueTemplateList::iterator iter = uniqueTemplatePerLevel.begin(); iter != uniqueTemplatePerLevel.end(); ++ iter)
	{
		CSpeedTreeWrapper*	tree = NULL;
		for (Forests::iterator forestIter = m_forests.begin(); forestIter != m_forests.end(); ++ forestIter)
		{
			if ((tree = (forestIter -> second) -> DebugGetTree(*iter)))
			{
				break;
			}
		}
		ntError_p(tree, ("Tree not found\n"));
		unsigned int vertexSize = tree -> GetVertexFootprint();
		unsigned int indexSize = tree -> GetIndexFootprint();
		ntPrintf("%s : %d : %d : %d\n", ntStr::GetString((*iter) -> m_templateName), vertexSize, indexSize, indexSize + vertexSize);
	}

	ntPrintf("Total : %d  %d : %d\n", m_forests.size(), totalUnique, totalInstances);

	ntPrintf("==================================================\n");

	return CR_SUCCESS;
}
#endif
