#include "SpeedTreeManager.h"

#include "gfx/fxmaterial.h"
#include "core/visualdebugger.h"
#include "core/osddisplay.h"
#include "input/inputhardware.h"		// for debug control only
#include "speedtree/SpeedTreeForest.h"
#include "core/gatso.h"

//! constructor
SpeedTreeManager::SpeedTreeManager()
	:m_pForest(0)
	,m_fAccumTime(0.0f)
	,m_bSpeedTreeContext(false)
	,m_bSpeedTreeRender(true)
{
	m_allMaterial.reserve(4);

	m_pBranchMaterial = FXMaterialManager::Get().FindMaterial("speedtree_branch");
	m_allMaterial.push_back(m_pBranchMaterial);
	m_pFrondMaterial = FXMaterialManager::Get().FindMaterial("speedtree_frond");
	m_allMaterial.push_back(m_pFrondMaterial);
	m_pLeafMaterial = FXMaterialManager::Get().FindMaterial("speedtree_leaf");
	m_allMaterial.push_back(m_pLeafMaterial);
	m_pBillboardMaterial = FXMaterialManager::Get().FindMaterial("speedtree_billboard");
	m_allMaterial.push_back(m_pBillboardMaterial);
	
	// debug sdtuff
	if(true)
	{
		D3DXEFFECT_DESC effectDesc;
		HRESULT hr;
		for(u32 iMat = 0 ; iMat < m_allMaterial.size() ; ++iMat )
		{
			hr = m_allMaterial[iMat]->GetEffect()->GetDesc(&effectDesc);
		}
	}
	
	m_pForest = NT_NEW CSpeedTreeForest();
	m_pForest->Load("data/speedTree/Sample_Trees.stf");
	
	m_ulRenderBitVector = Forest_RenderAll;
}

//! destructor
SpeedTreeManager::~SpeedTreeManager()
{
	NT_DELETE( m_pForest );
}

void SpeedTreeManager::CheckKey()
{
	if ( CInputHardware::Get().GetKeyboardP()->IsKeyPressed( KEYC_S, KEYM_CTRL | KEYM_ALT | KEYM_SHIFT) )
	{
		m_bSpeedTreeContext = !m_bSpeedTreeContext;
		OSD::Add(OSD::DEBUG_CHAN, 0xffffffff, "Speedtree context: %s" , (m_bSpeedTreeContext?"on":"off") );
	}
	if ( CInputHardware::Get().GetKeyboardP()->IsKeyPressed( KEYC_X, KEYM_CTRL | KEYM_ALT | KEYM_SHIFT) )
	{
		m_bSpeedTreeRender = !m_bSpeedTreeRender;
		OSD::Add(OSD::DEBUG_CHAN, 0xffffffff, "Speedtree render: %s" , (m_bSpeedTreeRender?"on":"off") );
	}
	if ( CInputHardware::Get().GetKeyboardP()->IsKeyPressed( KEYC_A, KEYM_CTRL | KEYM_ALT | KEYM_SHIFT) )
	{
		m_bSpeedTreeAnimate = !m_bSpeedTreeAnimate;
		OSD::Add(OSD::DEBUG_CHAN, 0xffffffff, "Speedtree anim: %s" , (m_bSpeedTreeAnimate?"on":"off") );
	}
	if ( CInputHardware::Get().GetKeyboardP()->IsKeyPressed( KEYC_1, KEYM_CTRL | KEYM_ALT | KEYM_SHIFT) )
	{
		m_pForest->SetRenderBitVector(m_pForest->GetRenderBitVector()^Forest_RenderLeaves);
		OSD::Add(OSD::DEBUG_CHAN, 0xffffffff, "Forest_RenderLeaves: %s" , ((m_pForest->GetRenderBitVector()&Forest_RenderLeaves)?"on":"off") );
	}
	if ( CInputHardware::Get().GetKeyboardP()->IsKeyPressed( KEYC_2, KEYM_CTRL | KEYM_ALT | KEYM_SHIFT) )
	{
		m_pForest->SetRenderBitVector(m_pForest->GetRenderBitVector()^Forest_RenderFronds);
		OSD::Add(OSD::DEBUG_CHAN, 0xffffffff, "Forest_RenderFronds: %s" , ((m_pForest->GetRenderBitVector()&Forest_RenderFronds)?"on":"off") );
	}
	if ( CInputHardware::Get().GetKeyboardP()->IsKeyPressed( KEYC_3, KEYM_CTRL | KEYM_ALT | KEYM_SHIFT) )
	{
		m_pForest->SetRenderBitVector(m_pForest->GetRenderBitVector()^Forest_RenderBranches);
		OSD::Add(OSD::DEBUG_CHAN, 0xffffffff, "Forest_RenderBranches: %s" , ((m_pForest->GetRenderBitVector()&Forest_RenderBranches)?"on":"off") );
	}
	if ( CInputHardware::Get().GetKeyboardP()->IsKeyPressed( KEYC_4, KEYM_CTRL | KEYM_ALT | KEYM_SHIFT) )
	{
		m_pForest->SetRenderBitVector(m_pForest->GetRenderBitVector()^Forest_RenderBillboards);
		OSD::Add(OSD::DEBUG_CHAN, 0xffffffff, "Forest_RenderBillboards: %s" , ((m_pForest->GetRenderBitVector()&Forest_RenderBillboards)?"on":"off") );
	}
}

