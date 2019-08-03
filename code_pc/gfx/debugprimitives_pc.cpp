/***************************************************************************************************
*
*	$Header:: /game/debugprimitives.cpp 33    13/08/03 10:39 Simonb                                $
*
*	Renders debug primitives. 
*
*	CHANGESCHANGES
*
*	05/03/2003	Simon	Created
*
***************************************************************************************************/

#include "gfx/renderer.h"
#include "gfx/camera.h"
#include "core/timer.h"
#include "gfx/shader.h"
#include "anim/transform.h"
#include "gfx/graphing.h"
#include "gfx/graphicsdevice.h"
#include "gfx/dxerror_pc.h"
#include "gfx/debugprimitives_pc.h"
#include "gfx/debugsphere_pc.inl"
#include "gfx/debugcube_pc.inl"
#include "gfx/debugcapsule_pc.inl"
#include "gfx/debugline_pc.inl"

/***************************************************************************************************
*
*	CLASS			STANDARD_PRIMITIVE_SETUP
*
*	DESCRIPTION		Holds initialisation data for the standard rendering primitives.
*
***************************************************************************************************/

struct STANDARD_PRIMITIVE_SETUP
{
	const float* pfPositions;
	const u_short* pusIndices;
	int iNumVertices;
	int iNumIndices;
};

//! Initialisation data for the standard rendering primitives.
STANDARD_PRIMITIVE_SETUP g_astDebugPrimitiveSetup[] = 
{
	{ g_afSpherePositions,	g_ausSphereIndices,		sizeof(g_afSpherePositions)/(4*sizeof(float)),	sizeof(g_ausSphereIndices)/sizeof(u_short) }, 
	{ g_afCubePositions,	g_ausCubeIndices,		sizeof(g_afCubePositions)/(4*sizeof(float)),	sizeof(g_ausCubeIndices)/sizeof(u_short) }, 
	{ g_afCapsulePositions,	g_ausCapsuleIndices,	sizeof(g_afCapsulePositions)/(4*sizeof(float)),	sizeof(g_ausCapsuleIndices)/sizeof(u_short) }, 
	{ g_afLinePositions,	g_ausLineIndices,		sizeof(g_afLinePositions)/(4*sizeof(float)),	sizeof(g_ausLineIndices)/sizeof(u_short) }, 
	{ 0, 0, 0, 0 }
};

/***************************************************************************************************
*
*	FUNCTION		CDebugPrimitives::CDebugPrimitives
*
*	DESCRIPTION		Creates the debug primitive manager.
*
***************************************************************************************************/

