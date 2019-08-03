
#include "clouds/cloud_main_pc.h"
#include "gfx/sector_pc.h"
#include "anim/hierarchy.h"
#include "camera/camman_public.h"
#include "core/io.h"
#include "core/visualdebugger.h"
#include "gfx/texturemanager.h"
#include "gfx/depthhazeconsts.h"
#include "gfx/renderersettings.h"
#include "game/entity.h"
#include "game/entity.inl"
#include "gfx/hardwarecaps.h"
#include "tbd/franktmp.h"
#include "core/boostarray.inl"
#include "core/gatso.h"
#include "gfx/clump.h"			
#include "gfx/meshinstance.h"
//#include "gfx/rendertargetcache_pc.h"
#include "gfx/proc_vertexbuffer.h"
#include "gfx/graphicsdevice.h"
#include "gfx/surfacemanager.h"
#include "effect/effect_resourceman.h"
#include "game/entitymanager.h"




using namespace GridAccess;
using namespace FrankMisc;
using namespace SimpleFunction;






class CCloudsInstance: public CMeshInstance
{
public:
	//! constructor
	CCloudsInstance(Transform const* pobTransform, CMeshHeader const* pobMeshHeader):
		CMeshInstance(pobTransform,pobMeshHeader,true,false,false) {}
	
	void SetMaterial(const ntstd::String& name)
	{
		FXMaterial* pLitMaterial = FXMaterialManager::Get().FindMaterial(name.c_str());
		if(!pLitMaterial)
		{
			pLitMaterial = FXMaterialManager::Get().FindMaterial("lambert_debug");
			ntAssert(pLitMaterial);
		}

		this->ForceFXMaterial(pLitMaterial);
	}


	//! Renders the game material for this renderable.
	virtual void SetHeight(float fHeight)
	{
		FXMaterialInstance* pEmi = static_cast<FXMaterialInstance*>(GetMaterialInstance());
		const FXHandle& handle = pEmi->GetEffectMaterial()->GetEffect();
		FX_SET_VALUE_VALIDATE(handle , "m_height", &fHeight, sizeof(float) );
	}
	
	//! Renders the game material for this renderable.
	virtual void SetWeight(const CVector& weight)
	{
		FXMaterialInstance* pEmi = static_cast<FXMaterialInstance*>(GetMaterialInstance());
		const FXHandle& handle = pEmi->GetEffectMaterial()->GetEffect();
		FX_SET_VALUE_VALIDATE(handle , "m_f4Weight", &weight, sizeof(CVector) );
	}

	//! Renders the game material for this renderable.
	virtual void RenderMaterial()
	{
		ntAssert(m_pMaterial);
		m_pMaterial->PreRender( m_pobTransform, false );
		RenderMesh();
		m_pMaterial->PostRender();
	}
}; // end of class CHairInstance








CloudDefInterpolation::CloudDefInterpolation(const ntstd::List< CloudsLayerMutableInterface* >& obLayerMutable)
{
	int iSize = obLayerMutable.size();
	m_defArray.reserve(iSize);
	
	// filling array
	ntstd::List<CloudsLayerMutableInterface*>::const_iterator it = obLayerMutable.begin();
	for(int iDef = 0 ; iDef < iSize ; iDef++,it++ )
	{
		const CloudsLayerMutable& layer = *(*it);
		m_defArray.push_back(CloudDefInterpolationElem(&layer,layer.m_fInitialInfluence));
	}
}

void CloudDefInterpolation::Update(const TimeInfo& time)
{
	TimeSequenceManager::Update(time);
	
	////  loop on morphers
	//ntstd::List< GotoSequence<float> >::iterator it = m_morpherArray.begin();
	//while(it != m_morpherArray.end())
	//{
	//	it->Update(CTimer::Get().GetGameTimeInfo());
	//	if(!it->IsRunning())
	//	{
	//		ntstd::List< GotoSequence<float> >::iterator destroy = it;
	//		it++;
	//		m_morpherArray.erase(destroy);
	//	}
	//	else
	//	{
	//		it++;
	//	}
	//}
	
	// compute current:
	m_current.Clear();
	
	// find total
	float fTotal = 0.0f;
	for(u_int iDef = 0 ; iDef < m_defArray.size() ; iDef++ )
	{
		fTotal += m_defArray[iDef].m_fInfluence;
	}
	
	// add all influence to current
	for(u_int iDef = 0 ; iDef < m_defArray.size() ; iDef++ )
	{
		float fCoef = m_defArray[iDef].m_fInfluence / fTotal;
		const CloudsLayerMutable& layer = *m_defArray[iDef].m_pDef;
		m_current += layer * fCoef;
	}
}

void CloudDefInterpolation::AddInfluence(int iDef, float fDuration, float fNewInfluence)
{
	if((iDef>=0) && (iDef< static_cast<int>(m_defArray.size())))
	{
		GotoSequence<float>* pElem =
			NT_NEW GotoSequence<float>(&m_defArray[iDef].m_fInfluence,fNewInfluence,fDuration,TimeSequence::F_PLAYWHENCREATED);
		
		AddNewSequence(pElem);
	}
}

void CloudDefInterpolation::FocusInfluence(int iDefFocused, float fDuration)
{
	for(int iDef = 0 ; iDef < static_cast<int>(m_defArray.size()) ; iDef++ )
	{
		AddInfluence(iDef, fDuration, (iDef==iDefFocused) ? 1.0f : 0.0f );
	}
}







#ifdef _CLOUD_PROFILING
	
#include "gfx/graphing.h"
#include "input/inputhardware.h"
#include "gfx/renderer.h"

CloudProfiling::SubClass::~SubClass()
{
	//NT_DELETE_CHUNK(Mem::MC_GFX,  m_pSet );
}

CloudProfiling::SubClass::SubClass(const char* pcName, uint32_t dwColour, int iGraphSample, CGraph* pGraph):
	m_name(pcName),
	m_fValue(0.0f)
{
	m_pSet = pGraph->AddSampleSet( pcName, iGraphSample, dwColour);
}

/// constructor
CloudProfiling::CloudProfiling(int iGraphSample, int iNbFrame):
	m_pobGraph(0),
	m_iGraphSample(iGraphSample),
	m_bDrawGraph(false),
	m_bDrawText(true),
	m_fOneFrameDuration(0.0f),
	m_iNbFrame(iNbFrame)
{
	Init();
	InitStat();
	InitTimer();
}

/// destructor
CloudProfiling::~CloudProfiling()
{
	Release(); 
}

void CloudProfiling::Init()
{
	m_fOneFrameDuration = 1000.0f / 30.0f;
	m_pobGraph = NT_NEW CGraph( GRAPH_TYPE_ROLLING );
	m_pobGraph->SetYAxis( 0.0f, 1.0f, 0.25f );
	m_setArray[TOTAL] = SubClass("TOTAL", NTCOLOUR_ARGB(255,255,255,255),m_iGraphSample, m_pobGraph);
	m_setArray[NOISE] = SubClass("NOISE", NTCOLOUR_ARGB(255,255,0,255),m_iGraphSample, m_pobGraph);
	m_setArray[VISIBILITY] = SubClass("VISIBILITY", NTCOLOUR_ARGB(255,255,0,0),m_iGraphSample, m_pobGraph);
	m_setArray[DRAW] = SubClass("DRAW", NTCOLOUR_ARGB(255,255,255,0),m_iGraphSample, m_pobGraph);
	m_setArray[UPDATEFILE] = SubClass("UPDATEFILE", NTCOLOUR_ARGB(255,128,128,0),m_iGraphSample, m_pobGraph);
	m_setArray[PROFILING] = SubClass("PROFILING", NTCOLOUR_ARGB(255,0,128,128),m_iGraphSample, m_pobGraph);
	m_setArray[BETWEEN] = SubClass("BETWEEN", NTCOLOUR_ARGB(255,155,155,155),m_iGraphSample, m_pobGraph);
	m_setArray[DEBUG] = SubClass("DEBUG", NTCOLOUR_ARGB(255,200,50,128),m_iGraphSample, m_pobGraph);
}

void CloudProfiling::InitTimer()
{
	for(int iSet = 0 ; iSet < m_ArraySize ; iSet++ )
	{
		if(iSet == PROFILING) continue;
		m_setArray[iSet].m_fValue = 0.0f;
	}
}

void CloudProfiling::Release()
{
	NT_DELETE_CHUNK(Mem::MC_GFX,  m_pobGraph );
}

void CloudProfiling::Draw(float fDangerousNumber)
{
	// some useful value
	uint32_t dwColor = NTCOLOUR_ARGB(255,122,122,122);
	char mess[256];
	Pixel2 vpSize(
		static_cast<int>( g_VisualDebug->GetDebugDisplayWidth() ),
		static_cast<int>( g_VisualDebug->GetDebugDisplayHeight() ));
	int iTextDec = 20;
	
	for(int iSet = 0 ; iSet < m_ArraySize ; iSet++ )
	{
		float fFrame = m_setArray[iSet].m_fValue / m_fOneFrameDuration;
		m_setArray[iSet].m_pSet->AddSample(fFrame / fDangerousNumber);
	}
	
	// draw graph
	if(m_bDrawGraph)
	{
		// show our own graph
		float fHeight = 300.0f;
		float fWidth = m_iGraphSample * 2.0f;
		CPoint obTopLeft( 20.0f, static_cast<float>(vpSize[1]) - 20.0f - fHeight, 0.0f );
		CPoint obBottomRight( 20.0f + fWidth, static_cast<float>(vpSize[1]) - 20.0f, 0.0f );
		
		g_VisualDebug->RenderGraph( m_pobGraph, obTopLeft, obBottomRight );
		
		// text
		int dec = 0;
		int fY = vpSize[1]-30;
		int fX = 20;
		sprintf(mess,"Nb frame = %f", fDangerousNumber);
		CCamUtil::DebugPrintf(fX, vpSize[1]-fY-iTextDec*dec--, NTCOLOUR_ARGB(255,255,255,255),  mess);
		
		for(int iSet = 0 ; iSet < m_ArraySize ; iSet++ )
		{
			sprintf(mess, ntStr::GetString(m_setArray[iSet].m_name));
			CCamUtil::DebugPrintf(fX, vpSize[1]-fY-iTextDec*dec--, m_setArray[iSet].m_pSet->GetColour(),  mess);
		}
	}
	
	if(m_bDrawText)
	{
		// debug text
		int fX = vpSize[0]-350;
		int fY = 10;
		int iDec = 0;
		
		sprintf(mess,"FillRate = %f", m_fFillRate);
		CCamUtil::DebugPrintf(fX, fY+iTextDec*iDec++, dwColor, mess);
		
		sprintf(mess,"Total = %f",m_setArray[TOTAL].m_fValue / m_fOneFrameDuration);
		CCamUtil::DebugPrintf(fX, fY+iTextDec*iDec++, dwColor, mess);

		sprintf(mess,"Between = %f",m_setArray[BETWEEN].m_fValue / m_fOneFrameDuration);
		CCamUtil::DebugPrintf(fX, fY+iTextDec*iDec++, dwColor, mess);
	}
	
}



#endif // _CLOUD_PROFILING











//! constructor
GPUprecomputedNoise::GPUprecomputedNoise(Pixel4 freq, Pixel4 nbSamplePerCube, int iSeed)
	:m_pTexture(0)
	,m_freq(freq)
	,m_nbSamplePerCube(nbSamplePerCube)
	,m_iSeed(iSeed)
{
	Create();
	Fill();
}

//! destructor
GPUprecomputedNoise::~GPUprecomputedNoise()
{
	Release();
}

// release object
bool GPUprecomputedNoise::Release()
{
	if(m_pTexture != 0)
	{
		m_pTexture->Release();
		m_pTexture=0;
		return true;
	}
	else
	{
		return false;
	}
}

//void GPUprecomputedNoise::DrawDebug()
//{
//	HRESULT hr;
//	
//	hr = Renderer::Get().GetDevice()->SetFVF(D3DFVF_XYZ);
//	ntAssert(SUCCEEDED(hr));
//	Array<Vec3,4> data;
//	int iNbSquare = 10;
//	for(int iSquare = 0 ; iSquare < iNbSquare ; iSquare++ )
//	{
//		float z = iSquare / static_cast<float>(iNbSquare-1);
//		data[0]=Vec3(0,0,z);
//		data[1]=Vec3(1,0,z);
//		data[2]=Vec3(0,1,z);
//		data[3]=Vec3(1,1,z);
//		hr = GetD3DDevice()->DrawPrimitiveUP(D3DPT_TRIANGLEFAN,2,&data[0],sizeof(data));
//		ntAssert(SUCCEEDED(hr));
//	}
//}




// create
bool GPUprecomputedNoise::Create()
{
	HRESULT hr = S_OK;
	Release();
	Pixel4 size = m_freq * m_nbSamplePerCube;
	
	hr = GetD3DDevice()->CreateVolumeTexture(size[0],size[1],size[2],1,0,D3DFMT_A8,D3DPOOL_MANAGED,&m_pTexture,0);
	return SUCCEEDED(hr);
}

