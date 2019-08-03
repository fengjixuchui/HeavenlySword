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

#include "speedTreeCrap.h"
#include "SpeedTreeRandom.h"
#include "SpeedTreeVector.h"
#include "SpeedTreeForest.h"
#include "SpeedTreeConfig.h"
#include "SpeedTreeWrapper.h"
#include "core/explicittemplate.h"
#include "core/exportstruct_clump.h"
#include "anim/hierarchy.h"
#include "gfx/fxmaterial.h"
#include "gfx/texturemanager.h"
#include "SpeedTreeRenderable.h"
#include "speedtree/SpeedTreeManager.h"

using namespace	std;


///////////////////////////////////////////////////////////////////////	 
//	CSpeedTreeForest constructor

CSpeedTreeForest::CSpeedTreeForest(	)
	:m_fWindStrength(0.2f)
	,m_ulRenderBitVector(Forest_RenderAll)
{
	//m_afForestExtents[0] = m_afForestExtents[1]	= m_afForestExtents[2] = FLT_MAX;
	//m_afForestExtents[3] = m_afForestExtents[4]	= m_afForestExtents[5] = -FLT_MAX;
}


///////////////////////////////////////////////////////////////////////	 
//	CSpeedTreeForest destructor

CSpeedTreeForest::~CSpeedTreeForest( )
{
	for	(unsigned int i	= 0; i < m_treesInForest.size( ); ++i)
	{
		NT_DELETE( m_treesInForest[i] );
	}
	//for	(unsigned int i	= 0; i < m_referenceTrees.size(	); ++i)
	//{
	//	delete m_referenceTrees[i];
	//}

	m_pHierarchy->GetRootTransform()->RemoveFromParent();
	CHierarchy::Destroy(m_pHierarchy);
}

//extern void SetToPlatformResources();
//extern void SetToNeutralResources();

///////////////////////////////////////////////////////////////////////	 
//	CSpeedTreeForest::Load
//
//	A (null) pFilename value forces	the	load routine to	generate the
//	trees procedurally instead of loading from an STF file

bool CSpeedTreeForest::Load(const char*	pFilename)
{
	bool bSuccess =	false;
	
	// setup a few parameters shared by	all	tree models
	CSpeedTreeRT::SetNumWindMatrices(c_nNumWindMatrices);
	
	bool bLoaded = bLoaded = LoadFromStfFile(pFilename);
	ntAssert_p(bLoaded,	("Cannot load speedtree	file: %s",pFilename));


	if (bLoaded)
	{
		// setup speedwind
		m_cSpeedWind.Load(c_pSpeedWindFilename);
		CSpeedWind::SWindAttributes	sWind =	m_cSpeedWind.GetAttributes(	);
		sWind.m_uiNumMatrices =	c_nNumWindMatrices;
		m_cSpeedWind.SetAttributes(sWind);
		SetWindStrength(m_fWindStrength);

		// set LOD limits
		SetLodLimits( );

		bSuccess = FinaliseLoad();

		// once	the	trees are loaded, the geometry buffered, and instances
		// created,	we can delete some of the memory in	the	RT class
		for	(unsigned int i	= 0; i < m_treesInForest.size(	); ++i)
		{
			 m_treesInForest[i]->GetSpeedTree()->DeleteTransientData();
		}
	}
	ntAssert_p(bLoaded,	("Cannot finaliser load	of speedtree file: %s",pFilename));
	

	return bSuccess;
}



void CSpeedTreeForest::LoadWindMatrix(FXMaterial* pMaterial)
{
	for(u32 uiLocation = 0 ; uiLocation < c_nNumWindMatrices ; ++uiLocation )
	{
		VERIFY(pMaterial->GetEffect()->SetMatrix(pMaterial->GetEffect()->GetParameterElement("g_amWindMatrices", uiLocation),
			reinterpret_cast<const D3DXMATRIX*>(GetWind()->GetWindMatrix(uiLocation))));
	}
}