DebugPrimitives::DebugPrimitives(int iBufferLength)
  : m_iMaxNumPrimitives(iBufferLength), 
	m_iNumPrimitives(0), 
	m_aobPrimitives(NT_NEW_CHUNK(Mem::MC_GFX) PRIMITIVE[iBufferLength]), 
	m_aobQueue(NT_NEW_CHUNK(Mem::MC_GFX) const PRIMITIVE*[iBufferLength])
{
	// count the number of vertices, indices and primitives
	int iVertexCount = 0, iIndexCount = 0, iPrimitiveCount = 0;
	for(; g_astDebugPrimitiveSetup[iPrimitiveCount].iNumVertices != 0; ++iPrimitiveCount)
	{
		iVertexCount += g_astDebugPrimitiveSetup[iPrimitiveCount].iNumVertices;
		iIndexCount += g_astDebugPrimitiveSetup[iPrimitiveCount].iNumIndices;
	}

	// create storage for all
	m_astStandardPrimitives.Reset(NT_NEW_CHUNK(Mem::MC_GFX) STANDARD_PRIMITIVE[iPrimitiveCount]);

	m_pobPositions = Renderer::Get().m_Platform.CreateStaticVertexBuffer( iVertexCount*4*sizeof(float) );
	m_pobIndices = Renderer::Get().m_Platform.CreateStaticIndexBuffer( iIndexCount*sizeof(u_short) );

	// lock the vertex positions storage
	float* pfPositions = 0;
	m_pobPositions->Lock(0, 0, reinterpret_cast<void**>(&pfPositions), 0);

	// lock the indices storage
	u_short* pusIndices = 0;
	m_pobIndices->Lock(0, 0, reinterpret_cast<void**>(&pusIndices), 0);

	// copy the data
	int iVertexOffset = 0, iIndexOffset = 0;
	for(int iPrimitive = 0; iPrimitive < iPrimitiveCount; ++iPrimitive)
	{
		// store the data
        NT_MEMCPY(&pfPositions[4*iVertexOffset], g_astDebugPrimitiveSetup[iPrimitive].pfPositions, g_astDebugPrimitiveSetup[iPrimitive].iNumVertices*4*sizeof(float));
        for(int iIndex = 0; iIndex < g_astDebugPrimitiveSetup[iPrimitive].iNumIndices; ++iIndex)
			pusIndices[iIndexOffset + iIndex] = static_cast<u_short>(g_astDebugPrimitiveSetup[iPrimitive].pusIndices[iIndex] + iVertexOffset);
		
        // record the primitive
		m_astStandardPrimitives[iPrimitive].iVertexOffset = iVertexOffset;
		m_astStandardPrimitives[iPrimitive].iIndexOffset = iIndexOffset;
		m_astStandardPrimitives[iPrimitive].iNumVertices = g_astDebugPrimitiveSetup[iPrimitive].iNumVertices;
		m_astStandardPrimitives[iPrimitive].iNumIndices = g_astDebugPrimitiveSetup[iPrimitive].iNumIndices;

		// increment the offsets
		iVertexOffset += g_astDebugPrimitiveSetup[iPrimitive].iNumVertices;
		iIndexOffset += g_astDebugPrimitiveSetup[iPrimitive].iNumIndices;
	}

	// unlock the storage
	m_pobPositions->Unlock();
	m_pobIndices->Unlock();

	// load the shaders
	m_obVertexShader.SetASMFunction(
		SHADERTYPE_VERTEX, 
		"vs_1_1 \n"
		"dcl_position0 v0 \n"		// v0 = position
		"dcl_texcoord0 v1 \n"		// v1 = (zoffset, 0, 0, 1)

		// c1 = projection
		// c5 = (zscale, #, #, #)
		// c6 = colour
		// c7 = lightdir
		"def c8, 0.0f, 0.5f, 1.0f, 0.0f \n"			// c8 = (0.0f, 0.5f, 1.0f, #)
		
		"dp3 r0.w, v0, v0 \n"
		"rsq r0.w, r0.wwww \n"
		"mul r0.xyz, v0, r0.wwww \n"				// normalise v0 into r0

		"dp3 r1.w, r0, c7 \n"
		"min r1.w, r1.wwww, c8.xxxx \n"
		"mad r1.w, r1.wwww, c8.yyyy, c8.zzzz \n"	// r1 = 1.0f + 0.5f*min(dot(normal, lightdir), 0.0f)

		"mov r2, v0 \n"
		"mad r2.z, v1.xxxx, c5.xxxx, r2 \n"			// position.z += zoffset*zscale (into r2)

		"m4x4 r0, r2, c1 \n"						// transform position
		"mad oPos, r0.wwww, c0, r0 \n"				// texel adjustment

		"mul oD0.xyz, c6, r1.wwww \n"
		"mov oD0.w, c6.wwww \n"
	);

	m_obGraphVertexShader.SetASMFunction(
		SHADERTYPE_VERTEX, 
		"vs_1_1 \n"
		"dcl_position0 v0 \n"		// v0 = position

		// c1 = projection
		// c5 = (zscale, #, #, #)
		// c6 = colour
		// c7 = lightdir
		"m4x4 r0, v0, c1 \n"						// transform position
		"mad oPos, r0.wwww, c0, r0 \n"				// texel adjustment

		"mov oD0, c6 \n"
	);
    
	m_obPixelShader.SetASMFunction(
		SHADERTYPE_PIXEL, 
		//"ps_2_0 \n"
		//"dcl v0 \n"
		//"mov oC0, v0 \n"
		"ps_1_1 \n"
		"mov r0, v0 \n"
	);

	D3DVERTEXELEMENT9 stElements[] = 
	{
		{ 0, 0,					D3DDECLTYPE_FLOAT3,		D3DDECLMETHOD_DEFAULT, D3DDECLUSAGE_POSITION,	0 }, 
		{ 0, 3*sizeof(float),	D3DDECLTYPE_FLOAT1,		D3DDECLMETHOD_DEFAULT, D3DDECLUSAGE_TEXCOORD,	0 }, 
		D3DDECL_END()
	};
	dxerror( GetD3DDevice()->CreateVertexDeclaration(&stElements[0], m_pobVertexDeclaration.AddressOf()) );

	D3DVERTEXELEMENT9 stGraphElements[] = 
	{
		{ 0, 0,					D3DDECLTYPE_FLOAT2,		D3DDECLMETHOD_DEFAULT, D3DDECLUSAGE_POSITION,	0 }, 
		D3DDECL_END()
	};
	dxerror( GetD3DDevice()->CreateVertexDeclaration(&stGraphElements[0], m_pobGraphVertexDeclaration.AddressOf()) );
}

/***************************************************************************************************
*
*	FUNCTION		CDebugPrimitives::RenderSphere
*
*	DESCRIPTION		Queues a sphere to be rendered.
*
***************************************************************************************************/

void DebugPrimitives::RenderSphere(CMatrix const& obLocalTransform, uint32_t dwColour, int iFlags)
{
	// check for space
	if(m_iNumPrimitives >= m_iMaxNumPrimitives)
		return;

	// check the flags (only options make sense)
	iFlags &= DEBUG_PRIMITIVE_WIREFRAME | DEBUG_PRIMITIVE_NOCULLING | DEBUG_PRIMITIVE_NOZCOMPARE; 
	
	// queue the item
	m_aobPrimitives[m_iNumPrimitives].obLocalTransform = obLocalTransform;
	m_aobPrimitives[m_iNumPrimitives].iType = DEBUG_PRIMITIVE_SPHERE | iFlags;
	m_aobPrimitives[m_iNumPrimitives].dwColour = dwColour;
	m_aobPrimitives[m_iNumPrimitives].pobVertices = 0;
	m_aobPrimitives[m_iNumPrimitives].uData.iNumVertices = 0;

	// add a queue entry for later sorting
	m_aobQueue[m_iNumPrimitives] = &m_aobPrimitives[m_iNumPrimitives];

	// increment the debug counter
	++m_iNumPrimitives;
}