bool GPUprecomputedNoise::Fill()
{
	Pixel4 size = m_freq * m_nbSamplePerCube;
	typedef u_char TexElemType;
	GradData grid(Pixel4(m_freq[0],m_freq[1],m_freq[2],m_freq[3]) + Pixel4(1,1,1,1),m_iSeed);
	grid.SetTilable();
	PerlinTest perlin(&grid);
	
	HRESULT hr;
	ntAssert(m_pTexture!=0);
	
	D3DLOCKED_BOX box;
	hr = m_pTexture->LockBox(0,&box,0,0);
	if(FAILED(hr)) return false;
	u_char* pArray = static_cast<u_char*>(box.pBits);
	
	CVector scale = CVector(
		1.0f/static_cast<float>(m_nbSamplePerCube[0]),
		1.0f/static_cast<float>(m_nbSamplePerCube[1]),
		1.0f/static_cast<float>(m_nbSamplePerCube[2]),
		1.0f);
	
	Pixel3 texCoord;
	for(texCoord[2] = 0 ; texCoord[2] < size[2] ; texCoord[2]++ )
	{
		for(texCoord[1] = 0 ; texCoord[1] < size[1] ; texCoord[1]++ )
		{
			for(texCoord[0] = 0 ; texCoord[0] < size[0] ; texCoord[0]++ )
			{
				CVector coord(
					static_cast<float>(texCoord[0]),
					static_cast<float>(texCoord[1]),
					static_cast<float>(texCoord[2]),
					0);
				coord*=scale;
				
				float fRes =perlin.Evaluate(coord);
				int iIndex = texCoord[2] * box.SlicePitch + texCoord[1] * box.RowPitch + texCoord[0];
				pArray[iIndex] = static_cast<u_char>(0.5*(fRes+1.0f)*255.0f);
			}
		}
	}
	
	hr = m_pTexture->UnlockBox(0);
	if(FAILED(hr)) return false;

	return true;
}





/// constructor
Observer::Observer()
{
	m_mask.Set(Observer::F_JUSTCONSTRUCTED);
	//m_pHero = CEntityManager::Get().GetPlayer();
	
	m_fTime = 0.0f;
	m_fTimeChange = 0.0f;
	m_dInitTime = CTimer::Get().GetGameTime();
	//m_fCycleTime = LevelLighting::Get().GetTimeOfDay();
	
	m_fCloudTimeChange = 0.0f;
	m_fCloudTime = 0.0f;
	
}

void Observer::PerFrameUpdate()
{
	// is second update
	if(m_mask.AllOfThem(Observer::F_FIRSTUPDATE))
	{
		m_mask.Unset(Observer::F_FIRSTUPDATE);
	}
	// if first update
	else if(m_mask.AllOfThem(Observer::F_JUSTCONSTRUCTED))
	{
		m_mask.Set(Observer::F_FIRSTUPDATE);
		m_mask.Unset(Observer::F_JUSTCONSTRUCTED);
	}
	
	// camera
	m_pCamera = RenderingContext::Get()->m_pViewCamera;
	
	// matrices
	m_invWorldToEye = m_pCamera->GetViewTransform()->GetWorldMatrix();
	//m_worldToEye = m_invWorldToEye.GetFullInverse();
	m_worldToEye = RenderingContext::Get()->m_worldToView;
	m_eyeToScreen = RenderingContext::Get()->m_viewToScreen;
	m_worldToScreen = RenderingContext::Get()->m_worldToScreen;
	
	//m_invEyeToScreen = m_eyeToScreen.GetFullInverse();	
	//m_invWorldToScreen = m_worldToScreen.GetFullInverse();
		
	// world position
	m_worldPosition = CVector(m_invWorldToEye.GetTranslation());
	m_worldPosition.W() = 1.0f;
	m_worldCamDirection = CVector(m_invWorldToEye[2]);
	m_worldCamDirection.W() = 0.0f;
	
	
	// screen stuff
	m_invScreenSize = CVector(1.0f/RenderingContext::Get()->m_fScreenAspectRatio,1.0f,0.0f,0.0f);
	m_fScreenRadius = sqrt(1.0f*1.0f +RenderingContext::Get()->m_fScreenAspectRatio*RenderingContext::Get()->m_fScreenAspectRatio);




	// time
	//m_fCycleTime = LevelLighting::Get().GetTimeOfDay();
	
	m_fTime = static_cast<float>(CTimer::Get().GetGameTime()-m_dInitTime);
	m_fTimeChange = CTimer::Get().GetGameTimeChange();
	
	//float fOldCycleTime = m_fCycleTime;
	//m_fCycleTime = LevelLighting::Get().GetTimeOfDay();
	//m_fCycleTimeChange = m_fTime-fOldCycleTime;
	
	CEntity* pHero = CEntityManager::Get().GetPlayer();
	if(pHero)
	{
		m_fTimeMultiplier = pHero->GetTimeMultiplier();
	}
	else
	{
		m_fTimeMultiplier = 1.0f;
	}
	
	m_fTimeMultiplier -= 1.0f;
	m_fTimeMultiplier *= 10.0f;
	m_fTimeMultiplier += 1.0f;
	
	m_fCloudTimeChange = m_fTimeChange * m_fTimeMultiplier;
	m_fCloudTime += m_fCloudTimeChange;
	
	//m_fHeroTime = m_fTime * m_fTimeMultiplier;
	//m_fHeroTimeChange = m_fTimeChange * m_fTimeMultiplier;
}


bool Observer::Discontinuity()
{
	return m_mask.AllOfThem(Observer::F_FIRSTUPDATE);
}







CloudsNoise::CloudsNoise(const CloudsNoiseDefInterface* pDef):
	m_pDef(pDef),
	m_fReactiveTime(0.0f),
	m_fFinalTime(0.0f),
	m_position(CONSTRUCT_CLEAR)
{
	// nothing
}





// per frame update
void CloudsNoise::Update(float fIncTime)
{
	m_uProperty = M_NONE;
	
	m_fReactiveTime += m_pDef->m_fTimeSpeed * fIncTime;
	m_fFinalTime = m_fReactiveTime + m_pDef->m_fTimeOrigin;
	
	m_position += m_pDef->m_obWindSpeed * fIncTime;
	m_position.W() = 0;
}




	










namespace CloudsNS
{
	void PerColumnVertexInfo::ResetArray(int iNbSubLayers)
	{
		for(int iSubLayer = 0 ; iSubLayer <= iNbSubLayers ; iSubLayer++ )
		{
			m_vertexArray[iSubLayer].Reset();
		}
	}
	
	PerColumnVertexInfo::PerColumnVertexInfo():
		m_normalisedPosition(CONSTRUCT_CLEAR),
		m_mask()
	{
		ResetArray(g_iMaxLayer-1);
	}
} // end of namespace cloudsNS







using namespace CloudsNS;



void CloudLayer::SetParent(const Clouds* pClouds,int iNumLayer)
{
	ntAssert(m_pClouds==0);
	m_pClouds = pClouds;
	m_iNumLayer = iNumLayer;
}

CloudLayer::CloudLayer(const CloudsLayerInterface* pDef):
	m_pDef(pDef),
	m_mutableDef(pDef->m_obLayerMutable),
	m_pClouds(0),
	m_fAngle(0.0f),
	m_pVertexBuffer(0),
	m_globalAmplitude(m_mutableDef.Get().m_fGlobalDensityMin,m_mutableDef.Get().m_fGlobalDensityMax,2.0f,TimeSequence::F_MIRROR)
{
	ntAssert(m_pDef->m_iNbSubLayer<g_iMaxLayer);
	SetNewSubdivideLevel(m_pDef->m_iNbSubdivision);
	CreateVertexBuffer();
	//SetTiming();	
}

void CloudLayer::SetTiming()
{
	m_globalAmplitude =
		LinearInterpolation<float>(m_mutableDef.Get().m_fGlobalDensityMin,m_mutableDef.Get().m_fGlobalDensityMax,2.0f,TimeSequence::F_MIRROR);
}

// per frame update
void CloudLayer::SetNewSubdivideLevel(int iNbSubdivide)
{
	InitDirtySize();
	Subdivide4Info tmpInf(Subdivide4Info::ISOCAHEDRON20, iNbSubdivide);
	m_facesContainer.Reset(tmpInf.m_iNbFaces,m_dirtySize[iNbSubdivide][0]);
	m_verticesContainer.Reset(tmpInf.m_iNbVertices,m_dirtySize[iNbSubdivide][1]);
	BreakTimeCoherency();
}



Triangle CloudLayer::CreateTrig(const PerColumnFaceInfo& faceInfo)
{
	return Triangle(
		m_verticesContainer[faceInfo.m_indices[0]].m_normalisedPosition,
		m_verticesContainer[faceInfo.m_indices[1]].m_normalisedPosition,
		m_verticesContainer[faceInfo.m_indices[2]].m_normalisedPosition);
}


void CloudLayer::InitVisibilityTreshold()
{
	m_fSureItsVisible = m_mutableDef.Get().m_fSureItsVisible;
	m_fCullCoef = m_mutableDef.Get().m_fDotCullCoef;
	
	float fMaxLod = static_cast<float>(m_pDef->m_iNbSubdivision);
	//float fMaxCull = (1.0f - m_pDef->m_fTerrainAltitudeRate) * m_fCullCoef;
	for(int iSub = 0 ; iSub <= m_pDef->m_iNbSubdivision ; iSub++ )
	{
		m_bbSphereTreshold[iSub] = (m_fCullCoef * iSub) / fMaxLod;	
	}
	
	m_fScreenSizeX = Vec2(m_mutableDef.Get().m_obScreenSizeX.X(),m_mutableDef.Get().m_obScreenSizeX.Y());
	m_fScreenSizeY = Vec2(m_mutableDef.Get().m_obScreenSizeY.X(),m_mutableDef.Get().m_obScreenSizeY.Y());
}

void CloudLayer::InitDirtySize()
{
	Subdivide4Info tmpInf(Subdivide4Info::ISOCAHEDRON20);
	for(int iLod = 0 ; iLod <= g_iMaxLod ; iLod++ )
	{
		m_dirtySize[iLod] = Pixel2(tmpInf.m_iNbFaces/7,tmpInf.m_iNbVertices/7);
		tmpInf = tmpInf.Subdivide(tmpInf);
	}
}


void CloudLayer::PushFace(int iFace, int iLevel)
{
	ntAssert( (iLevel>=0) && (iLevel<=m_pClouds->m_pDef->m_iNbSubdivisionMax) );
	IsocahedronContainer::Face& face = m_pClouds->m_isocahedrons[iLevel]->m_faces[iFace];	
	
	CVector aa = m_pClouds->m_isocahedrons[iLevel]->m_vertices[face.m_indices[0]].m_position;
	CVector bb = m_pClouds->m_isocahedrons[iLevel]->m_vertices[face.m_indices[1]].m_position;
	CVector cc = m_pClouds->m_isocahedrons[iLevel]->m_vertices[face.m_indices[2]].m_position;
	
	// ground stuff
	float fMaxGroundDot = ntstd::Max(
		m_horizontalPlanDirection.Dot(aa),
		m_horizontalPlanDirection.Dot(bb));
	fMaxGroundDot = ntstd::Max(fMaxGroundDot,
		m_horizontalPlanDirection.Dot(cc));
	
	//float fNormalisedHeight = CloudExportFX::GetNormalisedHorizon(fMaxGroundDot,m_mutableDef.Get().m_fTerrainAltitudeRate);
	//if(fNormalisedHeight < -m_mutableDef.Get().m_fAltitudeCull)
	//{
	//	return ;
	//}
	
	// eye stuff
	CVector a = aa - m_perFrameInfo.m_eyeLocal;
	CVector b = bb - m_perFrameInfo.m_eyeLocal;
	CVector c = cc - m_perFrameInfo.m_eyeLocal;

	a/=a.Length();
	b/=b.Length();
	c/=c.Length();
	
	float fMaxEyeDot = ntstd::Max(m_perFrameInfo.m_eyePlanDirection.Dot(a),m_perFrameInfo.m_eyePlanDirection.Dot(b));
	fMaxEyeDot = ntstd::Max(fMaxEyeDot,m_perFrameInfo.m_eyePlanDirection.Dot(c));
	
	if(fMaxEyeDot<m_bbSphereTreshold[iLevel])
	{
		return;
	}
	
	// add or create new trig
	if(iLevel == m_pDef->m_iNbSubdivision)
	{
		if((m_pDef->m_bUseExtraVisibility) && (fMaxEyeDot<m_fSureItsVisible))
 		{
			aa = aa * m_perFrameInfo.m_localToScreen;
			aa /= aa.W();
			bb = bb * m_perFrameInfo.m_localToScreen;
			bb /= bb.W();
			cc = cc * m_perFrameInfo.m_localToScreen;
			cc /= cc.W();

			//m_fScreenSizeY
			//m_fScreenSizeX
			if((aa.X()<m_fScreenSizeX[0]) && (bb.X()<m_fScreenSizeX[0]) && (cc.X()<m_fScreenSizeX[0]))
			{
				return;
			}
			if((aa.X()>m_fScreenSizeX[1]) && (bb.X()>m_fScreenSizeX[1]) && (cc.X()>m_fScreenSizeX[1]))
			{
				return;
			}
			if((aa.Y()<m_fScreenSizeY[0]) && (bb.Y()<m_fScreenSizeY[0]) && (cc.Y()<m_fScreenSizeY[0]))
			{
				return;
			}
			if((aa.Y()>m_fScreenSizeY[1]) && (bb.Y()>m_fScreenSizeY[1]) && (cc.Y()>m_fScreenSizeY[1]))
			{
				return;
			}
		}
		
		if(m_facesContainer.IsValid(iFace))
		{
			m_perFrameInfo.m_allreadyHereFacesSet.insert(iFace);
		}
		else
		{
			m_perFrameInfo.m_newFacesSet.insert(iFace);
		}
	}
	else
	{
		// go on next lod
		for(int iSub = 0 ; iSub < 4 ; iSub++ )
		{
			this->PushFace(4*iFace+iSub, iLevel+1);
		}
	}
}

