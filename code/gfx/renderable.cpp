/***************************************************************************************************
*
*	DESCRIPTION
*
*	NOTES
*
***************************************************************************************************/

#include "gfx/renderable.h"
#include "anim/transform.h"
#include "anim/hierarchy.h"
#include "camera.h"
#include "core/frustum.h"
#include "core/exportstruct_clump.h"
#include "gfx/meshinstance.h"
#include "renderersettings.h"
#include "gfx/renderer.h"
#include "gfx/rendercontext.h"
#include "core/gatso.h"
#include "core/visualdebugger.h"
 
CRenderable::CRenderable(const Transform* pobTransform, bool bRenderOpaque, bool bShadowRecieve, bool bShadowCast, unsigned int iRenderableType ) 
  : 
    m_FrameFlags( RFF_CLEAR ),
    m_pobTransform( pobTransform ),
	m_bIsAlphaBlended( false ),
	m_bIsSkinned( false ),
	m_bIsShapeBlended( false ),
	m_bUseHeresy( true ),
	m_iRenderableType( iRenderableType )
{ 
	m_iDisableCount_Rendering = bRenderOpaque ? 0 : 1;
	m_iDisableCount_SCasting = bShadowCast ? 0 : 1;
	m_iDisableCount_SRecieving = bShadowRecieve ? 0 : 1;

	// XXX EVIL DIRTY HACK

	//
	//	WHAT WE'RE DOING:
	//
	//		If pobTransform is the root of a hierarchy then we want to make sure we change it to be the
	//		first child of the root, if it has one; if not, don't change the transform we store.
	//		Unfortunately we have no way to tell properly if pobTransform is the root of a hierarchy;
	//		to "approximate" this, we say that pobTransform is a hierarchy's root if pobTransform->GetParent()
	//		is either NULL or the world-root transform.
	//
	//	WHY WE'RE DOING THIS:
	//
	//		The renderer uses the root position as a kind of "object centre" for a rough bounding-box
	//		and culling calculation. Unfortunately some animations move the root quite a way from the
	//		actual object being rendered, which means that the object might not get rendered in some
	//		circumstances. There are additional difficulties when rendering ragdolls as well because
	//		the Havok ragdolls have no concept of a "root" bone and so the root is not updated properly
	//		when characters are ragdolled.
	//
	//	IMPORTANT NOTE:
	//
	//		This is a quick hack for E3 2006. We need to solve this problem - either by ensuring the root
	//		is properly updated at all times or by changing the renderer to use some other measure of
	//		where the object is in space apart from the root.
	//
	
	// Is this a hierarchy root?
	ntError( pobTransform != NULL );
	ntError( CHierarchy::GetWorld() != NULL );
	ntError( CHierarchy::GetWorld()->GetRootTransform() != NULL );
	if ( pobTransform->GetParent() == NULL || pobTransform->GetParent() == CHierarchy::GetWorld()->GetRootTransform() )
	{
		if ( pobTransform->GetFirstChild() != NULL )
		{
			pobTransform = pobTransform->GetFirstChild();
		}
	}

	// END OF HACK.
}

CRenderable::~CRenderable() {}

Transform const* CRenderable::GetTransform() const 
{
	return m_pobTransform;
}

/***************************************************************************************************
*
*	FUNCTION		CRenderable::GetBounds
*
*	DESCRIPTION		returns the AABB in local space (not transformed)
*
***************************************************************************************************/

CAABB const& CRenderable::GetBounds() const 
{
	return m_obBounds;
}

CAABB CRenderable::GetWorldSpaceAABB() const
{
	// assuming no local transform scale our world space AABB is maximum root(3) bigger and 
	// translated by world transform

/*	CAABB const& localBox = GetBounds();
	CPoint const& worldTrans = GetTransform()->GetWorldMatrixFast().GetTranslation();

	static const float root3 = 1.7320508f;
	CDirection rotInvLength = localBox.GetHalfLengths() * root3;
	rotInvLength.X() = ntstd::Max( ntstd::Max(rotInvLength.X(), rotInvLength.Y()), rotInvLength.Z() );
	rotInvLength.Z() = rotInvLength.Y() = rotInvLength.X();

	CAABB box( CONSTRUCT_DONT_CLEAR );
	box.Min() = localBox.GetCentre() + worldTrans - rotInvLength;
	box.Max() = localBox.GetCentre() + worldTrans + rotInvLength;
*/
	// the old fashioned 
	CAABB box = GetBounds();
	box.Transform( GetTransform()->GetWorldMatrixFast() );

	return box;
}