void CSpeedTreeForest::LoadLeafMatrix(FXMaterial* pMaterial)
{
	for(u32 uiLocation = 0 ; uiLocation < GetWind()->GetNumLeafAngles( ) ; ++uiLocation )
	{
		VERIFY(pMaterial->GetEffect()->SetMatrix(pMaterial->GetEffect()->GetParameterElement("g_amLeafAngleMatrices", uiLocation),
			reinterpret_cast<const D3DXMATRIX*>(GetWind()->GetLeafAngleMatrix(uiLocation))));
	}
}

				   


///////////////////////////////////////////////////////////////////////	 
//	CSpeedTreeForest::InitGraphics

bool CSpeedTreeForest::FinaliseLoad(void)
{
	m_texCompositeMap =	TextureManager::Get().LoadTexture_Neutral( c_pCompositeMapFilename );
	return true;
}



///////////////////////////////////////////////////////////////////////	 
//	CSpeedTreeForest::AdjustExtents

//void CSpeedTreeForest::AdjustExtents(float x, float	y, float z)
//{
//	if (m_treesInForest.size() == 1)
//	{
//		memcpy(m_afForestExtents, m_treesInForest[0]->GetBoundingBox( ), 6 * sizeof(float));
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

bool CSpeedTreeForest::LoadFromStfFile(const char* pFilename)
{
	bool bSuccess =	false;

	// open	the	file
	int	nAttemptedReads	= 0, nSuccessfulReads =	0;			  
	FILE* pFile	= fopen(pFilename, "r");
	if (pFile)
	{
		// find	the	end	of the file
		fseek(pFile, 0L, SEEK_END);
		int	nEnd = ftell(pFile);
		fseek(pFile, 0L, SEEK_SET);

		int	nTree =	0;
		while (ftell(pFile)	!= nEnd)
		{
			// keep	track of the tree for error	reporting
			++nTree;

			// read	the	mandatory data
			char szName[1024];
			unsigned int nSeed = 0,	nNumInstances =	0;
			float fSize	= 0.0f,	fSizeVariance =	0.0f;
			float afPos[3] = { 0.0f	};
			nAttemptedReads++;
			if (fscanf(pFile, "%s %d %g	%g %g %g %g	%d\n", szName, &nSeed, &fSize, &fSizeVariance, afPos, afPos	+ 1, afPos + 2,	&nNumInstances)	== 8)
			{
				nSuccessfulReads++;
				nAttemptedReads	+= nNumInstances;
				// make	a new tree structure
				CSpeedTreeWrapper* pTree = NT_NEW CSpeedTreeWrapper( new CSpeedTreeRT(), this);

				// load	the	tree with its position
				string strTreeFilename = std::string(Util::DirName(pFilename)) + std::string("/") +	szName;
				if (pTree->LoadTree(strTreeFilename.c_str( ), nSeed, fSize,	fSizeVariance))
				{
                    // set the tree's location
					pTree->GetSpeedTree()->SetTreePosition(afPos[0],afPos[1],afPos[2]);
					m_treesInForest.push_back(pTree);

					// instances
					for	(unsigned int i	= 0; i < nNumInstances;	++i)
					{
						// read	the	instance location
						if (fscanf(pFile, "%g %g %g\n",	afPos, afPos + 1, afPos	+ 2) ==	3)
						{
							CSpeedTreeWrapper* pInstance = pTree->MakeInstance();
                            if (pInstance)
                            {
			                    pInstance->GetSpeedTree()->SetTreePosition(afPos[0],afPos[1],afPos[2]);
                                // add instance to the scene and associate it with appropriate poperty nodes
                                m_treesInForest.push_back(pInstance);
                                ++nSuccessfulReads;
                            }
                            else
                            {
                                fprintf(stderr, "SpeedTree instancing function failed\n", nTree, pFilename);
                            }
						}
						else
						{
							ntPrintf( "Error reading STF data [tree	%d in %s]\n", nTree, pFilename);
						}
					}
				}
			}
			else
			{
				ntPrintf( "Error reading STF data [tree	%d in %s]\n", nTree, pFilename);
			}
		}
		fclose(pFile);
	}
	else
	{
		ntError_p(false, ( "Error opening '%s'\n", pFilename) );
	}
/*
	ntstd::Vector< GpJointLinkage >	joint_linkage	( m_treesInForest.size() + 1 );
	joint_linkage[ 0 ].m_flags				= 0;
	joint_linkage[ 0 ].m_parentIndex		= -1;
	joint_linkage[ 0 ].m_nextSiblingIndex	= -1;
	joint_linkage[ 0 ].m_firstChildIndex	= 1;
	
	for(u32	iIndex = 1 ; iIndex	< joint_linkage.size() ; ++iIndex )
	{
		joint_linkage[ iIndex ].m_flags				= 0;
		joint_linkage[ iIndex ].m_parentIndex		= 0;
		joint_linkage[ iIndex ].m_nextSiblingIndex	= iIndex<(joint_linkage.size()-1)?iIndex+1:-1;
		joint_linkage[ iIndex ].m_firstChildIndex	= -1;
	}
	
	m_pHierarchy = CHierarchy::Create( &(joint_linkage.front()), joint_linkage.size() );
	CHierarchy::GetWorld()->GetRootTransform()->AddChild(m_pHierarchy->GetRootTransform());
*/

	// create tree
	for(u32	iTree =	0 ;	iTree <	m_treesInForest.size() ; ++iTree )
	{
		m_treesInForest[iTree]->SetRenderable(m_pHierarchy->GetTransform(iTree+1));
	}

	if (nAttemptedReads	== nSuccessfulReads	&& nAttemptedReads > 0)
	{
		bSuccess = true;
	}

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
	m_cSpeedWind.Advance(fTimeInSecs, m_fWindStrength, c_afWindDirection[0], c_afWindDirection[1], c_afWindDirection[2]);
}