void CloudLayer::PerFrameUpdate()
{
	InitVisibilityTreshold();

	// TODO Deano I've removed timeinfo from CTimer for the moment... 
	// so for now this code is broken, FIXME when clouds come back from the dead
//	m_globalAmplitude.Update(CTimer::Get().GetGameTimeInfo());
//	m_mutableDef.Update(CTimer::Get().GetGameTimeInfo());
	ntAssert( false && "Deano has broken this" );
	
	// transform
	m_fAngle += m_pClouds->m_gameObserver.GetCloudTimeChange() * m_mutableDef.Get().m_fAngleSpeed;
	
	// rotation
	m_perFrameInfo.m_rotate = CMatrix(
		static_cast<CDirection>(m_mutableDef.Get().m_obDomeRotationAxis),m_fAngle);
	m_perFrameInfo.m_invRotate = CMatrix(
		static_cast<CDirection>(m_mutableDef.Get().m_obDomeRotationAxis),-m_fAngle);
	
	CQuat id(CONSTRUCT_IDENTITY);
	m_fLocalVerticalTranslation = m_mutableDef.Get().m_fTerrainAltitudeRate*m_mutableDef.Get().m_fAltitudeMin;
	m_translateVec = CVector(0.0f,-m_fLocalVerticalTranslation,0.0f,0.0f); 	
	m_translate = CMatrix(id,CPoint(m_translateVec));
	m_invTranslate = CMatrix(id,CPoint(-m_translateVec));
	
	m_scale =CMatrix(
		m_mutableDef.Get().m_fAltitudeMin,0.0f,0.0f,0.0f,
		0.0f,m_mutableDef.Get().m_fAltitudeMin,0.0f,0.0f,
		0.0f,0.0f,m_mutableDef.Get().m_fAltitudeMin,0.0f,
		0.0f,0.0f,0.0f,1.0f);
	m_invScale = CMatrix(
		1.0f/m_mutableDef.Get().m_fAltitudeMin,0.0f,0.0f,0.0f,
		0.0f,1.0f/m_mutableDef.Get().m_fAltitudeMin,0.0f,0.0f,
		0.0f,0.0f,1.0f/m_mutableDef.Get().m_fAltitudeMin,0.0f,
		0.0f,0.0f,0.0f,1.0f);
	
	m_perFrameInfo.m_localToScreen = m_perFrameInfo.m_rotate * m_scale * m_translate
		* m_pClouds->m_gameObserver.m_worldToScreen;
	
	CMatrix invLocalToWord = m_invTranslate * m_invScale * m_perFrameInfo.m_invRotate;
	
	// cam center
	m_perFrameInfo.m_eyeLocal = m_pClouds->m_gameObserver.m_worldPosition * invLocalToWord;
	m_perFrameInfo.m_eyeLocal.W() = 1.0f;
	
	// cam direction
	m_perFrameInfo.m_eyePlanDirection = (m_pClouds->m_gameObserver.m_worldPosition + m_mutableDef.Get().m_fAltitudeMin * m_pClouds->m_gameObserver.m_worldCamDirection) * invLocalToWord;
	m_perFrameInfo.m_eyePlanDirection -= m_perFrameInfo.m_eyeLocal;
	m_perFrameInfo.m_eyePlanDirection *= 1.0f / m_perFrameInfo.m_eyePlanDirection.Length();
	
	// horizontal plan coordinate
	m_horizontalPlanDirection = CVector(0.0f,1.0f,0.0f,0.0f) * m_perFrameInfo.m_invRotate;
	m_horizontalPlanDirection.W() = 0.0f;
}

void CloudLayer::GenerateCell()
{
	IsocahedronContainer& iso = *(m_pClouds->m_isocahedrons[m_pDef->m_iNbSubdivision]);
	
	m_perFrameInfo.m_allreadyHereFacesSet.clear();
	m_perFrameInfo.m_newFacesSet.clear();
	
	// generate cell
	for(int iFace = 0 ; iFace < m_pClouds->m_isocahedrons[0]->m_info.m_iNbFaces ; iFace++ )
	{
		PushFace(iFace,0);
	}
	

    // faces
    {
		ntstd::Set<int> oldFacesSet;
		ntstd::set_difference(
			m_activeFaces.begin(), m_activeFaces.end(),
			m_perFrameInfo.m_allreadyHereFacesSet.begin(), m_perFrameInfo.m_allreadyHereFacesSet.end(),
			inserter(oldFacesSet, oldFacesSet.begin()));
		
    	// replace old face by new face
	    ntstd::Set<int>::iterator itnew = m_perFrameInfo.m_newFacesSet.begin();
	    ntstd::Set<int>::iterator endnew = m_perFrameInfo.m_newFacesSet.end();
	    ntstd::Set<int>::iterator itold = oldFacesSet.begin();
	    ntstd::Set<int>::iterator endold = oldFacesSet.end();
	    while((itnew!=endnew) && (itold!=endold))
	    {
			m_facesContainer.ReLink(*itold, *itnew);
			Pixel3 indices = iso.m_faces[(*itnew)].m_indices;
	    	m_facesContainer[*itnew].Init(indices);
	    	// use olf for new
	    	itnew++;
	    	itold++;
	    }
		
		// free old
		while(itold!=endold)
		{
			m_facesContainer.Unlink(*itold);
			itold++;
		}
		
		// add new
	 	while(itnew!=endnew)
		{
	    	bool bRes =  m_facesContainer.NewLink(*itnew);
	    	if(bRes)
	    	{
	    		ntPrintf("Warning, new allocation of m_facesContainer");
	    	}
	    	Pixel3 indices = iso.m_faces[(*itnew)].m_indices;
	    	m_facesContainer[*itnew].Init(indices);
	    	itnew++;
		}
		
		// update
	    ntstd::Set<int>::iterator itInter = m_perFrameInfo.m_allreadyHereFacesSet.begin();
	    ntstd::Set<int>::iterator endInter = m_perFrameInfo.m_allreadyHereFacesSet.end();
	 	while(itInter!=endInter)
		{
	    	m_facesContainer[*itInter].PerFrameUpdate();
	    	itInter++;
		}
    }
	
	// new face container
    m_activeFaces.clear();
	ntstd::set_union(
		m_perFrameInfo.m_newFacesSet.begin(), m_perFrameInfo.m_newFacesSet.end(),
		m_perFrameInfo.m_allreadyHereFacesSet.begin(), m_perFrameInfo.m_allreadyHereFacesSet.end(),
		inserter(m_activeFaces, m_activeFaces.begin()));
   
 	// create vertices set:
 	m_perFrameInfo.m_newVerticesSet.clear();
 	m_perFrameInfo.m_allreadyHereVerticesSet.clear();
	{
		ntstd::Set<int>::iterator itaux = m_activeFaces.begin();
		ntstd::Set<int>::iterator endaux = m_activeFaces.end();
		while(itaux!=endaux)
		{
			Pixel3 indices = m_facesContainer[*itaux].m_indices;
			for(int iVertex = 0 ; iVertex < 3 ; iVertex++ )
			{
				if(m_verticesContainer.IsValid(indices[iVertex]))
				{
					m_perFrameInfo.m_allreadyHereVerticesSet.insert(indices[iVertex]);
				}
				else
				{
					m_perFrameInfo.m_newVerticesSet.insert(indices[iVertex]);
				}
			}
		    itaux++;
		}
	}
	
	// vertices
	{
		// vertices diff
		ntstd::Set<int> oldVerticesSet;
		
		ntstd::set_difference(
			m_activeVertices.begin(), m_activeVertices.end(),
			m_perFrameInfo.m_allreadyHereVerticesSet.begin(), m_perFrameInfo.m_allreadyHereVerticesSet.end(),
			inserter(oldVerticesSet, oldVerticesSet.begin()));
         
		// replace old face by new face
	    ntstd::Set<int>::iterator itnew = m_perFrameInfo.m_newVerticesSet.begin();
	    ntstd::Set<int>::iterator endnew = m_perFrameInfo.m_newVerticesSet.end();
	    ntstd::Set<int>::iterator itold = oldVerticesSet.begin();
	    ntstd::Set<int>::iterator endold = oldVerticesSet.end();
	    while((itnew!=endnew) && (itold!=endold))
	    {
			m_verticesContainer.ReLink(*itold, *itnew);
	    	m_verticesContainer[*itnew].m_mask.Set(PerColumnVertexInfo::F_NEW);
	    	// use olf for new
	    	itnew++;
	    	itold++;
	    }
		
		// free old
		while(itold!=endold)
		{
			m_verticesContainer.Unlink(*itold);
			itold++;
		}
		
		// add new
	 	while(itnew!=endnew)
		{
	    	bool bRes = m_verticesContainer.NewLink(*itnew);
	    	if(bRes)
	    	{
	    		ntPrintf("Warning, new allocation of m_verticesContainer");
	    	}
	    	m_verticesContainer[*itnew].Init(m_pDef->m_iNbSubLayer);
	    	itnew++;
		}
		
		// update
	    ntstd::Set<int>::iterator itInter = m_perFrameInfo.m_allreadyHereVerticesSet.begin();
	    ntstd::Set<int>::iterator endInter = m_perFrameInfo.m_allreadyHereVerticesSet.end();
	 	while(itInter!=endInter)
		{
	    	m_verticesContainer[*itInter].PerFrameUpdate();
	    	itInter++;
		}
	}
		
    m_activeVertices.clear();
	ntstd::set_union(
		m_perFrameInfo.m_newVerticesSet.begin(), m_perFrameInfo.m_newVerticesSet.end(),
		m_perFrameInfo.m_allreadyHereVerticesSet.begin(), m_perFrameInfo.m_allreadyHereVerticesSet.end(),
		inserter(m_activeVertices, m_activeVertices.begin()));
}



//void CloudLayer::CleanVertex()
//{
//	//m_visibleVertices.Erase();
//
//	// clean vertex
//	for(int iTrig = 0 ; iTrig < m_arrayFace.GetSize() ; iTrig++ )
//	{
//		PerColumnFaceInfo& faceInfo = m_arrayFace[iTrig];
//		for(int iVertex = 0 ; iVertex < 3 ; iVertex++ )
//		{
//			m_auxVertices.insert(faceInfo.m_indices[iVertex]);
//			//PerColumnVertexInfo& columnInfo = m_arrayVertex[faceInfo.m_indices[iVertex]];
//			//if(!columnInfo.m_mask.AllOfThem(PerColumnVertexInfo::F_REGISTERED))
//			//{
//			//	m_visibleVertices.Push(faceInfo.m_indices[iVertex]);
//			//	columnInfo.m_mask.Set(PerColumnVertexInfo::F_REGISTERED);
//			//}
//		}
//	}
//}



// 
void CloudLayer::ComputeCPUNoise()
{
	//float fSphereAngle = 3.14f * 0.5f * cos(m_pClouds->m_gameObserver.m_fTime * 0.1f);
	float fSphereAngle = 1.3f;
	CMatrix sphereRotation(CDirection(1.0f,0.0f,0.0f),fSphereAngle);


	//float fIncTime = m_pClouds->GetGameObserver().m_fHeroTimeChange;
	float fIncTime = m_pClouds->GetGameObserver().GetCloudTimeChange();
	
	// cpu noise (not really use anymore)
	{
		typedef ntstd::List<CloudsNoiseDefInterface*> NoiseList;
		const NoiseList& noiseList = m_pDef->m_obNoiseFunction;
		NoiseList::const_iterator noiseEnd = noiseList.end();
		for ( NoiseList::const_iterator it = noiseList.begin(); it != noiseEnd; ++it )
		{
			CloudsNoise& noise = (*it)->Get();
			noise.Update(fIncTime);
		}
	}
	
	// gpu noise
	{
		typedef ntstd::List<CloudsGPUNoise*> NoiseList;
		const NoiseList& noiseList = m_pDef->m_obGPUFunction;
		NoiseList::const_iterator noiseEnd = noiseList.end();
		for ( NoiseList::const_iterator it = noiseList.begin(); it != noiseEnd; ++it )
		{
			const CloudsGPUNoise& noise = *(*it);
			noise.Update(fIncTime);
		}
	}
}
 
void CloudLayer::ComputeBillboardTransform()
{
	ntstd::Set<int>::iterator itFacesEnd = m_activeFaces.end();
	for (ntstd::Set<int>::iterator itFaces = m_activeFaces.begin(); itFaces != itFacesEnd; ++itFaces )
	{
		PerColumnFaceInfo& faceInfo = m_facesContainer[*itFaces];
		const CVector& a = m_verticesContainer[faceInfo.m_indices[0]].m_normalisedPosition;
		const CVector& b = m_verticesContainer[faceInfo.m_indices[1]].m_normalisedPosition;
		const CVector& c = m_verticesContainer[faceInfo.m_indices[2]].m_normalisedPosition;
		
		// 
		CVector middle = (a+b+c)*(1.0f/3.0f);
		
		//ez
		CDirection ez = CDirection(middle - m_perFrameInfo.m_eyeLocal);
		ez /= ez.Length();
		
		// ex
		CDirection ex = CDirection(middle - a);
		ex /= ex.Length();
		
		// ey
		CDirection ey = ex.Cross(ez);
		
		// create transform matrix
		faceInfo.m_particleOffsetTransform.SetXAxis(ex);
		faceInfo.m_particleOffsetTransform.SetYAxis(ey);
		faceInfo.m_particleOffsetTransform.SetZAxis(ez);
	}
}