/***************************************************************************************************
*
*	FUNCTION		CDebugPrimitives::RenderCube
*
*	DESCRIPTION		Queues a cube to be rendered.
*
***************************************************************************************************/

void DebugPrimitives::RenderCube(CMatrix const& obLocalTransform, uint32_t dwColour, int iFlags)
{
	// check for space
	if(m_iNumPrimitives >= m_iMaxNumPrimitives)
		return;

	// check the flags (only options make sense)
	iFlags &= DEBUG_PRIMITIVE_WIREFRAME | DEBUG_PRIMITIVE_NOCULLING | DEBUG_PRIMITIVE_NOZCOMPARE; 
	
	// queue the item
	m_aobPrimitives[m_iNumPrimitives].obLocalTransform = obLocalTransform;
	m_aobPrimitives[m_iNumPrimitives].iType = DEBUG_PRIMITIVE_CUBE | iFlags;
	m_aobPrimitives[m_iNumPrimitives].dwColour = dwColour;
	m_aobPrimitives[m_iNumPrimitives].pobVertices = 0;
	m_aobPrimitives[m_iNumPrimitives].uData.iNumVertices = 0;

	// add a queue entry for later sorting
	m_aobQueue[m_iNumPrimitives] = &m_aobPrimitives[m_iNumPrimitives];

	// increment the debug counter
	++m_iNumPrimitives;
}

/***************************************************************************************************
*
*	FUNCTION		CDebugPrimitives::RenderCapsule
*
*	DESCRIPTION		Queues a capsule to be rendered.
*
***************************************************************************************************/

void DebugPrimitives::RenderCapsule(CMatrix const& obLocalTransform, float fLocalLength, uint32_t dwColour, int iFlags)
{
	// check for space
	if(m_iNumPrimitives >= m_iMaxNumPrimitives)
		return;

	// check the flags (only options make sense)
	iFlags &= DEBUG_PRIMITIVE_WIREFRAME | DEBUG_PRIMITIVE_NOCULLING | DEBUG_PRIMITIVE_NOZCOMPARE; 
	
	// queue the item
	m_aobPrimitives[m_iNumPrimitives].obLocalTransform = obLocalTransform;
	m_aobPrimitives[m_iNumPrimitives].iType = DEBUG_PRIMITIVE_CAPSULE | iFlags;
	m_aobPrimitives[m_iNumPrimitives].dwColour = dwColour;
	m_aobPrimitives[m_iNumPrimitives].pobVertices = 0;
	m_aobPrimitives[m_iNumPrimitives].uData.fFloatData = fLocalLength;

	// add a queue entry for later sorting
	m_aobQueue[m_iNumPrimitives] = &m_aobPrimitives[m_iNumPrimitives];

	// increment the debug counter
	++m_iNumPrimitives;
}


/***************************************************************************************************
*
*	FUNCTION		CDebugPrimitives::RenderArc
*
*	DESCRIPTION		Queues an arc to be rendered - writes out a number of debug lines - moving out
*					in both directions from the given mid point.
*
***************************************************************************************************/

void DebugPrimitives::RenderArc( CMatrix const& obLocalTransform, float fRadius, float fSweep, uint32_t dwColour, int iFlags )
{
	// Check for rendering space
	if( m_iNumPrimitives >= m_iMaxNumPrimitives )
		return;

	// Draw each side using our helper
	RenderHalfArc( obLocalTransform, fRadius, fSweep / 2.0f, dwColour, iFlags, true );
	RenderHalfArc( obLocalTransform, fRadius, fSweep / 2.0f, dwColour, iFlags, false );
}


