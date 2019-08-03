//--------------------------------------------------
//!
//!	\file core/visualdebugger.cpp
//!
//--------------------------------------------------
#include "core/visualdebugger.h"
//#include "gfx/renderer.h"			// this is only required for world to screen computations. we should shift it
#include "gfx/camera.h"
#include "anim/transform.h"			// im thinking this should be in core dean...
#include "anim/hierarchy.h"


void VisualDebuggerInterface::RenderHierarchy(const CHierarchy& h, float fAxisSize, const CDirection& offset, int iFlags)
{
	RenderSphere(CQuat(CONSTRUCT_IDENTITY), h.GetTransform(0)->GetWorldTranslation() + offset, fAxisSize, DebugColour::red(100));
	
	for(int iElem = 1; iElem < h.GetTransformCount() ; iElem++ )
	{
		const CPoint& old = h.GetTransform(iElem)->GetParent()->GetWorldTranslation();
		const CMatrix& world = h.GetTransform(iElem)->GetWorldMatrix();
		RenderSphere(CQuat(CONSTRUCT_IDENTITY), CPoint(world[3])+offset, fAxisSize*0.1f, DebugColour::black(100), iFlags);
		RenderLine(old+offset,CPoint(world[3])+offset,DebugColour::white(), iFlags);
		RenderAxis( world, fAxisSize, iFlags);
	}
}

bool VisualDebuggerInterface::WorldToScreen( const CPoint& world, CPoint& screen )
{
	float fWidth, fHeight;
	const CCamera* pCurrCamera = GetDrawParameters( fWidth, fHeight );
	ntAssert_p( pCurrCamera, ("Must have a valid camera here!"));

	CMatrix currCamera = pCurrCamera->GetViewTransform()->GetWorldMatrix();
	CPoint test = world - currCamera.GetTranslation();
	
	if (test.Dot( CPoint(currCamera.GetZAxis()) ) > 0.0f)
	{
		// transform point to projection space
		CMatrix proj;
		pCurrCamera->GetProjection( fWidth / fHeight, proj );

		CVector temp( world * currCamera.GetAffineInverse() );
		temp.W() = 1.0f;
		temp = temp * proj;

		// deal with points at infinity. sigh
		if( temp.W() < EPSILON )
			temp = CVector( 0.0f, 0.0f, -1.0f, 1.0f );
		else
			temp /= fabsf(temp.W());

		// return the viewport coordinate
		temp += CVector(1.0f, -1.0f, 0.0f, 0.0f);
		temp *= CVector(0.5f*fWidth, -0.5f*fHeight, 1.0f, 1.0f);

		screen = CPoint( temp );
		return true;
	}
	return false;
}

class DummyVisualDebugger : public VisualDebuggerInterface
{
public:
	
	virtual void RenderSphere(CMatrix const&, uint32_t, int){};
	virtual void RenderCube(CMatrix const&, uint32_t, int){};
	virtual void RenderCapsule(CMatrix const&, float, uint32_t, int){};
	virtual void RenderArc(CMatrix const&, float, float, uint32_t, int){};
	virtual void RenderLine(CPoint const&, CPoint const&, uint32_t, int){};
	virtual void RenderPoint(CPoint const&, float, uint32_t, int){};
	virtual void RenderDirectedLine(CPoint const&, CPoint const&, float, uint32_t, int){};
	virtual void RenderPrimitive(const CPoint*, int, CMatrix const&, uint32_t, int){};
	virtual void RenderGraph(const CGraph*, CPoint const&, CPoint const&, int){};
	virtual void RenderCube(CMatrix const& matrix, uint32_t iColour, float size, int iFlags = 0)
	{
		UNUSED( matrix );
		UNUSED( iColour );
		UNUSED( size );
		UNUSED( iFlags );
	}

	virtual float GetDebugDisplayWidth(){ return 0.0f; }
	virtual float GetDebugDisplayHeight(){ return 0.0f; }
	virtual const CCamera* GetDrawParameters( float&, float& ) { return 0; }

	virtual void Printf2D( float, float, uint32_t, int, const char* pcTxt, ... )
	{
		static const int MAXTEXTLEN = 2048;
		char pcBuffer[ MAXTEXTLEN ];
		// Format the text.
		va_list	stArgList;
		va_start(stArgList, pcTxt);
		int iLen = vsnprintf(pcBuffer, MAXTEXTLEN-1, pcTxt, stArgList);
		UNUSED( iLen );
		va_end(stArgList);
		ntPrintf( pcBuffer );
	}
	virtual void Printf3D( CPoint const&, uint32_t, int, const char*, ... ){}
	virtual void Printf3D( CPoint const&, float, float, uint32_t, int, const char*, ... ){}
	virtual void Draw3D(){}
	virtual void Draw2D(){}
	virtual void Reset(){}

} g_DummyVisualDebugger;

VisualDebuggerInterface* g_VisualDebug = &g_DummyVisualDebugger;