// return 1 if middle of layer
float CloudLayer::IsMiddle(float fHeight)
{
	fHeight = 2.0f * (fHeight-0.5f);
	
	float fTreshold = 2 * 0.1f;
	float fLimit = (1.0f - fTreshold);
	
	
	float fcenter = abs(fHeight);
	if(fcenter < fLimit)
	{
		return 1.0f;
	}
	else
	{
		float x = (fcenter-fLimit) /  fTreshold;
		return 1.0f - x*x;
	}
}




// as it said
void CloudLayer::BreakTimeCoherency()
{
	m_activeFaces.clear();
	m_activeVertices.clear();
}




// return 1 if middle top of layer
float CloudLayer::IsTop(float fHeight)
{
	fHeight = 2.0f * (fHeight-0.5f);
	
	float fTreshold = 2 * 0.1f;
	float fLimit = (1.0f - fTreshold);
	
	if(fHeight < fLimit)
	{
		return 0.0f;
	}
	else
	{
		float x = (fHeight-fLimit) /  fTreshold;
		return x*x;
	}
}

// return 1 if middle bottom of layer
float CloudLayer::IsBottom(float fHeight)
{
	fHeight = 2.0f * (fHeight-0.5f);
	return IsTop(-fHeight);
}

// return 1 if middle bottom of layer
CVector CloudLayer::GetPositionnalInf(float fHeight)
{
	CVector res(CONSTRUCT_CLEAR);
	fHeight = 2.0f * (fHeight-0.5f);
	
	float fTreshold = 2 * 0.1f;
	float fLimit = (1.0f - fTreshold);
	
	// top
	if(fHeight < fLimit)
	{
		res.Z() = 0.0f;
	}
	else
	{
		float x = (fHeight-fLimit) /  fTreshold;
		res.Z() =  x*x;
	}
	
	// bottom
	fHeight=-fHeight;
	if(fHeight < fLimit)
	{
		res.X() = 0.0f;
	}
	else
	{
		float x = (fHeight-fLimit) /  fTreshold;
		res.X() =  x*x;
	}
	
	//middle
	res.Y() = 1.0f - res.X()-res.Z();
	
	return res;
}

// compute density culling
void CloudLayer::ComputeDensityCulling()
{
	float fSmoothDensityMin = m_pClouds->m_pDef->m_fSmoothDensityMin;
	
	ntstd::Set<int>::iterator itFacesEnd = m_activeFaces.end();
	for (ntstd::Set<int>::iterator itFaces = m_activeFaces.begin(); itFaces != itFacesEnd; ++itFaces )
	{
		PerColumnFaceInfo& faceInfo = m_facesContainer[*itFaces];
		float fMaxDensity = -1.0f;
		for(int iVertex = 0 ; iVertex < 3 ; iVertex++ )
		{
			PerColumnVertexInfo& columnInfo = m_verticesContainer[faceInfo.m_indices[iVertex]];
			fMaxDensity = ntstd::Max(fMaxDensity,columnInfo.fMaxDensity);
		}
		if(fMaxDensity<fSmoothDensityMin)
		{
			// no empty cell
			//faceInfo.m_mask.Set(PerColumnFaceInfo::F_NODENSITY);
		}
	}
}




















	
// constructor
CloudLayer::~CloudLayer()
{
	NT_DELETE_CHUNK(Mem::MC_GFX,  m_pVertexBuffer );
}



void CloudLayer::CreateVertexBuffer()
{
	SAFEDELETE(m_pVertexBuffer);
	m_pVertexBuffer = NT_NEW CloudParticles(
		m_pDef->m_iNbSprites,
		PerCellDataArraySIZE,
		m_pDef->m_iSeed,
		m_mutableDef.Get().m_fPyramidHeight);
}














//
//
////! create index
//void CloudParticles2::CreateIndex()
//{
//	hr = Renderer::Get().GetDevice()->CreateIndexBuffer( (iPolySize+2) * sizeof(WORD),  0, D3DFMT_INDEX16, D3DPOOL_DEFAULT, &m_pVertexIndexArray[iPolySize], NULL );
//	ntAssert(SUCCEEDED(hr));
//	
//	WORD* pIndices;
//	hr = m_pVertexIndexArray[iPolySize]->Lock( 0, 0, (void**)&pIndices, 0 );
//	ntAssert(SUCCEEDED(hr));
//
//	for(int iFor = 0 ; iFor < iPolySize+1 ; iFor++ )
//	{
//		pIndices[iFor]=static_cast<WORD>(iFor);
//	}
//	pIndices[iPolySize+1]=static_cast<WORD>(1);
//	
//	hr = m_pVertexIndexArray[iPolySize]->Unlock();
//	ntAssert(SUCCEEDED(hr));
//}
//
////! create declaration
//void CloudParticles2::CreateVertex()
//{
//	
//}
//
////! create vertex buffer
//void CloudParticles2::CreateDeclaration()
//{
//	D3DVERTEXELEMENT9 decl[] =
//	{
//		{ 0, 0, D3DDECLTYPE_SHORT4N, D3DDECLMETHOD_DEFAULT, D3DDECLUSAGE_POSITION, 0 },
//		{ 1, 0, D3DDECLTYPE_SHORT4N, D3DDECLMETHOD_DEFAULT, D3DDECLUSAGE_POSITION, 0 },
//		D3DDECL_END()
//	};
//		
//	m_pVertexDeclaration = CVertexDeclarationManager::Get().GetDeclaration( decl );
//}
//
////! create particle buffer
//void CloudParticles2::CreateParticle()
//{
//	
//}
//
////! draw particles
//void CloudParticles2::Draw(int iNbCell)
//{
//	
//}
//
////! constructor
//CloudParticles2(int iNbSubSubdivide, int iNbCell, int iSeed):
//	m_iNbParticlesPerCell(0),
//	m_iNbSubSubdivide(iNbSubSubdivide),
//	m_iNbCell(iNbCell),
//	m_iSeed(iSeed),
//	m_pContainer(0)
//{
//	Subdivide4Info info(Subdivide4Info::TRIANGLE,iNbSubSubdivide);
//	m_pContainer = NT_NEW IsocahedronContainer(info);
//	m_iNbParticlesPerCell = m_pContainer->m_info.m_iNbFaces;
//}
//
////! release all
//void CloudParticles2::Release()
//{
//	m_pVertexBuffer->Release();
//	m_pVertexIndexArray->Release();
//}
//
//
//
//






















void CloudParticles::Draw(int iNbCell, int iPolySize)
{
	ntAssert(HardwareCapabilities::Get().SupportsVertexShader3());
	HRESULT hr;
	
	// not to draw more particle than possible
	int iNbParticles = iNbCell * m_iNbParticlesPerCell;
	
	// sync freq for instanciation purpose
	hr = GetD3DDevice()->SetStreamSourceFreq( 0, (D3DSTREAMSOURCE_INDEXEDDATA | static_cast<u_int>(iNbParticles)) );
	ntAssert(SUCCEEDED(hr));
	hr = GetD3DDevice()->SetStreamSourceFreq( 1, (D3DSTREAMSOURCE_INSTANCEDATA | static_cast<u_int>(1)) );
	ntAssert(SUCCEEDED(hr));

	// stream 0 -> vertex, stream 1 -> particle (object)
	hr = GetD3DDevice()->SetStreamSource( 0, m_pVertexBufferArray[iPolySize], 0, sizeof(PER_VERTEX_INFO) );
	ntAssert(SUCCEEDED(hr));
	hr = GetD3DDevice()->SetStreamSource( 1, m_pParticleBuffer, 0, sizeof(PER_PARTICLE_INFO) );
	ntAssert(SUCCEEDED(hr));
	
	// indices, vertex declaration and draw
	hr = GetD3DDevice()->SetIndices( m_pVertexIndexArray[iPolySize] );
	ntAssert(SUCCEEDED(hr));

	Renderer::Get().m_Platform.SetVertexDeclaration( m_pVertexDeclaration );

	hr = GetD3DDevice()->DrawIndexedPrimitive( D3DPT_TRIANGLEFAN, 0, 0, iPolySize+1 , 0, iPolySize);
	
	// back to normal freq
	hr = GetD3DDevice()->SetStreamSourceFreq( 0, static_cast<u_int>(1) );
	ntAssert(SUCCEEDED(hr));
	hr = GetD3DDevice()->SetStreamSourceFreq( 1, static_cast<u_int>(1) );
	ntAssert(SUCCEEDED(hr));
}



//! create index
void CloudParticles::CreateIndex()
{
	for(int iPolySize = 0 ; iPolySize < m_iPolySizeMax ; iPolySize++ )
	{
		if(iPolySize<m_iPolySizeMin)
		{
			m_pVertexIndexArray[iPolySize]=0;
			continue;
		}
		
		HRESULT hr;

		hr = GetD3DDevice()->CreateIndexBuffer( (iPolySize+2) * sizeof(WORD),  0, D3DFMT_INDEX16, D3DPOOL_DEFAULT, &m_pVertexIndexArray[iPolySize], NULL );
		ntAssert(SUCCEEDED(hr));
		
		WORD* pIndices;
		hr = m_pVertexIndexArray[iPolySize]->Lock( 0, 0, (void**)&pIndices, 0 );
		ntAssert(SUCCEEDED(hr));

		for(int iFor = 0 ; iFor < iPolySize+1 ; iFor++ )
		{
			pIndices[iFor]=static_cast<WORD>(iFor);
		}
		pIndices[iPolySize+1]=static_cast<WORD>(1);
		
		hr = m_pVertexIndexArray[iPolySize]->Unlock();
		ntAssert(SUCCEEDED(hr));
	}
}

//! create declaration
void CloudParticles::CreateDeclaration()
{
	D3DVERTEXELEMENT9 decl[] =
	{
		{ 0, 0, D3DDECLTYPE_SHORT4N, D3DDECLMETHOD_DEFAULT, D3DDECLUSAGE_TEXCOORD, 0 },
		{ 1, 0, D3DDECLTYPE_SHORT4N, D3DDECLMETHOD_DEFAULT, D3DDECLUSAGE_POSITION, 0 },
		D3DDECL_END()
	};
	
	m_pVertexDeclaration = CVertexDeclarationManager::Get().GetDeclaration( decl );
}

//! create vertex buffer
void CloudParticles::CreateVertex()
{
	for(int iPolySize = 0 ; iPolySize < m_iPolySizeMax ; iPolySize++ )
	{
		if(iPolySize<m_iPolySizeMin)
		{
			m_pVertexBufferArray[iPolySize]=0;
			continue;
		}
		
		HRESULT hr;

		hr = GetD3DDevice()->CreateVertexBuffer( (iPolySize+1)*sizeof(PER_VERTEX_INFO), D3DUSAGE_WRITEONLY, 0, D3DPOOL_MANAGED, &m_pVertexBufferArray[iPolySize], NULL );
		ntAssert(SUCCEEDED(hr));

		PER_VERTEX_INFO* pVertices = 0;
		hr = m_pVertexBufferArray[iPolySize]->Lock( 0, (iPolySize+1)*sizeof(PER_VERTEX_INFO), (void**)&pVertices, 0 );
		ntAssert(SUCCEEDED(hr));

		float fMaxShort = static_cast<float>(MAXSHORT);
		float fPolysizeToShort = fMaxShort / static_cast<float>(iPolySize);
		
		pVertices[0].m_id = Array<SHORT,4>(
				static_cast<SHORT>(0),
				static_cast<SHORT>(0),
				static_cast<SHORT>(-fMaxShort * ntstd::Clamp(m_fPyramidHeight,0.0f,1.0f)),
				static_cast<SHORT>(-1));
		
		float fAngle = -2.0f*PI/iPolySize;
		for(int iFor = 1 ; iFor <= iPolySize ; iFor++ )
		{
			pVertices[iFor].m_id = Array<SHORT,4>(
				static_cast<SHORT>(fMaxShort*cos(iFor*fAngle)),
				static_cast<SHORT>(fMaxShort*sin(iFor*fAngle)),
				static_cast<SHORT>(0),
				static_cast<SHORT>(iFor * fPolysizeToShort));
		}
				
		hr = m_pVertexBufferArray[iPolySize]->Unlock();
		ntAssert(SUCCEEDED(hr));
	}
}

