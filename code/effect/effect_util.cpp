//--------------------------------------------------
//!
//!	\file effect_util.cpp
//!	handy stuff for effects
//!
//--------------------------------------------------

#include "effect_util.h"
#include "camera/camutils.h"
#include "effect_error.h"
#include "gfx/renderer.h"
#include "game/entitymanager.h"
#include "anim/hierarchy.h"
#include "effect/effect_manager.h"
#include "editable/anystring.h"

#include "core/visualdebugger.h"
#include "core/timer.h"
#include "gfx/rendercontext.h"
#include "gfx/depthhazeconsts.h"
#include "gfx/fxmaterial.h"
#include "objectdatabase/dataobject.h"

#ifdef PLATFORM_PC
#include "gfx/dxerror_pc.h"
#endif

#ifndef _RELEASE
static char aErrors[MAX_PATH];
#endif

START_STD_INTERFACE( PlaneDef )
	I2POINT		( m_origin,		PlaneOrigin )
	I2VECTOR	( m_ypr,		PlaneOrientation(deg) )
	DECLARE_POSTCONSTRUCT_CALLBACK( PostConstruct )
	DECLARE_EDITORCHANGEVALUE_CALLBACK( EditorChangeValue )
	DECLARE_EDITORCHANGEPARENT_CALLBACK( EditorChangeParent )
	DECLARE_DEBUGRENDER_FROMNET_CALLBACK( DebugRender )
END_STD_INTERFACE

//--------------------------------------------------
//!
//!	PostConstruct
//!
//--------------------------------------------------
PlaneDef::PlaneDef() :
	m_origin( CONSTRUCT_CLEAR ),
	m_ypr( CONSTRUCT_CLEAR )
{}

//--------------------------------------------------
//!
//!	PostConstruct
//!
//--------------------------------------------------
void PlaneDef::PostConstruct()
{
	CCamUtil::MatrixFromYawPitchRoll(	m_orientation,
										m_ypr.X() * DEG_TO_RAD_VALUE,
										m_ypr.Y() * DEG_TO_RAD_VALUE,
										m_ypr.Z() * DEG_TO_RAD_VALUE );

	SetFromNormalAndOrigin( m_plane, m_orientation.GetYAxis(), m_origin );
}

//--------------------------------------------------
//!
//!	DebugRender
//!
//--------------------------------------------------
void PlaneDef::DebugRender()
{
#ifndef _GOLD_MASTER
	CMatrix cube( CONSTRUCT_IDENTITY );
	cube[0][0] = 100.0f;
	cube[1][1] = 0.01f;
	cube[2][2] = 100.0f;

	cube = cube * m_orientation;
	cube.SetTranslation( m_origin );

	g_VisualDebug->RenderCube( cube, NTCOLOUR_RGBA(255,0,0,128), 0 ); 

	CMatrix mat( m_orientation );
	mat.SetTranslation( m_origin );
	EffectUtils::DebugRenderFrame( mat, 10.0f );
#endif
}





//--------------------------------------------------
//!
//!	wrappers around interface shennaingans
//!
//--------------------------------------------------
const char* EffectUtils::GetInterfaceName( void* ptr )
{
	DataObject* pDO = ObjectDatabase::Get().GetDataObjectFromPointer( ptr );
	if (pDO)
		return ntStr::GetString(pDO->GetName());
	return 0;
}

const char* EffectUtils::GetInterfaceType( void* ptr )
{
	DataObject* pDO = ObjectDatabase::Get().GetDataObjectFromPointer( ptr );
	if (pDO)
		return pDO->GetClassName();
	return 0;
}

void* EffectUtils::GetPtrFromAnyString( void* ptr )
{
#ifndef _RELEASE
	DataObject* pDO = ObjectDatabase::Get().GetDataObjectFromPointer( ptr );
	ntError( pDO );
	ntError( stricmp( pDO->GetClassName(), "AnyString" ) == 0 );
#endif

	return ObjectDatabase::Get().GetPointerFromName<void*>( ntStr::GetString( ( static_cast<AnyString*>( ptr ) )->m_obString ) );
}