/***************************************************************************************************
*
*	FUNCTION		CDebugPrimitives::RenderHalfArc
*
*	DESCRIPTION		Helper for drawing arcs
*
***************************************************************************************************/
void DebugPrimitives::RenderHalfArc(	CMatrix const&	obLocalTransform, 
										float			fRadius, 
										float			fHalfSweep, 
										uint32_t			dwColour, 
										int				iFlags, 
										bool			bPositiveX )
{

	// Points on a unit radius circle - every 10 degrees
	static const int iPointsPerHalf = 18;
	static const float fSweepPerLine = 10.0f;

	// The points we will need for z point values
	static const float afZCirclePoints[iPointsPerHalf + 1] = 
	{	
		1.0f,	0.984808f,	0.939693f,	0.866025f,	0.766044f,	0.642788f,	0.5f,		0.342020f,	0.173648f,	0.0f,
				-0.173648f,	-0.342020f,	-0.5f,		-0.642788f,	-0.766044f,	-0.866025f,	-0.939693f,	-0.984808f,	-1.0f
	};

	// The points we will need for x point values
	static const float afXCirclePoints[iPointsPerHalf + 1] = 
	{	
		0.0f,	0.173648f,	0.342020f,	0.5f,		0.642788f,	0.766044f,	0.866025f,	0.939693f,	0.984808f,	1.0f,
				0.984808f,	0.939693f,	0.866025f,	0.766044f,	0.642788f,	0.5f,		0.342020f,	0.173648f,	0.0f
	};

	// Set up a multiplier for the x values
	float fXMultiplier = ( bPositiveX ) ? 1.0f : -1.0f;

	// Set up the 'starting' start point
	CPoint obStart( 0.0f, 0.0f, fRadius );

	// Loop through and draw the lines about each side	
	float fDegrees = 0.0f;
	for ( int iLine = 0; iLine < iPointsPerHalf; ++iLine )
	{
		// Add on the degrees for this line
		fDegrees += fSweepPerLine;

		// If we have gone past our sweep then we need to calculate a point
		if ( fDegrees > ( fHalfSweep * RAD_TO_DEG_VALUE ) )
		{
			// Calculate the end point in local space
			CPoint obEnd(	fsinf( fHalfSweep ) * fRadius * fXMultiplier, 
							0.0f, 
							fcosf( fHalfSweep ) * fRadius );

			// Transform to world space in the renderline call
			RenderLine( ( obStart * obLocalTransform ), ( obEnd * obLocalTransform ), dwColour, iFlags );
		}

		// ...otherwise we can just reference points from our array
		else
		{
			// Calculate the end point in local space
			CPoint obEnd(	afXCirclePoints[iLine + 1] * fRadius * fXMultiplier, 
							0.0f, 
							afZCirclePoints[iLine + 1] * fRadius );

			// Transform to world space in the renderline call
			RenderLine( ( obStart * obLocalTransform ), ( obEnd * obLocalTransform ), dwColour, iFlags );

			// Save the end point for the next start point
			obStart = obEnd;
		}
	}
}


/***************************************************************************************************
*
*	FUNCTION		CDebugPrimitives::RenderLine
*
*	DESCRIPTION		Queues a line to be rendered.
*
***************************************************************************************************/

void DebugPrimitives::RenderLine(CPoint const& obWorldStart, CPoint const& obWorldEnd, uint32_t dwColour, int iFlags)
{
	// check for space
	if(m_iNumPrimitives >= m_iMaxNumPrimitives)
		return;

	// check the flags (all lines are wireframe and not culled)
	iFlags &= DEBUG_PRIMITIVE_SCREENSPACE | DEBUG_PRIMITIVE_NOZCOMPARE;
	iFlags |= DEBUG_PRIMITIVE_WIREFRAME; 
	
	// compute the line transform
	CMatrix obLineTransform(CONSTRUCT_IDENTITY);
	obLineTransform.SetZAxis(CDirection(obWorldEnd - obWorldStart));
	obLineTransform.SetTranslation(obWorldStart);

	// queue the item
	m_aobPrimitives[m_iNumPrimitives].obLocalTransform = obLineTransform;
	m_aobPrimitives[m_iNumPrimitives].iType = DEBUG_PRIMITIVE_LINE | iFlags;
	m_aobPrimitives[m_iNumPrimitives].dwColour = dwColour;
	m_aobPrimitives[m_iNumPrimitives].pobVertices = 0;
	m_aobPrimitives[m_iNumPrimitives].uData.iNumVertices = 0;

	// add a queue entry for later sorting
	m_aobQueue[m_iNumPrimitives] = &m_aobPrimitives[m_iNumPrimitives];

	// increment the debug counter
	++m_iNumPrimitives;
}

/***************************************************************************************************
*
*	FUNCTION		CDebugPrimitives::RenderPoint
*
*	DESCRIPTION		Queues a point to be rendered.
*
***************************************************************************************************/

void DebugPrimitives::RenderPoint(CPoint const& obWorldPos, float fPointSize, uint32_t dwColour, int iFlags)
{
	// check for space
	if(m_iNumPrimitives >= m_iMaxNumPrimitives)
		return;

	// check the flags (no flags make sense for points)
	iFlags &= DEBUG_PRIMITIVE_SCREENSPACE | DEBUG_PRIMITIVE_NOZCOMPARE;
	
	// compute the line point transform
	CMatrix obPointTransform(CONSTRUCT_IDENTITY);
	obPointTransform.SetTranslation(obWorldPos);

	// queue the item
	m_aobPrimitives[m_iNumPrimitives].obLocalTransform = obPointTransform;
	m_aobPrimitives[m_iNumPrimitives].iType = DEBUG_PRIMITIVE_POINT | iFlags;
	m_aobPrimitives[m_iNumPrimitives].dwColour = dwColour;
	m_aobPrimitives[m_iNumPrimitives].pobVertices = 0;
	m_aobPrimitives[m_iNumPrimitives].uData.fFloatData = fPointSize;

	// add a queue entry for later sorting
	m_aobQueue[m_iNumPrimitives] = &m_aobPrimitives[m_iNumPrimitives];

	// increment the debug counter
	++m_iNumPrimitives;
}