//! create particle buffer
void CloudParticles::CreateParticle()
{
	HRESULT hr;
	
	// create particle buffer
	hr = GetD3DDevice()->CreateVertexBuffer( m_iNbCell*m_iNbParticlesPerCell*sizeof(PER_PARTICLE_INFO), D3DUSAGE_DYNAMIC | D3DUSAGE_WRITEONLY, 0, D3DPOOL_DEFAULT, &m_pParticleBuffer, NULL );
	ntAssert(SUCCEEDED(hr));
	
	// lock
	PER_PARTICLE_INFO* pVertices = 0;
	hr = m_pParticleBuffer->Lock( 0, m_iNbCell*m_iNbParticlesPerCell*sizeof(PER_PARTICLE_INFO), (void**)&pVertices, 0 );
	ntAssert(SUCCEEDED(hr));
	
	float fMaxShort = static_cast<float>(MAXSHORT);
	//float fParticleToShort = fMaxShort / static_cast<float>(m_iNbCell*m_iNbParticlesPerCell);
	
	// create buffer
	erands(m_iSeed);
	int iCount = 0;
	
	CVector a(0.0f,0.0f,0.0f,0.0f);
	CVector b(1.0f,0.0f,0.0f,0.0f);
	CVector c(0.5f,0.8660254037844386f,0.0f,0.0f);
	
	while(iCount<m_iNbParticlesPerCell)
	{
		CVector euclidianCoord(CONSTRUCT_CLEAR);
		euclidianCoord.X() = erandf(1.0f);
		euclidianCoord.Y() = erandf(1.0f);
		
		CVector barycentricCoordinate(CONSTRUCT_CLEAR);
		barycentricCoordinate.Z() = euclidianCoord.Y() / c.Y();
		barycentricCoordinate.Y() = euclidianCoord.X() -  barycentricCoordinate.Z() * c.X();
		barycentricCoordinate.X() = 1.0f - (barycentricCoordinate.Y() + barycentricCoordinate.Z());
		
		float fMax = ntstd::Max(barycentricCoordinate.X(),barycentricCoordinate.Y());
		fMax = ntstd::Max(fMax,barycentricCoordinate.Z());
		float fMin = ntstd::Min(barycentricCoordinate.X(),barycentricCoordinate.Y());
		fMin = ntstd::Min(fMin,barycentricCoordinate.Z());
		if((fMax<=1.0f) && (fMin>=0.0f))
		{
			for(int iCell = 0 ; iCell < m_iNbCell ; iCell++ )
			{
				int iParticleId = iCell*m_iNbParticlesPerCell+iCount;
				ntAssert(iParticleId <= MAXSHORT);
				SHORT sParticleId = static_cast<SHORT>(iCell*m_iNbParticlesPerCell+iCount);
				pVertices[iParticleId].m_position[0] = static_cast<SHORT>( barycentricCoordinate.X() * fMaxShort );
				pVertices[iParticleId].m_position[1] = static_cast<SHORT>( barycentricCoordinate.Y() * fMaxShort );
				pVertices[iParticleId].m_position[2] = static_cast<SHORT>( barycentricCoordinate.Z() * fMaxShort );
				pVertices[iParticleId].m_position[3] = sParticleId;
			}
			iCount++;
		}
	}
	
	// unlock	
	hr = m_pParticleBuffer->Unlock();
	ntAssert(SUCCEEDED(hr));
}


//! release all
void CloudParticles::Release()
{
	for(int iPolySize = m_iPolySizeMin ; iPolySize < m_iPolySizeMax ; iPolySize++ )
	{
		m_pVertexBufferArray[iPolySize]->Release();
		m_pVertexIndexArray[iPolySize]->Release();
	}
	m_pParticleBuffer->Release();
}



//! constructor
CloudParticles::CloudParticles(int iNbParticlesPerCell, int iNbCell, int iSeed, float fPyramidHeight):
	m_iNbParticlesPerCell(iNbParticlesPerCell),
	m_iNbCell(iNbCell),
	m_iSeed(iSeed),
	m_fPyramidHeight(fPyramidHeight)
{
	CreateVertex();
	CreateIndex();
	CreateDeclaration();
	CreateParticle();
}








namespace 
{
	void CleanGPUNoise(CloudExportFX::CloudsGPUNoise& gpuNoise)
	{
		gpuNoise.f3Freq = CVector(CONSTRUCT_CLEAR);
		gpuNoise.f4Translate = CVector(CONSTRUCT_CLEAR);
		gpuNoise.f4Various = CVector(CONSTRUCT_CLEAR);
	}
	void CleanGPU2dNoise(CloudExportFX::CloudsGPU2dNoise& gpuNoise)
	{
		gpuNoise.f4Various = CVector(CONSTRUCT_CLEAR);
	}
} // end of namespace 


float CloudLayer::GetAverage(float fAverage,float fAmplitude)
{
	//return exp(-m_globalAmplitude.GetValue()*0.01f) - 1.0f;
	return SmoothLerp(fAverage-fAmplitude,fAverage,m_globalAmplitude.GetProgress());
}



// init new cloud ve]\rsion
void CloudLayer::SetPerLayerGPU()
{
	const CloudsLayerMutable& current = m_mutableDef.Get();
	
	CloudExportFX::CloudsLayer cloudsLayer;
	
	typedef ntstd::List<CloudsNoiseDefInterface*> NoiseList;
	NoiseList::const_iterator itNoise = m_pDef->m_obNoiseFunction.begin();
	ntAssert(itNoise != m_pDef->m_obNoiseFunction.end());
	CloudsNoise& noise = (*itNoise)->Get();
	
	cloudsLayer.f4NoisePosition = noise.m_position;
	

	GL_ARTIFICIAL_SCALE(cloudsLayer) =  current.m_fArtificialScale;
	GL_HEIGHT(cloudsLayer) = current.m_fHeight;
	GL_ALTITUDE_SKY(cloudsLayer) = current.m_fAltitudeMin;
	GL_ALTITUDE_HORIZON(cloudsLayer) = current.m_fAltitudeMax;

	GL_PARTICLESIZE(cloudsLayer)=current.m_fParticleSize;
	GL_TRANSPARENCY(cloudsLayer)=pow(current.m_fTransparency,1.0f / m_pClouds->m_pCloudArray->GetNbSlices());
	GL_PARTICLEBORDER(cloudsLayer)=current.m_fParticleBorder;
	GL_CLOUDBORDER(cloudsLayer)=current.m_fCloudBorder;

	GL_ALTITUDEINFLUENCE(cloudsLayer) = m_pDef->m_bEnableAltitude ? current.m_fAltitudeInfluence : 0.0f ;
	GL_GENERALINTENSITY(cloudsLayer) = current.m_fGeneralIntensity;
	
	cloudsLayer.f4Attenuation2 = m_pDef->m_obAttenuation;
	
	GL_Y_TEXCOORD_COEF(cloudsLayer) = m_pDef->m_fSkyHorizon;
	GL_NORMALISED_HEIGHT(cloudsLayer) = m_pDef->m_fSkyWidth;

	CDirection shadowDirection = RenderingContext::Get()->m_toKeyLight;
	CDirection eyeDirection = CDirection(m_pClouds->m_gameObserver.m_worldCamDirection);
	float fDot = shadowDirection.Dot(eyeDirection);
	float fMin = 0.5f;
	float fMax = 0.2f;
	float fDecreaseSun = (ntstd::Clamp(fDot,-fMin,fMax) + fMin) / (fMin+fMax) ;

	GL_SUNINFLUENCE(cloudsLayer) = m_pDef->m_bEnableSunAndMoon ? current.m_fSunOrMoonInfluence * fDecreaseSun : 0.0f;
	GL_DIFFUSEINFLUENCE(cloudsLayer) = m_pDef->m_bEnableDiffuse ? current.m_fDiffuseInfluence : 0.0f;
	GL_DHINFLUENCE(cloudsLayer) = m_pDef->m_bEnableDepthHaze ? current.m_fDepthHazeInfluence : 0.0f;
	
	
	cloudsLayer.f4AmbiantColor = CVector(current.m_obAmbiantColor);
	cloudsLayer.f4AmbiantColor.W() = m_pDef->m_bEnableAmbiant ? current.m_fAmbiant : 0.0f;
	
	cloudsLayer.f4SkyColor[0] = current.m_obAltitudeColorLow;
	cloudsLayer.f4SkyColor[1] = current.m_obAltitudeColorMedium;
	cloudsLayer.f4SkyColor[2] = current.m_obAltitudeColorHigh;
	
	GL_HORIZONEND(cloudsLayer) = current.m_fHorizonTreshold;
	
	int iNbGpuNoise = ntstd::Min( (int) m_pDef->m_obGPUFunction.size(),PerLayerGPUNoiseMaxSIZE);
	GL_NBGPUFUNC(cloudsLayer) = iNbGpuNoise;
	GL_NUMLAYER(cloudsLayer) = m_iNumLayer;
	GL_NBSLICES(cloudsLayer) =
		static_cast<float>(m_pClouds->m_pDef->m_iNbFrameLatency * m_pClouds->m_pDef->m_iNbLayerPerFrame);
	
	//VertexAndPixelDebugShader::SetShaderConstant(m_pClouds->m_register.m_layer,&cloudsLayer,CloudExportFX::CloudsLayer::g_iSize);
	FXMaterialInstance* pEmi = static_cast<FXMaterialInstance*>(m_pClouds->m_pSphere->GetMaterialInstance());
	const FXHandle& handle = pEmi->GetEffectMaterial()->GetEffect();
	FX_SET_VALUE_VALIDATE(handle , "g_layer", &cloudsLayer, sizeof(cloudsLayer) );
}

void CloudLayer::SetPerLayerNoiseGPU()
{	
/*	typedef ntstd::List<CloudsNoiseDefInterface*> NoiseList;
	NoiseList::const_iterator itNoise = m_pDef->m_obNoiseFunction.begin();
	ntAssert(itNoise != m_pDef->m_obNoiseFunction.end());
	CloudsNoise& noise = (*itNoise)->Get();
	float fTime = noise.m_fFinalTime;
*/

	int iNbGpuNoise = ntstd::Min( (int) m_pDef->m_obGPUFunction.size(),PerLayerGPUNoiseMaxSIZE);

	CVector freqCoef = CVector(
		 1.0f / static_cast<float>(m_pClouds->m_pDef->m_iVolumeFreqX),
		 1.0f / static_cast<float>(m_pClouds->m_pDef->m_iVolumeFreqY),
		 1.0f / static_cast<float>(m_pClouds->m_pDef->m_iVolumeFreqZ),
		 1.0f);
	 	
	ntstd::List<CloudsGPUNoise*>::const_iterator gpuNoiseEnd = m_pDef->m_obGPUFunction.end();
	ntstd::List<CloudsGPUNoise*>::const_iterator gpuNoiseIt = m_pDef->m_obGPUFunction.begin();
	for(int iGPUNoise = 0 ; iGPUNoise < PerLayerGPUNoiseMaxSIZE ; iGPUNoise++ )
	{
		CloudExportFX::CloudsGPUNoise& gpuNoise = m_gpuNoise[iGPUNoise];
		CleanGPUNoise(gpuNoise);
		if(gpuNoiseIt != gpuNoiseEnd)
		{
			if((*gpuNoiseIt)->m_bEnable)
			{
				CloudsGPUNoise& welderGPUNoise = *(*gpuNoiseIt);
				
				float fTime = welderGPUNoise.GetTime();
				gpuNoise.f3Freq = welderGPUNoise.m_obFreq * freqCoef * welderGPUNoise.m_fFreqCoef;
				//gpuNoise.f4Translate = CVector(0.0f,cos(fTime),sin(fTime),1.0f);
				gpuNoise.f4Translate = fTime * CVector(1.0f,0.0f,0.0f,1.0f);
				GL_GPUAMPLITUDE(gpuNoise) = welderGPUNoise.m_fAmplitude;
				GL_GPUAVERAGE(gpuNoise) = GetAverage(welderGPUNoise.m_fAverage,welderGPUNoise.m_fAmplitude);
				GL_GPUATTENUATION(gpuNoise) = welderGPUNoise.m_fAttenuation;
			}
			gpuNoiseIt++;
		}
	}
	
	//VertexAndPixelDebugShader::SetShaderConstant(m_pClouds->m_register.m_gpunoise,&m_gpuNoise,
	//	CloudExportFX::CloudsGPUNoise::g_iSize * iNbGpuNoise);
	FXMaterialInstance* pEmi = static_cast<FXMaterialInstance*>(m_pClouds->m_pSphere->GetMaterialInstance());
	const FXHandle& handle = pEmi->GetEffectMaterial()->GetEffect();
	CloudExportFX::CloudsGPUNoise* pFirst = &m_gpuNoise[0];
	int iSize = sizeof(CloudExportFX::CloudsGPUNoise);
	FX_SET_VALUE_VALIDATE(handle , "g_gpuNoise", pFirst, iSize * iNbGpuNoise );
}