///////////////////////////////////////////////////////////////////////	 
//	CSpeedTreeForest::SetLodLimits

void CSpeedTreeForest::SetLodLimits(void)
{
	// find	tallest	tree
	float fTallest = -1.0f;
	for	(u32 i	= 0; i < m_treesInForest.size( ); ++i)
	{
		float fHeight =	m_treesInForest[i]->GetBoundingBox(	)[5] - m_treesInForest[i]->GetBoundingBox( )[0];
		fTallest = __max(fHeight, fTallest);
	}

	// assign all trees	based on tallest
	for	(u32 i = 0;	i <	m_treesInForest.size( ); ++i)
	{
		m_treesInForest[i]->GetSpeedTree( )->SetLodLimits(fTallest	* c_fNearLodFactor,	fTallest * c_fFarLodFactor);
	}
}

/*
///////////////////////////////////////////////////////////////////////	 
//	CSpeedTreeForest::Render

void CSpeedTreeForest::Render(unsigned long	ulRenderBitVector)
{
	UNUSED(ulRenderBitVector);
	for	(u32 i	= 0; i < m_treesInForest.size( ); ++i)
	{
		//m_treesInForest[i]->m_renderables[TreeInForest::LEAVES]->RenderMaterial();
		//m_treesInForest[i]->m_renderables[TreeInForest::FROND]->RenderMaterial();
		//m_treesInForest[i]->m_renderables[TreeInForest::BRANCH]->RenderMaterial();
		//m_treesInForest[i]->m_renderables[TreeInForest::BILLBOARD]->RenderMaterial();
	}
}


void CSpeedTreeForest::RenderDepth(unsigned	long ulRenderBitVector)
{
	UNUSED(ulRenderBitVector);
	for	(u32 i	= 0; i < m_treesInForest.size( ); ++i)
	{
		//m_treesInForest[i]->m_renderables[TreeInForest::LEAVES]->RenderDepth();
		//m_treesInForest[i]->m_renderables[TreeInForest::FROND]->RenderDepth();
		//m_treesInForest[i]->m_renderables[TreeInForest::BRANCH]->RenderDepth();
		//m_treesInForest[i]->m_renderables[TreeInForest::BILLBOARD]->RenderDepth();
	}
}

void CSpeedTreeForest::RenderRecievers(unsigned	long ulRenderBitVector)
{
	UNUSED(ulRenderBitVector);
	for	(u32 i	= 0; i < m_treesInForest.size( ); ++i)
	{
		//m_treesInForest[i]->m_renderables[TreeInForest::LEAVES]->RenderShadowOnly();
		//m_treesInForest[i]->m_renderables[TreeInForest::FROND]->RenderShadowOnly();
		//m_treesInForest[i]->m_renderables[TreeInForest::BRANCH]->RenderShadowOnly();
		//m_treesInForest[i]->m_renderables[TreeInForest::BILLBOARD]->RenderShadowOnly();
	}
}


void CSpeedTreeForest::RenderShadowMap(unsigned	long ulRenderBitVector)
{
	UNUSED(ulRenderBitVector);
	for	(u32 i	= 0; i < m_treesInForest.size( ); ++i)
	{
		//m_treesInForest[i]->m_renderables[TreeInForest::LEAVES]->RenderShadowMap();
		//m_treesInForest[i]->m_renderables[TreeInForest::FROND]->RenderShadowMap();
		//m_treesInForest[i]->m_renderables[TreeInForest::BRANCH]->RenderShadowMap();
		//m_treesInForest[i]->m_renderables[TreeInForest::BILLBOARD]->RenderShadowMap();
	}
}
*/