/***************************************************************************************************
*
*	FUNCTION		CDebugPrimitives::RenderDirectedLine
*
*	DESCRIPTION		Queues a directed line to be rendered.
*
***************************************************************************************************/

void DebugPrimitives::RenderDirectedLine(CPoint const& obWorldStart, CPoint const& obWorldEnd, float fSpeed, uint32_t dwColour, int iFlags)
{
	// check for space
	if(m_iNumPrimitives >= m_iMaxNumPrimitives)
		return;

	// check the flags (all lines are wireframe and not culled)
	iFlags &= DEBUG_PRIMITIVE_SCREENSPACE | DEBUG_PRIMITIVE_NOZCOMPARE; 
	iFlags |= DEBUG_PRIMITIVE_WIREFRAME;
	
	// compute the line transform
	CMatrix obLineTransform(CONSTRUCT_IDENTITY);
	obLineTransform.SetZAxis(CDirection(obWorldEnd - obWorldStart));
	obLineTransform.SetTranslation(obWorldStart);

	// queue the item
	m_aobPrimitives[m_iNumPrimitives].obLocalTransform = obLineTransform;
	m_aobPrimitives[m_iNumPrimitives].iType = DEBUG_PRIMITIVE_DIRECTEDLINE | iFlags;
	m_aobPrimitives[m_iNumPrimitives].dwColour = dwColour;
	m_aobPrimitives[m_iNumPrimitives].pobVertices = 0;
	m_aobPrimitives[m_iNumPrimitives].uData.fFloatData = fSpeed;

	// add a queue entry for later sorting
	m_aobQueue[m_iNumPrimitives] = &m_aobPrimitives[m_iNumPrimitives];

	// increment the debug counter
	++m_iNumPrimitives;
}

/***************************************************************************************************
*
*	FUNCTION		CDebugPrimitives::RenderGraph
*
*	DESCRIPTION		Queues a graph to be rendered.
*
***************************************************************************************************/

void DebugPrimitives::RenderGraph(const CGraph* pobGraph, CPoint const& obTopLeft, CPoint const& obBottomRight, int iFlags)
{
	// check for space
	if(m_iNumPrimitives >= m_iMaxNumPrimitives)
		return;

	// Set the flags (only valid flag is no-z-compare)
	iFlags = DEBUG_PRIMITIVE_SCREENSPACE | DEBUG_PRIMITIVE_VIEWPORTSPACE;
	
	// compute the line transform
	CMatrix obLineTransform(CONSTRUCT_IDENTITY);
	obLineTransform[0][0] = obBottomRight.X() - obTopLeft.X();
	obLineTransform[1][1] = obTopLeft.Y() - obBottomRight.Y();
	obLineTransform[2][2] = 0.0f;
	obLineTransform.SetTranslation(CPoint(obTopLeft.X(), obBottomRight.Y(), 0.0f));

	// queue the item
	m_aobPrimitives[m_iNumPrimitives].obLocalTransform = obLineTransform;
	m_aobPrimitives[m_iNumPrimitives].iType = DEBUG_PRIMITIVE_GRAPH | iFlags;
	m_aobPrimitives[m_iNumPrimitives].dwColour = 0x00000000;
	m_aobPrimitives[m_iNumPrimitives].pobVertices = reinterpret_cast<const CPoint*>(pobGraph);
	m_aobPrimitives[m_iNumPrimitives].uData.fFloatData = 0.0f;

	// add a queue entry for later sorting
	m_aobQueue[m_iNumPrimitives] = &m_aobPrimitives[m_iNumPrimitives];

	// increment the debug counter
	++m_iNumPrimitives;
}

/***************************************************************************************************
*
*	FUNCTION		CDebugPrimitives::RenderPrimitive
*
*	DESCRIPTION		Queues a user primitive to be rendered.
*
***************************************************************************************************/

void DebugPrimitives::RenderPrimitive(const CPoint* pobVertices, int iNumVertices, CMatrix const& obLocalTransform, uint32_t dwColour, int iFlags)
{
	// check for space
	if(m_iNumPrimitives >= m_iMaxNumPrimitives)
		return;

	// check the flags
	ntAssert(iNumVertices > 0);
	iFlags &= DEBUG_PRIMITIVE_WIREFRAME |DEBUG_PRIMITIVE_NOCULLING 
		| DEBUG_PRIMITIVE_SCREENSPACE | DEBUG_PRIMITIVE_NOZCOMPARE
		| DEBUG_PRIMITIVE_TRIANGLELIST;

	// default to a triangle list if the type is unspecified
	if((iFlags & (DEBUG_PRIMITIVE_TRIANGLELIST)) == 0)
		iFlags |= DEBUG_PRIMITIVE_TRIANGLELIST;
	
	// queue the item
	m_aobPrimitives[m_iNumPrimitives].obLocalTransform = obLocalTransform;
	m_aobPrimitives[m_iNumPrimitives].iType = DEBUG_PRIMITIVE_USER | iFlags;
	m_aobPrimitives[m_iNumPrimitives].dwColour = dwColour;
	m_aobPrimitives[m_iNumPrimitives].pobVertices = pobVertices;
	m_aobPrimitives[m_iNumPrimitives].uData.iNumVertices = iNumVertices;

	// add a queue entry for later sorting
	m_aobQueue[m_iNumPrimitives] = &m_aobPrimitives[m_iNumPrimitives];

	// increment the debug counter
	++m_iNumPrimitives;
}