void CloudLayer::SetPerLayerNoise2dGPU()
{	
	int iNbGpuNoise = ntstd::Min( (int) m_pDef->m_obGPUFunction.size(),PerLayerGPU2dNoiseMaxSIZE);

	CVector freqCoef = CVector(
		 1.0f / static_cast<float>(m_pClouds->m_pDef->m_iVolumeFreqX),
		 1.0f / static_cast<float>(m_pClouds->m_pDef->m_iVolumeFreqY),
		 1.0f / static_cast<float>(m_pClouds->m_pDef->m_iVolumeFreqZ),
		 1.0f);
	
	ntstd::List<Clouds2DNoiseDef*>::const_iterator gpuNoiseEnd = m_pDef->m_obGPU2DNoise.end();
	ntstd::List<Clouds2DNoiseDef*>::const_iterator gpuNoiseIt = m_pDef->m_obGPU2DNoise.begin();
	for(int iGPUNoise = 0 ; iGPUNoise < PerLayerGPU2dNoiseMaxSIZE ; iGPUNoise++ )
	{
		CloudExportFX::CloudsGPU2dNoise& gpuNoise = m_gpu2dNoise[iGPUNoise];
		CleanGPU2dNoise(gpuNoise);
		if(gpuNoiseIt != gpuNoiseEnd)
		{
			if((*gpuNoiseIt)->m_bEnable)
			{
				Clouds2DNoiseDef& welderGPUNoise = *(*gpuNoiseIt);
				
				gpuNoise.f4Various = CVector(CONSTRUCT_CLEAR);
				GL_GPU2dFREQ(gpuNoise) = welderGPUNoise.m_fFrequency;
				GL_GPU2dAMPLITUDE(gpuNoise) = welderGPUNoise.m_fAmplitude;
				GL_GPU2dAVERAGE(gpuNoise) = GetAverage(welderGPUNoise.m_fAverage,welderGPUNoise.m_fAmplitude);
				GL_GPU2dATTENUATION(gpuNoise) = welderGPUNoise.m_fAttenuation;
			}
			gpuNoiseIt++;
		}
	}
	
	//VertexAndPixelDebugShader::SetShaderConstant(m_pClouds->m_register.m_gpunoise,&m_gpuNoise,
	//	CloudExportFX::CloudsGPUNoise::g_iSize * iNbGpuNoise);
	FXMaterialInstance* pEmi = static_cast<FXMaterialInstance*>(m_pClouds->m_pSphere->GetMaterialInstance());
	const FXHandle& handle = pEmi->GetEffectMaterial()->GetEffect();
	CloudExportFX::CloudsGPU2dNoise* pFirst = &m_gpu2dNoise[0];
	int iSize = sizeof(CloudExportFX::CloudsGPUNoise);
	FX_SET_VALUE_VALIDATE(handle , "g_gpu2dNoise", pFirst, iSize * iNbGpuNoise );
}


void CloudLayer::DrawParticles()
{	
	LowResCloudBufferArray* pCa = m_pClouds->m_pCloudArray.Get();

	int iBegin = pCa->m_iNbLayerPerFrame * pCa->m_nbFrameLatency.m_value;
	int iEnd = iBegin + pCa->m_iNbLayerPerFrame;
	for(int iSubLayer = iBegin ; iSubLayer < iEnd ; iSubLayer++ )
	{
		pCa->SwitchAndSetViewport();
		
		ntAssert(!pCa->GetCloudBuffer(0).m_bNeedClean);
		float fHeight = static_cast<float>(iSubLayer) / (pCa->m_nbFrameLatency.m_max * pCa->m_iNbLayerPerFrame);
		m_pClouds->m_pSphere->SetHeight(1-fHeight);
		m_pClouds->m_pSphere->RenderMaterial();
	}
	
	
	//for(int iSubLayer = m_pDef->m_iNbSprites ; iSubLayer >= 0 ; iSubLayer-- )
	//{
	//	float fHeight = static_cast<float>(iSubLayer) / m_pDef->m_iNbSprites;
	//	m_pClouds->m_pSphere->SetHeight(fHeight);
	//	m_pClouds->m_pSphere->RenderMaterial();
	//}
	return;
}




























// init new cloud version
void Clouds::SetGlobalGPU()
{
	CloudExportFX::CloudsGlobal cloudsGlobal;
	
	CDirection shadowDirection = RenderingContext::Get()->m_toKeyLight;
	CVector sunPosition(shadowDirection.X(),shadowDirection.Y(),shadowDirection.Z(),0.0f);
	sunPosition = sunPosition * m_gameObserver.m_worldToScreen;
	sunPosition /= sunPosition.W();
	sunPosition.W() = 1.5f;
	cloudsGlobal.f4SunScreenPosition = sunPosition;
	
	Pixel2 rtSize = m_pCloudArray->m_textureSize;
	Vec2 deltaTex(0.5f/rtSize[0],0.5f/rtSize[1]);

	GET_TEXDELTA_X(cloudsGlobal) = deltaTex[0];
	GET_TEXDELTA_Y(cloudsGlobal) = deltaTex[1];
	GET_ASPECT(cloudsGlobal) = RenderingContext::Get()->m_fScreenAspectRatio;
	GET_NBFUNC(cloudsGlobal) = static_cast<float>(NoiseManagement::Get().GetCloudFunc1D().GetTextureSize()[1]);

	cloudsGlobal.f4GlobalVarious2 =  m_pDef->m_obGPUDebugValue;
	
	FXMaterialInstance* pEmi = static_cast<FXMaterialInstance*>(m_pSphere->GetMaterialInstance());
	const FXHandle& handle = pEmi->GetEffectMaterial()->GetEffect();
	FX_SET_VALUE_VALIDATE(handle , "g_global", &cloudsGlobal, sizeof(cloudsGlobal) );
}

// render the effect
void Clouds::Render()
{
	// nothing if off
	if(!IsEnable())
	{
		return;
	}

	CGatso::Start( "clouds" );

	// BETWEEN END
#ifdef _CLOUD_PROFILING
	if(!m_bRenderedOneOrMore)
	{
		m_cloudProfiling[CloudProfiling::BETWEEN].Start();
	}
	m_cloudProfiling[CloudProfiling::BETWEEN].StopAndSet();
#endif

	// MISC2
#ifdef _CLOUD_PROFILING
	m_cloudProfiling[CloudProfiling::PROFILING].Start();
	m_cloudProfiling.m_fFillRate = m_pCloudArray->GetFillRate() /
		static_cast<float>(m_pHDRTarget->GetHeight() * m_pHDRTarget->GetWidth());
	
	// draw profiling stuff
	m_cloudProfiling.Draw(m_pDef->m_fProfilingNbFrame);
	if(m_cloudProfiling.m_setArray[CloudProfiling::TOTAL].m_fValue > 1000.0f / 30.0f)
	{
		m_cloudProfiling.m_setArray[CloudProfiling::TOTAL].m_fValue=m_cloudProfiling.m_setArray[CloudProfiling::TOTAL].m_fValue;
	}

	m_cloudProfiling.InitStat();
	m_cloudProfiling.InitTimer();

	// MISC2 END
	m_cloudProfiling[CloudProfiling::PROFILING].StopAndSet();
#endif



	// TOTAL
#ifdef _CLOUD_PROFILING
	m_cloudProfiling[CloudProfiling::TOTAL].Start();
#endif

	RenderNew();
	
	// TOTAL END
#ifdef _CLOUD_PROFILING
	m_cloudProfiling[CloudProfiling::TOTAL].StopAndSet();
#endif


	// BETWEEN
#ifdef _CLOUD_PROFILING
	m_cloudProfiling[CloudProfiling::BETWEEN].Start();
#endif

	CGatso::Stop( "clouds" );
}


//
void Clouds::ReleaseIsocahedron()
{
	for(int iIso = 0 ; iIso < m_isocahedrons.GetSize() ; iIso++ )
	{
		NT_DELETE_CHUNK(Mem::MC_GFX,  m_isocahedrons[iIso] );
	}
	m_isocahedrons.Clear();
}

// create isocahedron
void Clouds::CreateIsocahedron(int iSubLevel)
{
	ntAssert(iSubLevel>=0);
	if(m_isocahedrons.GetSize()>0)
	{
		ReleaseIsocahedron();
	}
	m_isocahedrons.Reset(iSubLevel+1);
	
	m_isocahedrons[0] = NT_NEW IsocahedronContainer(Subdivide4Info::ISOCAHEDRON20);
	for(int i = 0 ; i < iSubLevel ; i++ )
	{
		m_isocahedrons[i+1] = NT_NEW IsocahedronContainer(IsocahedronContainer::SUBDIVIDE,*m_isocahedrons[i]);
	}
}

// render the effect
void Clouds::DrawDebugIsocahedronContainer(const IsocahedronContainer& iso)
{	
	float fSize =10.0f;
	CPoint dec(0.0f,10.0f,0.0f);
	
	for(int iTrig = 0 ; iTrig < iso.m_info.m_iNbFaces ; iTrig++ )
	{
		for(int iVertex = 0 ; iVertex < 3 ; iVertex++ )
		{
			Pixel3 p = iso.m_faces[iTrig].m_indices;
			g_VisualDebug->RenderLine(
				dec+CPoint(iso.m_vertices[p[0]].m_position)*fSize,
				dec+CPoint(iso.m_vertices[p[1]].m_position)*fSize,0xff00ff00);
			g_VisualDebug->RenderLine(
				dec+CPoint(iso.m_vertices[p[1]].m_position)*fSize,
				dec+CPoint(iso.m_vertices[p[2]].m_position)*fSize,0xff00ff00);
			g_VisualDebug->RenderLine(
				dec+CPoint(iso.m_vertices[p[2]].m_position)*fSize,
				dec+CPoint(iso.m_vertices[p[0]].m_position)*fSize,0xff00ff00);
		}
	}
}




/// constructor
Clouds::Clouds(const CloudsDefInterface* pDef):
	m_drawDebugVolume(64),
	m_shaderFile("./shaders/cloudseffect.fx"),
	m_shaderFile2("./shaders/cloudexport.fx"),
	m_tex2dnoiseFile("./data/cloud_2dnoise.dds","cloud_2dnoise.dds"),
	m_pDef(pDef),
	m_viCounter(0,30),
	m_bRenderedOneOrMore(false),
	m_pVolumeTex(0),
	m_pGPUnoise(0),
	m_bEnable(false),
	m_pCloudArray(0)

#ifdef _CLOUD_PROFILING
	,m_cloudProfiling(200,8)
#endif
{
	ntAssert( m_pDef );
	ntAssert(HardwareCapabilities::Exists());
	ntAssert(HardwareCapabilities::Get().SupportsPixelShader3() && HardwareCapabilities::Get().SupportsVertexShader3());
	Init();
}

// enable cloud
void Clouds::EnableClouds(bool bEnable)
{
	m_bEnable = bEnable;
}

// are clouds enable
bool Clouds::IsEnable()
{
	return m_bEnable;
}

void Clouds::LoadTexture()
{
	m_tex2dnoise = TextureManager::Get().LoadTexture_Neutral(m_tex2dnoiseFile.GetGameName().c_str());
}

void Clouds::Release()
{
	ReleaseNew();
}


void Clouds::UpdateExternalFile()
{
	if(m_shaderFile.IsNewerVersionOnDisk() || m_shaderFile2.IsNewerVersionOnDisk())
	{
		ntPrintf("*********************\nReloading shaders file: %s", m_shaderFile.GetGameName().c_str());
		ReleaseShaders();
		CreateShaders();
		ntPrintf("Done\n");
	}
	
	if(m_tex2dnoiseFile.IsNewerVersionOnDisk())
	{
		ntPrintf("*********************\nReloading texture file: %s", m_tex2dnoiseFile.GetGameName().c_str());
		TextureManager::Get().UnloadTexture_Neutral(m_tex2dnoiseFile.GetGameName().c_str());
		m_tex2dnoise = TextureManager::Get().LoadTexture_Neutral(m_tex2dnoiseFile.GetGameName().c_str());
	}
}


void Clouds::UpdateFunc1D()
{
	if(NoiseManagement::Get().CheckChanged(m_pDef->m_obShaderFunction))
	{
		NoiseManagement::Get().CreateGPUFunction(m_pDef->m_obShaderFunction,m_pDef->m_iNbSampleInTextures);
	}
}


void Clouds::CreateShaders()
{
	ntAssert(HardwareCapabilities::Get().SupportsVertexShader3() && HardwareCapabilities::Get().SupportsPixelShader3());
	while(!m_debug3DShader.Load(m_shaderFile.GetGameName().c_str(),"debug3D")) { Sleep(1000); }
}

void Clouds::ReleaseShaders()
{
	m_debug3DShader.Release();
}



//// Depth Haze data type
//struct DepthHaze {
//	float4x4 m44WorldToEye;
//	float3 f3WorldviewMatrix;
//	float3 f3AConsts;
//	float3 f3GConsts;
//	float3 f3SunDir;
//	float3 f3Beta1PlusBeta2;
//	float3 f3BetaDash1;
//	float3 f3BetaDash2;
//	float3 f3OneOverBeta1PlusBeta2;
//	float4 f3SunColour;
//};




void Clouds::SetFlags(bool beginIfTrue)
{
	if(beginIfTrue)
	{
		Renderer::Get().SetCullMode( GFX_CULLMODE_NONE );

		// z test
		if(m_pDef->m_bZTest)
		{
			GetD3DDevice()->SetRenderState( D3DRS_ZFUNC, D3DCMP_LESSEQUAL );
		}
		else
		{
			GetD3DDevice()->SetRenderState( D3DRS_ZFUNC, D3DCMP_ALWAYS );
		}
		
		// z test
		if(m_pDef->m_bZWrite)
		{
			GetD3DDevice()->SetRenderState( D3DRS_ZWRITEENABLE, TRUE );
		}
		else
		{
			GetD3DDevice()->SetRenderState( D3DRS_ZWRITEENABLE, FALSE );
		}
		
		// create an implict texture for the time being
		// DEANO, this doesnt work as its a volume texture, forgive me, ive fucked the cache, wil...
//		Texture::Ptr noise = SurfaceManager::Get().CreateTexture( Texture::CreationStruct(m_pGPUnoise->GetTextureP(), true) );
//		Renderer::Get().SetTexture( 3, noise );
		GetD3DDevice()->SetTexture( 3, m_pGPUnoise->GetTextureP() );

		Renderer::Get().SetSamplerAddressMode( 3, TEXTUREADDRESS_WRAPALL );
		if(m_pDef->m_bVolumeFiltering)
		{
			Renderer::Get().SetSamplerFilterMode( 3, TEXTUREFILTER_TRILINEAR );
		}
		else
		{
			Renderer::Get().SetSamplerFilterMode( 3, TEXTUREFILTER_POINT );
		}
		
		Texture::Ptr func = SurfaceManager::Get().CreateTexture(
								Texture::CreationStruct( NoiseManagement::Get().GetCloudFunc1D().GetTexture(), true) );
		
		Renderer::Get().SetTexture( 4, func );
		Renderer::Get().SetSamplerAddressMode( 4, TEXTUREADDRESS_CLAMPALL );
		Renderer::Get().SetSamplerFilterMode( 4, TEXTUREFILTER_BILINEAR );

	}
	else
	{
		Renderer::Get().SetCullMode( GFX_CULLMODE_NORMAL );
		Renderer::Get().SetBlendMode( GFX_BLENDMODE_NORMAL );
		Renderer::Get().SetAlphaTestMode( GFX_ALPHATEST_NONE );
		Renderer::Get().SetZBufferMode( GFX_ZMODE_NORMAL );
	}
}