void CSpeedTreeForest::UpdateWind()
{
	AdvanceWind(SpeedTreeManager::Get().m_fAccumTime);

	// advance all trees' LOD and wind
	for	(unsigned int i	= 0; i < m_treesInForest.size(	); ++i)
	{
		m_treesInForest[i]->Advance( );
	}
}

void CSpeedTreeForest::UpdateForLight()
{
	CDirection keyDirection = -RenderingContext::Get()->m_keyDirection;
	for(int i = 0 ; i < 3 ; ++i ) m_afCameraDir[i] = keyDirection[i];
	for(int i = 0 ; i < 3 ; ++i ) m_afCameraPos[i] = -1000*keyDirection[i];

	// set camera
	SetCamera(false);
	this->BuildLeafAngleMatrices(m_afCameraDir);
}



void CSpeedTreeForest::UpdateForCamera()
{
	const CCamera* pCamera = CamMan_Public::GetP();
	CMatrix invWorldToEye = pCamera->GetViewTransform()->GetWorldMatrix();
	CVector worldCamDirection = CVector(invWorldToEye[2]);
	CPoint worldCamPosition = invWorldToEye.GetTranslation();
	
	for(int i = 0 ; i < 3 ; ++i ) m_afCameraDir[i] = worldCamDirection[i];
	for(int i = 0 ; i < 3 ; ++i ) m_afCameraPos[i] = worldCamPosition[i];

	// set camera
	SetCamera(false);
	this->BuildLeafAngleMatrices(m_afCameraDir);
}



void CSpeedTreeForest::SetCamera(bool bBillboard)
{
	if(bBillboard)
	{
		CSpeedTreeRT::SetCamera(m_afCameraPos, m_afCameraDir);
	}
	else
	{
		float afBaseDir[3] = { 1.0f, 0.0f, 0.0f };
		CSpeedTreeRT::SetCamera(m_afCameraPos, afBaseDir);
	}
}



SpeedTreeStat CSpeedTreeForest::GetStat()
{
	SpeedTreeStat res;
	for(std::vector<CSpeedTreeWrapper*>::iterator it	= m_treesInForest.begin();
		it != m_treesInForest.end();
		++it)
	{
		res+=(*it)->GetStat();
	}

	return res;
}