/***************************************************************************************************
*
*	FUNCTION		CDebugPrimitives::Draw
*
*	DESCRIPTION		Renders everything in the queue to the current viewport.
*
***************************************************************************************************/

void DebugPrimitives::Draw(const CCamera* pobCamera, CDirection const& obScale, CDirection const& obOffset)
{
	if(m_iNumPrimitives > 0 && pobCamera != 0)
	{
		static const int iProjectionRegister = 1;
		static const int iZScaleRegister = 5;
		static const int iColourRegister = 6;
		static const int iLightDirRegister = 7;

		// sort the queue
		ntstd::sort(&m_aobQueue[0], &m_aobQueue[m_iNumPrimitives], CPrimitiveSorter(pobCamera->GetViewTransform()->GetWorldMatrix().GetZAxis()));

		// load the shaders
		Renderer::Get().SetVertexShader(&m_obVertexShader);
		Renderer::Get().SetPixelShader(&m_obPixelShader);

		// load the format
		Renderer::Get().m_Platform.SetVertexDeclaration( m_pobVertexDeclaration );

		// set the inputs
		GetD3DDevice()->SetIndices(m_pobIndices.Get());
		GetD3DDevice()->SetStreamSource(0, m_pobPositions.Get(), 0, 4*sizeof(float));

		// set universal render states
		GetD3DDevice()->SetRenderState(D3DRS_SHADEMODE, D3DSHADE_FLAT);

		// compute the world-space projection
		CMatrix viewToScreen;
		pobCamera->GetProjection( Renderer::Get().m_targetCache.GetAspectRatio(), viewToScreen );
		CMatrix obWorldProjection = pobCamera->GetViewTransform()->GetWorldMatrix().GetAffineInverse() * viewToScreen;
			

		// compute the viewport-space transform
		CMatrix obViewportTransform(CONSTRUCT_IDENTITY);
		obViewportTransform[0][0] = obScale.X();
		obViewportTransform[3][0] = obOffset.X();
		obViewportTransform[1][1] = obScale.Y();
		obViewportTransform[3][1] = obOffset.Y();

		// render them
		for(int iPrimitive = 0; iPrimitive < m_iNumPrimitives; ++iPrimitive)
		{
			const PRIMITIVE* pstPrimitive = m_aobQueue[iPrimitive];
			int iType = pstPrimitive->iType & DEBUG_PRIMITIVE_USER;

			CMatrix obProjectionMatrix;

			bool bViewportSpace = false;
			if	(
				(pstPrimitive->iType & DEBUG_PRIMITIVE_VIEWPORTSPACE) ||
				(pstPrimitive->iType & DEBUG_PRIMITIVE_DISPLAYSPACE)
				)
				bViewportSpace = true;

			if(bViewportSpace)
			{
				obProjectionMatrix = (pstPrimitive->obLocalTransform*obViewportTransform).GetTranspose();

				// upload the viewport space transform with no lighting
				Renderer::Get().SetVertexShaderConstant(iLightDirRegister, &CVecMath::GetZeroDirection(), 1);
			}
			else
			{
				obProjectionMatrix = (pstPrimitive->obLocalTransform*obWorldProjection).GetTranspose();

				// upload the light direction
				CDirection obLightDir = CDirection(1.0f, 1.0f, -1.0f)*pstPrimitive->obLocalTransform.GetTranspose();
				obLightDir.Normalise();
				Renderer::Get().SetVertexShaderConstant(iLightDirRegister, &obLightDir, 1);
			}

			// upload the complete transform
			Renderer::Get().SetVertexShaderConstant(iProjectionRegister, &obProjectionMatrix, 4);

			// upload the colour
			CVector obColour;
			obColour.SetFromNTColor( pstPrimitive->dwColour );
			Renderer::Get().SetVertexShaderConstant(iColourRegister, &obColour, 1);

			// check for wireframe and alpha blending
			bool bWireframe = (pstPrimitive->iType & DEBUG_PRIMITIVE_WIREFRAME) != 0;
			uint32_t iAlphaMask = (0xffu << NTCOLOUR_A_SHIFT);
			bool bAlpha = (pstPrimitive->dwColour & iAlphaMask) != iAlphaMask;
			bool bCull = (pstPrimitive->iType & DEBUG_PRIMITIVE_NOCULLING) == 0;
			bool bZCompare = (pstPrimitive->iType & DEBUG_PRIMITIVE_NOZCOMPARE) == 0;

			// set the render states for this primitive
			Renderer::Get().SetBlendMode( bAlpha ? GFX_BLENDMODE_LERP : GFX_BLENDMODE_OVERWRITE );
			Renderer::Get().SetZBufferMode( bZCompare ? (bAlpha ? GFX_ZMODE_LESSEQUAL_READONLY : GFX_ZMODE_LESSEQUAL) : GFX_ZMODE_DISABLED );
			Renderer::Get().SetCullMode( bCull ? GFX_CULLMODE_NORMAL : GFX_CULLMODE_NONE );
			Renderer::Get().SetFillMode( bWireframe ? GFX_FILL_WIREFRAME : GFX_FILL_SOLID );

			// draw the primitives
			switch(iType)
			{
			case DEBUG_PRIMITIVE_SPHERE:
				GetD3DDevice()->DrawIndexedPrimitive(
					D3DPT_TRIANGLESTRIP, 0, 
					m_astStandardPrimitives[DEBUG_PRIMITIVE_SPHERE].iVertexOffset, 
					m_astStandardPrimitives[DEBUG_PRIMITIVE_SPHERE].iNumVertices, 
					m_astStandardPrimitives[DEBUG_PRIMITIVE_SPHERE].iIndexOffset, 
                    m_astStandardPrimitives[DEBUG_PRIMITIVE_SPHERE].iNumIndices - 2
				);
				break;

			case DEBUG_PRIMITIVE_CUBE:
				GetD3DDevice()->DrawIndexedPrimitive(
					D3DPT_TRIANGLELIST, 0, 
					m_astStandardPrimitives[DEBUG_PRIMITIVE_CUBE].iVertexOffset, 
					m_astStandardPrimitives[DEBUG_PRIMITIVE_CUBE].iNumVertices, 
					m_astStandardPrimitives[DEBUG_PRIMITIVE_CUBE].iIndexOffset, 
                    m_astStandardPrimitives[DEBUG_PRIMITIVE_CUBE].iNumIndices/3
				);
				break;

			case DEBUG_PRIMITIVE_CAPSULE:
				{
					CDirection obScale(pstPrimitive->uData.fFloatData, 0.0f, 0.0f);
					Renderer::Get().SetVertexShaderConstant(iZScaleRegister, &obScale, 1);
					GetD3DDevice()->DrawIndexedPrimitive(
						D3DPT_TRIANGLESTRIP, 0, 
						m_astStandardPrimitives[DEBUG_PRIMITIVE_CAPSULE].iVertexOffset, 
						m_astStandardPrimitives[DEBUG_PRIMITIVE_CAPSULE].iNumVertices, 
						m_astStandardPrimitives[DEBUG_PRIMITIVE_CAPSULE].iIndexOffset, 
						m_astStandardPrimitives[DEBUG_PRIMITIVE_CAPSULE].iNumIndices - 2
					);
				}
				break;

			case DEBUG_PRIMITIVE_LINE:
				GetD3DDevice()->DrawPrimitive(
					D3DPT_LINELIST, 
					m_astStandardPrimitives[DEBUG_PRIMITIVE_LINE].iVertexOffset, 
					m_astStandardPrimitives[DEBUG_PRIMITIVE_LINE].iNumVertices/2
				);
				break;

			case DEBUG_PRIMITIVE_POINT:
				GetD3DDevice()->SetRenderState(D3DRS_POINTSIZE, *reinterpret_cast<const uint32_t*>(&pstPrimitive->uData.fFloatData));
				GetD3DDevice()->DrawPrimitive(
					D3DPT_POINTLIST, 
					m_astStandardPrimitives[DEBUG_PRIMITIVE_LINE].iVertexOffset, 
                    1
				);
				break;

			case DEBUG_PRIMITIVE_DIRECTEDLINE:
				{
					// introduce a gap in the line
					float fGap = _R(fmod(CTimer::Get().GetGameTime()*pstPrimitive->uData.fFloatData, 1.0));
					float fGapMin = ntstd::Max(fGap - 0.05f, 0.0f);
					float fGapMax = ntstd::Min(fGap + 0.05f, 1.0f);
					CDirection aobVertices[] = {
						CDirection(CONSTRUCT_CLEAR), 
						CDirection(0.0f, 0.0f, fGapMin), 
						CDirection(0.0f, 0.0f, fGapMax), 
						CDirection(0.0f, 0.0f, 1.0f)
					};

					// render the two line segments
					GetD3DDevice()->DrawPrimitiveUP(D3DPT_LINELIST, 1, &aobVertices[0], sizeof(CDirection));
					GetD3DDevice()->DrawPrimitiveUP(D3DPT_LINELIST, 1, &aobVertices[2], sizeof(CDirection));
					GetD3DDevice()->SetStreamSource(0, m_pobPositions.Get(), 0, 4*sizeof(float));
				}				
				break;

			case DEBUG_PRIMITIVE_GRAPH:
				{
					// Set our vertex format for the graph (float X, float Y)
					Renderer::Get().SetVertexShader( &m_obGraphVertexShader );
					Renderer::Get().m_Platform.SetVertexDeclaration( m_pobGraphVertexDeclaration );

					// get the graph to render itself 
					const CGraph* pobGraph = reinterpret_cast<const CGraph*>(pstPrimitive->pobVertices);
					pobGraph->Render(pstPrimitive->obLocalTransform,obViewportTransform);

					// reset the vertex format and stream source
					Renderer::Get().SetVertexShader( &m_obVertexShader );
					Renderer::Get().m_Platform.SetVertexDeclaration( m_pobVertexDeclaration );
					GetD3DDevice()->SetStreamSource(0, m_pobPositions.Get(), 0, 4*sizeof(float));
				}
				break;

			default: // DEBUG_PRIMITIVE_USER
				{
					ntAssert(pstPrimitive->pobVertices && pstPrimitive->uData.iNumVertices != 0);
					ntAssert((pstPrimitive->iType & DEBUG_PRIMITIVE_TRIANGLELIST) != 0);
					CDirection obScale(CONSTRUCT_CLEAR);
					Renderer::Get().SetVertexShaderConstant(iZScaleRegister, &obScale, 1);
					GetD3DDevice()->DrawPrimitiveUP(
						D3DPT_TRIANGLELIST, pstPrimitive->uData.iNumVertices/3, 
						pstPrimitive->pobVertices, sizeof(CPoint)
					);
					GetD3DDevice()->SetStreamSource(0, m_pobPositions.Get(), 0, 4*sizeof(float));
				}
				break;
			}
		}

		// tidy up the state 
		GetD3DDevice()->SetStreamSource(0, 0, 0, 0);
		GetD3DDevice()->SetIndices(0);

		Renderer::Get().SetFillMode( GFX_FILL_SOLID );

		GetD3DDevice()->SetRenderState(D3DRS_SHADEMODE, D3DSHADE_GOURAUD);
	}
}