//--------------------------------------------------
//!
//!	CreateQuadIndexBuffer
//! Create an index buffer for drawing a list of quads.
//!
//--------------------------------------------------
IBHandle EffectUtils::CreateQuadIndexBuffer(	u_int iNumQuads,
												u_int& iNumIndices,
												bool bUseTriLists )
{
	IBHandle result;

	if (EffectManager::AllocsDisabled())
		return result;

	ntError_p( iNumQuads <= 0x4000, ("exceeding maximum number of quads renderable with 16bit index buffers!") );

	if (bUseTriLists)
		iNumIndices = (iNumQuads * 6);
	else
		iNumIndices = (iNumQuads * 6)-2;

#ifdef PLATFORM_PC

	result = Renderer::Get().m_Platform.CreateStaticIndexBuffer(iNumIndices*sizeof(u_short));

	u_short* pIndex;
	dxerror( result->Lock( 0, 0, (void**)&pIndex, 0 ) );

	if (bUseTriLists)
	{
		for (u_int i = 0; i < iNumQuads; i++)
		{
			*pIndex++ = (u_short)(i*4)+0; // 0
			*pIndex++ = (u_short)(i*4)+1; // 1
			*pIndex++ = (u_short)(i*4)+2; // 2

			*pIndex++ = (u_short)(i*4)+2; // 2
			*pIndex++ = (u_short)(i*4)+1; // 1
			*pIndex++ = (u_short)(i*4)+3; // 3
		}
	}
	else // construct a tri-strip buffer
	{		
		u_short iCurr = 0;
		for (u_int i = 0; i < iNumQuads; i++)
		{
			for (u_int j = 0; j < 4; j++)
				*pIndex++ = iCurr++; // 0, 1, 2, 3

			if (i < iNumQuads-1) // degenerates
			{
				*pIndex++ = --iCurr; // 3
				*pIndex++ = ++iCurr; // 4
			}
		}
	}

	result->Unlock();

#elif defined (PLATFORM_PS3)

	result = RendererPlatform::CreateIndexStream( Gc::kIndex16, iNumIndices, Gc::kStaticBuffer  );

	if (bUseTriLists)
	{
		u_short indices[6];
		for (u_int i = 0; i < iNumQuads; i++)
		{
			indices[0] = (u_short)(i*4)+0; // 0
			indices[1] = (u_short)(i*4)+1; // 1
			indices[2] = (u_short)(i*4)+2; // 2

			indices[3] = (u_short)(i*4)+2; // 2
			indices[4] = (u_short)(i*4)+1; // 1
			indices[5] = (u_short)(i*4)+3; // 3

			result->Write( indices, i * 6 * sizeof(u_short), 6 * sizeof(u_short) );
		}
	}
	else // construct a tri-strip buffer
	{		
		u_short iCurr = 0;
		u_short indices[6];
		for (u_int i = 0; i < iNumQuads; i++)
		{
			for (u_int j = 0; j < 4; j++)
				indices[j] = iCurr++; // 0, 1, 2, 3

			if (i < iNumQuads-1) // degenerates
			{
				indices[4] = --iCurr; // 3
				indices[5] = ++iCurr; // 4

				result->Write( indices, i * 6 * sizeof(u_short), 6 * sizeof(u_short) );
			}
			else
			{
				result->Write( indices, i * 6 * sizeof(u_short), 4 * sizeof(u_short) );
			}
		}
	}

#endif

	return result;
}

//--------------------------------------------------
//!
//!	FindNamedTransform
//!
//--------------------------------------------------
Transform* EffectUtils::FindNamedTransform( const CHashedString& pEntName, const CHashedString& pTransformName )
{
	if ( ntStr::IsNull(pEntName))
	{
		#ifndef _RELEASE
		sprintf( aErrors, "FindNamedTransform called with no entity name" );
		EffectErrorMSG::AddDebugError( aErrors );
		#endif
		return NULL;
	}

	CEntity* pEnt = CEntityManager::Get().FindEntity( pEntName );

	if ( pEnt == NULL )
	{
		#ifndef _RELEASE
		sprintf( aErrors, "FindNamedTransform called on entity that does not exist: %s", ntStr::GetString(pEntName) );
		EffectErrorMSG::AddDebugError( aErrors );
		#endif
		return NULL;
	}

	return FindNamedTransform( pEnt, pTransformName );
}

//--------------------------------------------------
//!
//!	FindNamedTransform
//!
//--------------------------------------------------
Transform* EffectUtils::FindNamedTransform( const CEntity* pEnt, const CHashedString& pTransformName )
{
	ntAssert(pEnt);

	CHashedString	nameHash;

	if ( ntStr::IsNull(pTransformName) )
		nameHash = CHashedString(HASH_STRING_ROOT);
	else
		nameHash = CHashedString(pTransformName);

	int	iIndex = pEnt->GetHierarchy()->GetTransformIndex(nameHash);

	if	(
		( iIndex < 0 ) ||
		( iIndex >= pEnt->GetHierarchy()->GetTransformCount() )
		)
	{
		#ifndef _RELEASE
		sprintf( aErrors, "FindNamedTransform called on an invalid transform: %s", ntStr::GetString(pTransformName) );
		EffectErrorMSG::AddDebugError( aErrors );
		#endif

		nameHash = CHashedString(HASH_STRING_ROOT);
		iIndex = pEnt->GetHierarchy()->GetTransformIndex( nameHash );
	}

	return pEnt->GetHierarchy()->GetTransform( iIndex );
}