void Clouds::CreateVolumeTexture()
{
	SAFEDELETE(m_pGPUnoise);
	m_pGPUnoise = NT_NEW GPUprecomputedNoise(m_pDef->GetVolumeFreq(),m_pDef->GetVolumeSize(),m_pDef->m_iVolumeSeed);
}

void Clouds::Init()
{
	CreateShaders();
	LoadTexture();
	CreateVolumeTexture();
	CreateIsocahedron(m_pDef->m_iNbSubdivisionMax);
	CreateClump();
	
	ResetRenderingTarget();
}

void Clouds::ResetRenderingTarget()
{
	m_pCloudArray.Reset(NT_NEW LowResCloudBufferArray(
		m_pDef->GetRenderingTargetSize(),
		m_pDef->m_iNbFrameLatency,
		m_pDef->m_iNbLayerPerFrame));
}

void Clouds::UpdateLayers()
{
	int iNumLayer = 0;
	ntstd::List<CloudsLayerInterface*>::const_iterator end = m_pDef->m_obLayers.end();
	for ( ntstd::List<CloudsLayerInterface*>::const_iterator it = m_pDef->m_obLayers.begin(); it != end; ++it )
	{
		bool bRes = (*it)->IsLinked();
		if(!bRes)
		{
			(*it)->Link(this,iNumLayer);
		}
		
		(*it)->GetLayer().PerFrameUpdate();
		iNumLayer++;
	}
}

// init new cloud version
void Clouds::ReleaseNew()
{
	DeleteClump();
	ReleaseIsocahedron();
	NT_DELETE_CHUNK(Mem::MC_GFX,  m_pGPUnoise );
}





// get the game observer
const Observer& Clouds::GetGameObserver() const
{
	return m_gameObserver;
}

class PointerToLayer
{
public:
	const CloudsLayerInterface* m_pLayerDef;
public:
	inline PointerToLayer(): m_pLayerDef(0) {};
	explicit inline PointerToLayer(const CloudsLayerInterface* pLayerDef): m_pLayerDef(pLayerDef) {};
	inline float GetAltitudeMin() const
	{
		return GetLayer().m_mutableDef.Get().m_fAltitudeMin;
	}
	inline bool operator < (const PointerToLayer& layerDef) const
	{
		return this->GetAltitudeMin() > layerDef.GetAltitudeMin();
	}
	inline CloudLayer& GetLayer() const
	{
		return m_pLayerDef->GetLayer();
	}
	inline const CloudsLayerInterface& GetLayerDef() const
	{
		return *m_pLayerDef;
	}
}; // end of class PointerToLayer


// init new cloud version
void Clouds::DrawParticles()
{	
	// init sorted list
	ntstd::List<PointerToLayer> sortedList;
	{
		ntstd::List<CloudsLayerInterface*>::const_iterator end = m_pDef->m_obLayers.end();
		for ( ntstd::List<CloudsLayerInterface*>::const_iterator it = m_pDef->m_obLayers.begin(); it != end; ++it )
		{
			sortedList.push_back( PointerToLayer((*it)) );
		}
	}
	sortedList.sort();
	ntAssert(sortedList.size()==1);
	CloudLayer& firstLayer = sortedList.begin()->GetLayer();
	
	if(!firstLayer.m_pDef->m_bEnable)
	{
		return ;
	}
	
	// prepare the sky
	{
		m_pSphere->SetMaterial(ntstd::String("sky2clouds"));
		SetGlobalGPU();
		firstLayer.SetPerLayerGPU();
		m_pCloudArray->SwitchAndSetViewport();
		
		m_pSphere->RenderMaterial();
	}
	
	// alpha on
	{
		if(m_pDef->m_bUseAlphaBlend)
		{
			if(m_pDef->m_bDebug1)
			{
				GetD3DDevice()->SetRenderState(D3DRS_ALPHABLENDENABLE, TRUE);
				GetD3DDevice()->SetRenderState( D3DRS_SRCBLEND, D3DBLEND_INVSRCALPHA );
				GetD3DDevice()->SetRenderState( D3DRS_DESTBLEND, D3DBLEND_SRCALPHA );
			}
			else
			{
				GetD3DDevice()->SetRenderState(D3DRS_ALPHABLENDENABLE, TRUE);
				GetD3DDevice()->SetRenderState( D3DRS_SRCBLEND, D3DBLEND_ONE );
				GetD3DDevice()->SetRenderState( D3DRS_DESTBLEND, D3DBLEND_ZERO );
			}
			
			GetD3DDevice()->SetRenderState(D3DRS_SEPARATEALPHABLENDENABLE, TRUE);
			GetD3DDevice()->SetRenderState( D3DRS_SRCBLENDALPHA, D3DBLEND_ZERO );
			GetD3DDevice()->SetRenderState( D3DRS_DESTBLENDALPHA, D3DBLEND_SRCALPHA );
		}
	}
	
	
	// render one layer (all the slices)
	{
		m_pSphere->SetMaterial(ntstd::String("clouds"));
		SetGlobalGPU();
		firstLayer.SetPerLayerGPU();
		GetD3DDevice()->SetRenderState( D3DRS_COLORWRITEENABLE, D3DCOLORWRITEENABLE_ARGB );	
		
		firstLayer.SetPerLayerNoiseGPU();
		firstLayer.DrawParticles();
	}

	m_pCloudArray->PerFrameIncrement();

	
	// final compositing
	{
		GetD3DDevice()->SetRenderState(D3DRS_SEPARATEALPHABLENDENABLE, FALSE);
		if(m_pDef->m_bDebug2)
		{
			GetD3DDevice()->SetRenderState(D3DRS_ALPHABLENDENABLE, TRUE);
			GetD3DDevice()->SetRenderState( D3DRS_SRCBLEND, D3DBLEND_INVSRCALPHA );
			GetD3DDevice()->SetRenderState( D3DRS_DESTBLEND, D3DBLEND_SRCALPHA );

			if(m_pDef->m_bUseAlphaTest)
			{
				float fV = static_cast<float>(0x000000FF) * m_pDef->m_fAlphaTestRef;
				uint32_t ulV = static_cast<uint32_t>(fV);
				Renderer::Get().SetAlphaTestMode( GFX_ALPHATEST_LESS, ulV );
			}
		}
		else
		{
			Renderer::Get().SetBlendMode( GFX_BLENDMODE_OVERWRITE );
			Renderer::Get().SetAlphaTestMode( GFX_ALPHATEST_NONE );
		}
		
		GetD3DDevice()->SetRenderState( D3DRS_COLORWRITEENABLE, D3DCOLORWRITEENABLE_RGB);
		m_pSphere->SetMaterial(ntstd::String("clouds2"));
		float fWeight = m_pCloudArray->GetProgress();
		m_pSphere->SetWeight(CVector(0.0f,fWeight,1.0f-fWeight,0.0f));
		SetGlobalGPU();
		firstLayer.SetPerLayerGPU();
		firstLayer.SetPerLayerNoise2dGPU();
		
		// Set our texture
		Renderer::Get().SetTexture( 5, m_tex2dnoise );
		Renderer::Get().SetSamplerAddressMode( 5, TEXTUREADDRESS_WRAPALL );
		Renderer::Get().SetSamplerFilterMode( 5, TEXTUREFILTER_BILINEAR );
		
		// rendering target
		for(int iOld = 0 ; iOld < LowResCloudBufferArray::sm_iMotionBlur ; iOld++ )
		{
			Renderer::Get().SetTexture(iOld,m_pCloudArray->GetCloudBuffer(iOld).GetDest()->GetTexture() );
			Renderer::Get().SetSamplerAddressMode( iOld, TEXTUREADDRESS_CLAMPALL );
			if(m_pDef->m_bDebug3)
			{
				Renderer::Get().SetSamplerFilterMode( iOld, TEXTUREFILTER_POINT );
			}
			else
			{
				Renderer::Get().SetSamplerFilterMode( iOld, TEXTUREFILTER_BILINEAR );
			}
		}
		
		ZBuffer::Ptr zbuffer = Renderer::Get().m_targetCache.GetDepthTarget();
		ntAssert_p( zbuffer, ("Must have a valid ZBuffer here") );
		
		Renderer::Get().m_targetCache.SetColourAndDepthTargets( m_pHDRTarget, zbuffer );

		m_pSphere->RenderMaterial();
	}
}





// render the effect
void Clouds::GenerateCell()
{
	ntstd::List<CloudsLayerInterface*>::const_iterator end = m_pDef->m_obLayers.end();
	for ( ntstd::List<CloudsLayerInterface*>::const_iterator it = m_pDef->m_obLayers.begin(); it != end; ++it )
	{
		CloudLayer& layer = (*it)->GetLayer();
		layer.GenerateCell();
	}
}


// render the effect
void Clouds::ComputeCPUNoise()
{
	ntstd::List<CloudsLayerInterface*>::const_iterator end = m_pDef->m_obLayers.end();
	for ( ntstd::List<CloudsLayerInterface*>::const_iterator it = m_pDef->m_obLayers.begin(); it != end; ++it )
	{
		CloudLayer& layer = (*it)->GetLayer();
		layer.ComputeCPUNoise();
	}
}




/*
//--------------------------------------------------
//!
//!	Allocate textures required by shadow map
//!
//--------------------------------------------------
bool Clouds::AllocateTextures()
{
	if( m_bAllocated == false )
	{
		Pixel2 size(m_pDef->m_iGridSizeX,m_pDef->m_iGridSizeY);
		m_pRenderTarget = CRenderTargetCache::Get().AllocateRenderTarget( size[0], size[1], D3DFMT_A8R8G8B8  );

		m_bAllocated = true;
		return true;
	}
	else
	{
		return false;
	}
}

//--------------------------------------------------
//!
//!	Deallocate textures required by shadow map
//!
//--------------------------------------------------
void Clouds::DeallocateTextures()
{
	if( m_bAllocated == true )
	{
		// (in this case, shadow map data is in the rendertarget)
		CRenderTargetCache::Get().DeallocateRenderTarget( m_pRenderTarget );

		m_bAllocated = false;
	}
}
*/




// render the effect
void Clouds::ComputeBillboardTransform()
{
	ntstd::List<CloudsLayerInterface*>::const_iterator end = m_pDef->m_obLayers.end();
	for ( ntstd::List<CloudsLayerInterface*>::const_iterator it = m_pDef->m_obLayers.begin(); it != end; ++it )
	{
		CloudLayer& layer = (*it)->GetLayer();
		layer.ComputeBillboardTransform();
	}
}

// render the effect
void Clouds::ComputeDensityCulling()
{
	ntstd::List<CloudsLayerInterface*>::const_iterator end = m_pDef->m_obLayers.end();
	for ( ntstd::List<CloudsLayerInterface*>::const_iterator it = m_pDef->m_obLayers.begin(); it != end; ++it )
	{
		CloudLayer& layer = (*it)->GetLayer();
		layer.ComputeDensityCulling();
	}
}

void Clouds::DeleteClump()
{
	NT_DELETE_CHUNK(Mem::MC_GFX,  m_pSphere );
	CClumpLoader::Get().UnloadClump( m_pClump );
}

void Clouds::CreateClump()
{
	m_pClump = CClumpLoader::Get().LoadClump_Neutral( "data/clouds/iso_4it.clump", true );
	//m_pClump = CClumpLoader::Get().LoadClump( "data/clouds/clouds.clump", true );
	const CMeshHeader* pMeshHeader = &m_pClump->m_pobMeshHeaderArray[0];
	const Transform* pTransform = CHierarchy::GetWorld()->GetRootTransform();
	m_pSphere = NT_NEW CCloudsInstance(pTransform, pMeshHeader);
}




void Clouds::DebugKey()
{
	if(CInputHardware::Get().GetKeyboard().IsKeyPressed( KEYC_P, KEYM_CTRL ))
	{
		ntstd::List<CloudsLayerInterface*>::const_iterator end = m_pDef->m_obLayers.end();
		for ( ntstd::List<CloudsLayerInterface*>::const_iterator it = m_pDef->m_obLayers.begin(); it != end; ++it )
		{
			CloudLayer& layer = (*it)->GetLayer();
			if(layer.m_globalAmplitude.IsRunning())
			{
				layer.m_globalAmplitude.Pause();
			}
			else
			{
				layer.m_globalAmplitude.Play();
			}
		}
	}
	
	if(CInputHardware::Get().GetKeyboard().IsKeyPressed( KEYC_C, KEYM_ALT ))
	{
		m_bEnable = !m_bEnable;
	}
}