bool CRenderable::UpdateFrameFlags( CullingFrustum const* pFrustum, const unsigned int iNumShadowVolumes, CVector* pShadowPlanes, CullingFrustum* pShadowFrustums )
{
	ntAssert_p( iNumShadowVolumes <= 4, ("We only support upto 4 shadow volumes") );

	CAABB box = GetWorldSpaceAABB();

	CGatso::Start( "CRenderable::UpdateFrameFlags.Frustum" );

	// Note using fast test box will claim all box that are visible are also clipping 
	CullingFrustum::TEST_STATUS status = pFrustum->FastTestBox(&box); // pFrustum->TestBox( &box );

	// clear the frame flags (correct value for completely outside
	m_FrameFlags = RFF_CLEAR;

	switch( status )
	{
	case CullingFrustum::PARTIALLY_INSIDE:
		m_FrameFlags |= RFF_CLIPPING;
		// intential fallthrough
	case CullingFrustum::COMPLETELY_INSIDE:
		m_FrameFlags |= RFF_VISIBLE;
		break;
	default:
		break;
	}

	CGatso::Stop( "CRenderable::UpdateFrameFlags.Frustum" );

	// do shadow classification
	if( pShadowPlanes != 0 && iNumShadowVolumes > 0)
	{
		CGatso::Start( "CRenderable::UpdateFrameFlags.Shadow" );
		// recieving checks first
		if( IsShadowRecieving() )
		{
			if( m_FrameFlags & RFF_VISIBLE )
			{
				for( unsigned int i=0;i < iNumShadowVolumes;i++)
				{
					switch( pShadowFrustums[i].TestBox( &box ) )
					{
					case CullingFrustum::PARTIALLY_INSIDE:
						m_FrameFlags |= (RFF_CLIPS_SHADOWMAP0 << i);
						// intential fallthrough
					case CullingFrustum::COMPLETELY_INSIDE:
						m_FrameFlags |= (RFF_RECIEVES_SHADOWMAP0 << i);
						break;
					default:
						break;
					}
				}
			}
		}
		if( IsShadowCasting() )
		{
			// the object doesn't intersect the frustum itself but
			// what about its shadow?
			// its definately not a reciever (its not visible) but the swept test
			// will tell if its a caster
			const float obWorldShadowLength = 1000.f;
			CDirection obWorldScaledShadowDir = RenderingContext::Get()->m_shadowDirection * obWorldShadowLength;

			CSphereBound sphere( box );
			for( unsigned int i=0;i < iNumShadowVolumes;i++)
			{
				if( pShadowFrustums[i].TestSweptSphere( &sphere, &obWorldScaledShadowDir ) )
				{
					m_FrameFlags |= (RFF_CAST_SHADOWMAP0 << i);
				}
			}
		}

		CGatso::Stop( "CRenderable::UpdateFrameFlags.Shadow" );
	}

	return (m_FrameFlags & RFF_VISIBLE) ? true : false;
}

float g_MaxDiagonal = 5000.0f;
void CRenderable::CalculateSortKey(const CMatrix *pTransform)
{
	CMatrix obTransform = m_pobTransform->GetWorldMatrixFast();
	CPoint vCentre = m_obBounds.GetCentre() * obTransform;

	// transform so we get a float z in camera space
	float fViewZ = (vCentre * (*pTransform)).Z();

	// negative z treated as positive (not right but shouldn't matter)

	CAABB AABB = GetWorldSpaceAABB();
	float diagonal = AABB.GetHalfLengths().Length();
	unsigned int bigObjsFlag = 0;
	if ( diagonal > g_MaxDiagonal )
	{
		bigObjsFlag = 1;
	}

	unsigned int iVal = 0;
	if( m_bIsAlphaBlended )
	{
		// set upper bit so alpha > non-alpha
		iVal = 0x80000000;

		// sort back to front so val = 1/iViewZ
		if( fabsf(fViewZ) > 1e-5f )
		{
			float fVal = 1.f / fabsf(fViewZ);
			iVal = iVal | (*reinterpret_cast<unsigned int*>(&fVal) >> 1) & ~0x80000000;
		} 
		//else
		//{
		//	iVal = 0xFFFFFFFF; 
		//}
	} else
	{
		iVal = 0x0;
		// sort front to back 
		float fVal = fabsf(fViewZ);


		iVal = (iVal | (bigObjsFlag<<30) | (*reinterpret_cast<unsigned int*>(&fVal) >> 2)) & ~0x80000000;
	}

	m_iSortKey = iVal;
}

IFrameFlagsUpdateCallback* CRenderable::GetUpdateFrameFlagsCallback() const
{
    return NULL;
}

void CRenderable::DisplayBounds()
{
	CAABB box = GetWorldSpaceAABB();
	box.DebugRender( CMatrix(CONSTRUCT_IDENTITY), NTCOLOUR_ARGB(0xff,0xff,0,0), DPF_WIREFRAME );

//	m_obBounds.DebugRender( m_pobTransform->GetWorldMatrixFast(), 0xffff0000, DPF_WIREFRAME );
}