void SpeedTreeManager::DebugRender()
{
	if(m_bSpeedTreeContext)
	{
		SpeedTreeStat stat = m_pForest->GetStat();
		Vec2 position(10,10);
		g_VisualDebug->Printf2D(position[0],position[1],DC_PURPLE,0,"ALL = %i", stat.GetTotal());
		position[1]+=10;
		g_VisualDebug->Printf2D(position[0],position[1],DC_PURPLE,0,"Branch count = %i", stat.m_BranchTriangleCount);
		position[1]+=10;
		g_VisualDebug->Printf2D(position[0],position[1],DC_PURPLE,0,"Frond count = %i", stat.m_FrondTriangleCount);
		position[1]+=10;
		g_VisualDebug->Printf2D(position[0],position[1],DC_PURPLE,0,"Leaf count = %i", stat.m_leafTriangleCount);
		position[1]+=10;
		g_VisualDebug->Printf2D(position[0],position[1],DC_PURPLE,0,"Billboard count = %i", stat.m_BillboardTriangleCount);
	}
}

//// load a tree
//CSpeedTreeWrapper* Load(const std::string& sptFileName)
//{
//	CSpeedTreeWrapper* pRes = new CSpeedTreeWrapper();
//	pRes->LoadTree(sptFileName.c_str());
//	m_treeWrapperContainer[sptFileName] = pRes;
//	return pRes;
//}

//void SpeedTreeManager::Animate(float fElapsedTime)
//{
//	CheckKey();
//
//	if(m_bSpeedTreeAnimate)
//	{
//		static float fLastFrameTime = 0.0f;
//		static int nFrameCount = 0;
//		const int c_nFrameInterval = 30;
//
//		// compute time
//		m_fAccumTime += fElapsedTime;
//
//		// advance wind
//		CSpeedTreeRT::SetTime(m_fAccumTime);
//
//		// report frame rate
//		//++nFrameCount;
//		//if (!(nFrameCount % c_nFrameInterval))
//		//{
//		//	static float fLastReportTime = m_fAccumTime;
//		//	float fTimeSinceLastReport = m_fAccumTime - fLastReportTime;
//
//		//	if (fTimeSinceLastReport > 0.0f)
//		//	{
//		//		float fRate = c_nFrameInterval / fTimeSinceLastReport;
//		//		fLastReportTime = m_fAccumTime;
//		//		printf("\t%.1f Hz\n", fRate);
//		//	}
//		//}
//		fLastFrameTime = m_fAccumTime;
//	}
//}

void SpeedTreeManager::UpdateForLight()
{
	CGatso::Start( "SpeedTreeManager::UpdateForLight" );
	m_pForest->UpdateForLight();
	CGatso::Stop( "SpeedTreeManager::UpdateForLight" );
}
void SpeedTreeManager::UpdateForCamera()
{
	CGatso::Start( "SpeedTreeManager::UpdateForCamera" );
	m_pForest->UpdateForCamera();
	CGatso::Stop( "SpeedTreeManager::UpdateForCamera" );
}
void SpeedTreeManager::UpdateWind()
{
	CGatso::Start( "SpeedTreeManager::UpdateWind" );
	m_pForest->UpdateWind();
	CGatso::Stop( "SpeedTreeManager::UpdateWind" );
}
void SpeedTreeManager::PerFrameUpdate(float fElapsedTime)
{
	CGatso::Start( "SpeedTreeManager::PerFrameUpdate" );

	CheckKey();
	if(m_bSpeedTreeAnimate)
	{
		static float fLastFrameTime = 0.0f;

		// compute time
		m_fAccumTime += fElapsedTime;

		// advance wind
		CSpeedTreeRT::SetTime(m_fAccumTime);
		fLastFrameTime = m_fAccumTime;
	}

	UpdateWind();
	//m_pForest->UpdateLod();
	CGatso::Stop( "SpeedTreeManager::PerFrameUpdate" );
}