struct global
{
	CMatrix m_worldViewProj;
	CVector m_worldEyePosition;
	CVector m_light;
	CVector m_dummy;
};


void Clouds::DrawNormal(const CPoint& translate)
{
	GradData grid(m_pDef->GetVolumeFreq() + Pixel4(1,1,1,1),m_pDef->m_iVolumeSeed);
	grid.SetTilable();
	
	Pixel4 coord(0);
	for(coord[0] = 0 ; coord[0] < grid.m_size[0] ; coord[0]++ )
	{
		for(coord[1] = 0 ; coord[1] < grid.m_size[1] ; coord[1]++ )
		{
			for(coord[2] = 0 ; coord[2] < grid.m_size[2] ; coord[2]++ )
			{
				CPoint p(static_cast<float>(coord[0]),
					static_cast<float>(coord[1]),
					static_cast<float>(coord[2]));
				g_VisualDebug->RenderLine(
					translate+p,
					translate+p+CPoint(grid[coord]),
					0xff00ffff);
			}
		}
	}
}

void Clouds::DrawDebugVolume()
{
	Renderer::Get().SetCullMode( GFX_CULLMODE_NONE );
	GetD3DDevice()->SetRenderState( D3DRS_ALPHATESTENABLE, TRUE );
	
	m_debug3DShader.Set();
	global g;
	float fTime = m_gameObserver.GetCloudTime();
	
	//g.m_worldViewProj = RenderingContext::Get()->m_worldToScreen;
	g.m_worldViewProj = m_gameObserver.m_worldToScreen;
	g.m_worldEyePosition = m_gameObserver.m_worldPosition;
	g.m_light = CVector(cos(fTime),0.0f,sin(fTime),0.0f);
	g.m_dummy = CVector(CONSTRUCT_CLEAR);
	g.m_dummy.X() = fTime;
	VertexAndPixelDebugShader::SetShaderConstant(1,&g,sizeof(global));
	
	CloudExportFX::CloudsDebugData debugData;
	debugData.f4VolumeSize = CVector(
		static_cast<float>(m_pDef->m_iVolumeFreqX),
		static_cast<float>(m_pDef->m_iVolumeFreqY),
		static_cast<float>(m_pDef->m_iVolumeFreqZ),
		static_cast<float>(m_pDef->m_iVolumeFreqW));
		
	debugData.f4VolumeSubSize = CVector(
		static_cast<float>(m_pDef->m_iVolumeSizeX),
		static_cast<float>(m_pDef->m_iVolumeSizeY),
		static_cast<float>(m_pDef->m_iVolumeSizeZ),
		static_cast<float>(m_pDef->m_iVolumeSizeW));

	debugData.f4VolumeSubSize = CVector();
	VertexAndPixelDebugShader::SetShaderConstant(20,&debugData,sizeof(debugData));
	
	Pixel3 freq = Pixel3::SafeCopy(m_pGPUnoise->m_freq);
	FrankMisc::DrawGrid(freq,CPoint(0,1,0),CDirection(debugData.f4VolumeSize), DC_RED );
	m_drawDebugVolume.Draw();	
	
	Renderer::Get().SetCullMode( GFX_CULLMODE_NORMAL );
	Renderer::Get().SetAlphaTestMode( GFX_ALPHATEST_NONE );
	
	DrawNormal(CPoint(0,1,0));
}
	



// render the effect
void Clouds::RenderNew()
{	
	// to perform only a part of the computation
	int iLevelOfCompletion = 0;
	
	// profiling stuff
	if(m_pDef->m_iComplete < ++iLevelOfCompletion) return;
	
	
	// UPDATEFILE
#ifdef _CLOUD_PROFILING
	m_cloudProfiling[CloudProfiling::UPDATEFILE].Start();
#endif
	if(m_pDef->m_bCheckShader)
	{
		m_viCounter[0]++;
		if(m_viCounter[0]==m_viCounter[1])
		{
			UpdateExternalFile();
			m_viCounter[0]=0;
		}
 	}
 	
	// profiling stuff
	if(m_pDef->m_iComplete < ++iLevelOfCompletion) return;
	
	// set observer parameters;
	m_debugObserver.PerFrameUpdate();
	if(!m_pDef->m_bFreezeGameCamera || !m_bRenderedOneOrMore)
	{
		m_gameObserver.PerFrameUpdate();
	}

	// profiling stuff
	if(m_pDef->m_iComplete < ++iLevelOfCompletion) return;
	
	UpdateLayers();
	UpdateFunc1D();
	
	// END UPDATEFILE
#ifdef _CLOUD_PROFILING
	m_cloudProfiling[CloudProfiling::UPDATEFILE].StopAndSet();
#endif

	// profiling stuff
	if(m_pDef->m_iComplete < ++iLevelOfCompletion) return;

	// VISIBILITY
#ifdef _CLOUD_PROFILING
	m_cloudProfiling[CloudProfiling::VISIBILITY].Start();
#endif
	GenerateCell();
	// END VISIBILITY
#ifdef _CLOUD_PROFILING
	m_cloudProfiling[CloudProfiling::VISIBILITY].StopAndSet();
#endif

	// profiling stuff
	if(m_pDef->m_iComplete < ++iLevelOfCompletion) return;


	// NOISE
#ifdef _CLOUD_PROFILING
	m_cloudProfiling[CloudProfiling::NOISE].Start();
#endif

	ComputeCPUNoise();
	ComputeDensityCulling();

	// profiling stuff
	if(m_pDef->m_iComplete < ++iLevelOfCompletion) return;

	ComputeBillboardTransform();

	// END NOISE
#ifdef _CLOUD_PROFILING
	m_cloudProfiling[CloudProfiling::NOISE].StopAndSet();
#endif

	// profiling stuff
	if(m_pDef->m_iComplete < ++iLevelOfCompletion) return;
	
	
	// DRAW
#ifdef _CLOUD_PROFILING
	m_cloudProfiling[CloudProfiling::DRAW].Start();
#endif
	
	SetFlags(true);
	
	DrawParticles();

	if(m_pDef->m_bDrawDebugVolume)
	{
		DrawDebugVolume();
	}

	SetFlags(false);
	
	// DRAW END
#ifdef _CLOUD_PROFILING
	m_cloudProfiling[CloudProfiling::DRAW].StopAndSet();
#endif	

	// DEBUG
#ifdef _CLOUD_PROFILING
	m_cloudProfiling[CloudProfiling::DEBUG].Start();
#endif

	// DEBUG END
#ifdef _CLOUD_PROFILING
	m_cloudProfiling[CloudProfiling::DEBUG].StopAndSet();
#endif	
	
	m_bRenderedOneOrMore=true;
}


void Clouds::SetSkyRenderingTarget()
{
	m_pCloudArray->SetSkyViewport();
}
	





























OneLowResCloudBuffer::OneLowResCloudBuffer()
	:m_size(0,0)
	,m_bAllocated(false)
	,iSwitch(0)
{
	// nothing
}

OneLowResCloudBuffer::OneLowResCloudBuffer(Pixel2 size)
	:m_size(0,0)
{
	AllocateTextures(size);
}

OneLowResCloudBuffer::~OneLowResCloudBuffer()
{
	if(m_bAllocated)
	{
		DeallocateTextures();
	}
}
//--------------------------------------------------
//!
//!	Allocate textures required by shadow map
//!
//--------------------------------------------------
void OneLowResCloudBuffer::AllocateTextures(Pixel2 size)
{
	ntAssert(!m_bAllocated);
	m_size = size;
	m_renderTargets[0] = SurfaceManager::Get().CreateRenderTarget(
							RenderTarget::CreationStruct( m_size[0], m_size[1], D3DFMT_A16B16G16R16F, false ) );

	m_renderTargets[1] = SurfaceManager::Get().CreateRenderTarget(
							RenderTarget::CreationStruct( m_size[0], m_size[1], D3DFMT_A16B16G16R16F, false ) );

//	m_renderTargets[0] = SurfaceManager::Get().CreateRenderTarget(
//							RenderTarget::CreationStruct( m_size[0], m_size[1], D3DFMT_A8R8G8B8, false ) );
//
//	m_renderTargets[1] = SurfaceManager::Get().CreateRenderTarget(
//							RenderTarget::CreationStruct( m_size[0], m_size[1], D3DFMT_A8R8G8B8, false ) );

	m_bAllocated = true;
}

//--------------------------------------------------
//!
//!	Deallocate textures required by shadow map
//!
//--------------------------------------------------
void OneLowResCloudBuffer::DeallocateTextures()
{
	ntAssert(m_bAllocated);
//	SurfaceManager::Get().ReleaseRenderTarget( m_pRenderTarget );
	SurfaceManager::Get().ReleaseRenderTarget( m_renderTargets[0] );
	SurfaceManager::Get().ReleaseRenderTarget( m_renderTargets[1] );
	m_bAllocated = false;
}

RenderTarget::Ptr OneLowResCloudBuffer::GetSrc()
{
	int iIndex = (iSwitch+1)&1;
	return m_renderTargets[iIndex];
}

RenderTarget::Ptr OneLowResCloudBuffer::GetDest()
{
	int iIndex = iSwitch;
	return m_renderTargets[iIndex];
}

void OneLowResCloudBuffer::Switch()
{
	iSwitch = (iSwitch+1)&1;
}













LowResCloudBufferArray::~LowResCloudBufferArray()
{
	// nothing
}

LowResCloudBufferArray::LowResCloudBufferArray(Pixel2 textureSize, int iNbFrameLatency, int iNbLayerPerFrame)
	:m_textureSize(0,0)
	,m_iNbLayerPerFrame(0)
	,m_nbFrameLatency(0,0)
{
	Allocate(textureSize);
	SetNbLayer(iNbFrameLatency,iNbLayerPerFrame);
}

void LowResCloudBufferArray::SetNbLayer(int iNbFrameLatency, int iNbLayerPerFrame)
{
	m_iNbLayerPerFrame = iNbLayerPerFrame;
	m_nbFrameLatency = ValueAndMax<int>(iNbFrameLatency,0);
}

void LowResCloudBufferArray::Allocate(Pixel2 textureSize)
{
	m_textureSize = textureSize;
	for(int iRotIndex = 0 ; iRotIndex < sm_iMotionBlur ; iRotIndex++ )
	{
		m_array[m_index[iRotIndex]].AllocateTextures(m_textureSize);
	}
}

// this increment is done after the offscreenn low res draw and before the final draw
bool LowResCloudBufferArray::PerFrameIncrement()
{
	m_nbFrameLatency.m_value++;
	if(m_nbFrameLatency.m_value == m_nbFrameLatency.m_max)
	{
		m_nbFrameLatency.m_value = 0;
		m_index++;
		OneLowResCloudBuffer& current = GetCloudBuffer(0);
		current.m_bNeedClean = true;
		return true;
	}
	else
	{
		return false;
	}
}

OneLowResCloudBuffer& LowResCloudBufferArray::GetCloudBuffer(int iOld)
{
	int iRotIndex = sm_iMotionBlur-1-iOld;
	return m_array[m_index[iRotIndex]];
}

void LowResCloudBufferArray::SwitchAndSetViewport()
{
	OneLowResCloudBuffer& current = GetCloudBuffer(0);
	//current.Switch();
	ZBuffer::Ptr zbuffer = Renderer::Get().m_targetCache.GetDepthTarget();
	ntAssert_p( zbuffer, ("Must have a valid ZBuffer here") );
	
	Renderer::Get().m_targetCache.SetColourAndDepthTargets( current.GetDest(), zbuffer );

	if(current.m_bNeedClean)
	{
		Renderer::Get().Clear( D3DCLEAR_TARGET, 0xffff0000, 0.0f, 0 );
		current.m_bNeedClean = false;
	}
	
	/*
	Renderer::Get().SetTexture( 0, current.GetSrc()->GetTexture() );
	Renderer::Get().SetSamplerAddressMode( 0, TEXTUREADDRESS_CLAMPALL );
	Renderer::Get().SetSamplerFilterMode( 0, TEXTUREFILTER_POINT );
	*/
}

// this value is never 0, because the first texture (sampler 1) allways have an influence
float LowResCloudBufferArray::GetProgress()
{
	return static_cast<float>(m_nbFrameLatency.m_value+1)/m_nbFrameLatency.m_max;
}


void LowResCloudBufferArray::SetSkyViewport()
{
	OneLowResCloudBuffer& current = GetCloudBuffer(0);

	ZBuffer::Ptr zbuffer = Renderer::Get().m_targetCache.GetDepthTarget();
	ntAssert_p( zbuffer, ("Must have a valid ZBuffer here") );
	
	// dest because its switched after before the first use
	Renderer::Get().m_targetCache.SetColourAndDepthTargets( current.GetDest(), zbuffer );
}


int LowResCloudBufferArray::GetFillRate()
{
	return m_textureSize[0] * m_textureSize[1] * m_iNbLayerPerFrame;
}

int LowResCloudBufferArray::GetNbSlices()
{
	return m_nbFrameLatency.m_max * m_iNbLayerPerFrame;
}