//--------------------------------------------------
//!
//!	EffectUtils::SetGlobalFXParameters
//! These should be cached by the effect manager.
//! We should also check to see if these are required at all
//! possibly the effect manager should interogate effects as
//! theyre registers / every update to see what they require
//!
//--------------------------------------------------
void EffectUtils::SetGlobalFXParameters( ID3DXEffect* pFX )
{
#ifdef PLATFORM_PC // FIXME_WIL

	ntAssert(pFX);

	// we only do this once a frame as our fx files are pooled so they
	// can all share these global constants
	static u_int iLastFrame = 0xffffffff;
	if ( iLastFrame != CTimer::Get().GetSystemTicks() )
	{
		iLastFrame = CTimer::Get().GetSystemTicks();

		const CCamera* pCam = RenderingContext::Get()->m_pViewCamera;

		CPoint cameraZ( pCam->GetViewTransform()->GetWorldMatrix().GetZAxis() );
		cameraZ.W() = -( pCam->GetViewTransform()->GetWorldMatrix().GetTranslation().Dot( cameraZ ) );
		FX_SET_VALUE_VALIDATE( pFX, "g_cameraZ", &cameraZ, sizeof(float) * 4 );

		CPoint eyePos = RenderingContext::Get()->GetEyePos();
		FX_SET_VALUE_VALIDATE( pFX, "g_eyePos", &eyePos, sizeof(float) * 3 );

		CVector vpScalars(	Renderer::Get().m_targetCache.GetWidthScalar(),
							Renderer::Get().m_targetCache.GetHeightScalar(),
							0.0f, 0.0f );

		FX_SET_VALUE_VALIDATE( pFX, "g_vpScalars", &vpScalars, sizeof(float) * 4 );
		
		FX_SET_VALUE_VALIDATE( pFX, "g_cameraUnitAxisX", &pCam->GetViewTransform()->GetWorldMatrix().GetXAxis(), sizeof(float) * 3 );
		FX_SET_VALUE_VALIDATE( pFX, "g_cameraUnitAxisY", &pCam->GetViewTransform()->GetWorldMatrix().GetYAxis(), sizeof(float) * 3 );

		FX_SET_VALUE_VALIDATE( pFX, "g_ImageTransform", &RenderingContext::Get()->m_postProcessingMatrix, sizeof(CMatrix) );

		CVector temp;
		temp = CDepthHazeSetting::GetAConsts();
		FX_SET_VALUE_VALIDATE( pFX, "g_DHConstsA", &temp, sizeof(float) * 3 );

		temp = CDepthHazeSetting::GetGConsts();
		FX_SET_VALUE_VALIDATE( pFX, "g_DHConstsG", &temp, sizeof(float) * 3 );

		temp = CDepthHazeSetting::GetBeta1PlusBeta2();
		FX_SET_VALUE_VALIDATE( pFX, "g_DHB1plusB2", &temp, sizeof(float) * 3 );		

		temp = CDepthHazeSetting::GetOneOverBeta1PlusBeta2();
		FX_SET_VALUE_VALIDATE( pFX, "g_DHRecipB1plusB2", &temp, sizeof(float) * 3 );		
	
		temp = CDepthHazeSetting::GetBetaDash1();
		FX_SET_VALUE_VALIDATE( pFX, "g_DHBdash1", &temp, sizeof(float) * 3 );
	
		temp = CDepthHazeSetting::GetBetaDash2();
		FX_SET_VALUE_VALIDATE( pFX, "g_DHBdash2", &temp, sizeof(float) * 3 );

		temp = CDepthHazeSetting::GetSunColour();
		FX_SET_VALUE_VALIDATE( pFX, "g_sunColour", &temp, sizeof(float) * 4 );
	}
#endif

}

//--------------------------------------------------
//!
//!	EffectUtils::DebugRenderFrame
//!
//--------------------------------------------------
void EffectUtils::DebugRenderFrame( const CMatrix& frame, float fScale )
{
#ifndef _GOLD_MASTER
	g_VisualDebug->RenderLine(	frame.GetTranslation(),
		(frame.GetXAxis() * fScale) + frame.GetTranslation(), NTCOLOUR_ARGB(0xff,0xff,0,0), 0 );
		
	g_VisualDebug->RenderLine(	frame.GetTranslation(),
		(frame.GetYAxis() * fScale) + frame.GetTranslation(), NTCOLOUR_ARGB(0xff,0,0xff,0), 0 );
		
	g_VisualDebug->RenderLine(	frame.GetTranslation(),
		(frame.GetZAxis() * fScale) + frame.GetTranslation(), NTCOLOUR_ARGB(0xff,0,0,0xff), 0 );
#endif
}