/***************************************************************************************************
*
*	FUNCTION		CDebugPrimitives::Flush
*
*	DESCRIPTION		Calls draw then resets the queue
*
***************************************************************************************************/

void DebugPrimitives::Flush(const CCamera* pobCamera, CDirection const& obScale, CDirection const& obOffset)
{
	Draw( pobCamera, obScale, obOffset );
	Reset();
}

/***************************************************************************************************
*
*	FUNCTION		CPrimitiveSorter::operator()
*
*	DESCRIPTION		The comparator used to sort debug primitives via ntstd::sort. Sorts by space, 
*					alpha, Z and wireframe.
*
***************************************************************************************************/

bool DebugPrimitives::CPrimitiveSorter::operator()(const DebugPrimitives::PRIMITIVE* pstLeft, const DebugPrimitives::PRIMITIVE* pstRight) const
{
	// sort on viewspace first
	bool bLeftViewportspace = false;
	if	(
		(pstLeft->iType & DEBUG_PRIMITIVE_VIEWPORTSPACE) ||
		(pstLeft->iType & DEBUG_PRIMITIVE_DISPLAYSPACE)
		)
		bLeftViewportspace = true;

	bool bRightViewportspace = false;
	if	(
		(pstRight->iType & DEBUG_PRIMITIVE_VIEWPORTSPACE) ||
		(pstRight->iType & DEBUG_PRIMITIVE_DISPLAYSPACE)
		)
		bRightViewportspace = true;

	if(bLeftViewportspace != bRightViewportspace)
		return bRightViewportspace;
	
	// if both viewspace then preserve the call order
	if(bLeftViewportspace)
		return pstLeft < pstRight;

	// sort on alpha next
	bool bLeftAlpha = ((pstLeft->dwColour & 0xff000000) != 0xff000000);
	bool bRightAlpha = ((pstRight->dwColour & 0xff000000) != 0xff000000);
	if(bLeftAlpha != bRightAlpha)
		return bRightAlpha;

	// get the distance from the viewer
	float fLeftDistance = m_obViewDirection.Dot(CDirection(pstLeft->obLocalTransform.GetTranslation()));
	float fRightDistance = m_obViewDirection.Dot(CDirection(pstRight->obLocalTransform.GetTranslation()));

	// sort differently depending on whether alpha or not
	if(bLeftAlpha)
	{
		// sort alpha back to front, then on wireframe
		if(fLeftDistance == fRightDistance)
			return (pstRight->iType & DEBUG_PRIMITIVE_WIREFRAME) != 0;
		return fLeftDistance > fRightDistance;
	}
	else
	{
		// sort to render non-z-tested stuff second
		bool bLeftNoZCompare = (pstLeft->iType & DEBUG_PRIMITIVE_NOZCOMPARE) != 0;
		bool bRightNoZCompare = (pstRight->iType & DEBUG_PRIMITIVE_NOZCOMPARE) != 0;

		if(bLeftNoZCompare != bRightNoZCompare)
			return bRightNoZCompare;

		// sort to render wireframe second
		bool bLeftWireframe = (pstLeft->iType & DEBUG_PRIMITIVE_WIREFRAME) != 0;
		bool bRightWireframe = (pstRight->iType & DEBUG_PRIMITIVE_WIREFRAME) != 0;

		if(bLeftWireframe != bRightWireframe)
			return bRightWireframe;

		// to stop Z-fighting sort on pointer values (i.e. the order primitives
		// were added) for exactly equal Z values
		if(fLeftDistance == fRightDistance)
			return pstLeft < pstRight;

		// sort solid front to back, wireframe or non-z-compared back to front
		if(bLeftWireframe || bLeftNoZCompare)
			return fLeftDistance > fRightDistance;
		else
			return fLeftDistance < fRightDistance;
	}
}

