//--------------------------------------------------------------------------------------------------
/**
	@file		GpBasicPrims.h

	@brief		

	@note		(c) Copyright Sony Computer Entertainment 2004. All Rights Reserved.	
**/
//--------------------------------------------------------------------------------------------------

#ifndef GP_BASIC_PRIMS_H
#define GP_BASIC_PRIMS_H

//--------------------------------------------------------------------------------------------------
//  INCLUDES
//--------------------------------------------------------------------------------------------------

#include	<Fw/FwStd/FwStdIntrusivePtr.h>
#include	<Fw/FwMaths/FwMatrix44.h>

#include	<Fp/FpGeom/FpAABB.h>
#include	<Fp/FpGeom/FpBoundingBox.h>
#include	<Fp/FpGeom/FpCube.h>
#include	<Fp/FpGeom/FpFrustum.h>
#include	<Fp/FpGeom/FpOBB.h>
#include	<Fp/FpGeom/FpSphere.h>

#include	<Gc/GcColour.h>
#include	<Gc/GcContext.h>


//--------------------------------------------------------------------------------------------------
//  FORWARD DECLARATIONS
//--------------------------------------------------------------------------------------------------

class	GcStreamBuffer;
class	GcShader;

typedef FwStd::IntrusivePtr<GcStreamBuffer>	GcStreamBufferHandle;
typedef FwStd::IntrusivePtr<GcShader>		GcShaderHandle;

//--------------------------------------------------------------------------------------------------
/**
	@class		GpBasicPrims

	@brief		contains debug geometric primitive drawing functions
**/
//--------------------------------------------------------------------------------------------------

class	GpBasicPrims	:	FwNonCopyable
{
public:

	enum DrawMode
	{
		kLines,						//!<	plain coloured lines
		kLinesDepthCued,			//!<	lines with a faked-up 'fog' effect to give impressions of depth
	};

	enum BoneMode
	{
		kBoneScaleRelative,			//!<	bone width scales with length
		kBoneScaleAbsolute,			//!<	bone width set absolutely
	};

	enum MatrixType
	{
		kWorldMatrix,
		kViewMatrix,
		kProjectionMatrix,
	};

	// Construction & Destruction
	static	void		Initialise( int pushBufferSize);
	static	void		Shutdown( void );

	// Matrix Accessors
	static	void		SetWorldMatrix( const FwMatrix44& world );				// Model to World
	static	void		SetViewMatrix( const FwMatrix44& view );				// World to View
	static	void		SetProjectionMatrix( const FwMatrix44& projection );	// View to Screen

	static	FwMatrix44	GetWorldMatrix( void );
	static	FwMatrix44	GetViewMatrix( void );
	static	FwMatrix44	GetProjectionMatrix( void );

	static void			SetAllTransforms(	const FwMatrix44&	world,
											const FwMatrix44&	view,
											const FwMatrix44&	projection);
	
	static void			GetAllTransforms(	FwMatrix44&	world,
											FwMatrix44&	view,
											FwMatrix44&	projection);
	
	// Axis Display
	static	void		DrawAxes( float size = 1.0f );

	// Cuboid Display
	static	void		DrawCuboid( const FwPoint* pPoints, GcColour_arg colour = Gc::kColourWhite, DrawMode mode = kLinesDepthCued );
	static	void		DrawCuboid( FwPoint_arg point0, FwPoint_arg point1, GcColour_arg colour = Gc::kColourWhite, DrawMode mode = kLinesDepthCued );
	static	void		DrawCuboid( float halfX, float halfY, float halfZ, GcColour_arg colour = Gc::kColourWhite, DrawMode mode = kLinesDepthCued );

	static	void		DrawCuboid( const FpAABB& cuboid, GcColour_arg colour = Gc::kColourWhite, DrawMode mode = kLinesDepthCued );
	static	void		DrawCuboid( const FpBoundingBox& cuboid, GcColour_arg colour = Gc::kColourWhite, DrawMode mode = kLinesDepthCued );
	static	void		DrawCuboid( const FpCube& cuboid, GcColour_arg colour = Gc::kColourWhite, DrawMode mode = kLinesDepthCued );
	static	void		DrawCuboid( const FpOBB& cuboid, GcColour_arg colour = Gc::kColourWhite, DrawMode mode = kLinesDepthCued );

	// Sphere Display
	static	void		DrawSphere( FwPoint_arg position, float radius, GcColour_arg colour = Gc::kColourWhite, DrawMode mode = kLinesDepthCued );
	static	void		DrawSphere( FpSphere_arg sphere, GcColour_arg colour = Gc::kColourWhite, DrawMode mode = kLinesDepthCued );

	// Frustum Display
	static	void		DrawFrustum( const FpFrustum& frustum, GcColour_arg colour = Gc::kColourWhite, DrawMode mode = kLinesDepthCued );
	static	void		DrawFrustum( const FpFrustumTransposed& frustum, GcColour_arg colour = Gc::kColourWhite, DrawMode mode = kLinesDepthCued );

	// Other Primitives
	static	void		DrawCylinder( float radius, float halfHeight, GcColour_arg colour = Gc::kColourWhite, DrawMode mode = kLinesDepthCued );
	static	void		DrawCapsule( float radius, float halfHeight, GcColour_arg colour = Gc::kColourWhite, DrawMode mode = kLinesDepthCued );
	static	void		DrawCone( float radius, float halfHeight, GcColour_arg colour = Gc::kColourWhite, DrawMode mode = kLinesDepthCued );
	static	void		DrawBone( FwVector_arg localOffset, float scale = 0.1f, BoneMode boneMode = kBoneScaleRelative, GcColour_arg colour = Gc::kColourWhite, DrawMode mode = kLinesDepthCued );
	static	void		DrawTriangle( FwPoint_arg point0, FwPoint_arg point1, FwPoint_arg point2, GcColour_arg colour = Gc::kColourWhite, DrawMode mode = kLinesDepthCued );
	static	void		DrawLine( FwPoint_arg point0, FwPoint_arg point1, GcColour_arg colour = Gc::kColourWhite, DrawMode mode = kLinesDepthCued );
	static	void		DrawDirectedLine( FwPoint_arg point0, FwPoint_arg point1, GcColour_arg colour = Gc::kColourWhite );

	static	void		Flush( void );

private:

	enum PrecomputedPrimType
	{
		kCuboid = 0,
		kSphere,
		kCylinder,
		kCapsule,
		kCone,
		kBone,
		kTriangle,
		kLine,
		kNumPrecomputedPrimTypes
	};

	static	void		CreatePrimGeometry();
	static	int			CreateCuboid(FwVector4* pDst);
	static	int			CreateSphere(FwVector4* pDst);
	static	int			CreateCylinder(FwVector4* pDst);
	static	int			CreateCapsule(FwVector4* pDst);
	static	int			CreateCone(FwVector4* pDst);
	static	int			CreateBone(FwVector4* pDst);
	static	int			CreateTriangle(FwVector4* pDst);
	static	int			CreateLine(FwVector4* pDst);
	static	void		DrawPrim( PrecomputedPrimType type, GcColour_arg colour, DrawMode mode, FwTransform& local, float yScale = 0.0f );

	static	float		CircleSin( int i )
	{
		return ms_sinTable[ i & (kNumCircleSegments-1) ];
	}

	static	float		CircleCos( int i )
	{
		return ms_sinTable[ (i+(kNumCircleSegments/4)) & (kNumCircleSegments-1) ];
	}

	static bool			GetFreeSpaceCallback( Ice::Render::CommandContextData* pBuffer, uint size );

	static	const int	kNumCircleSegments	=	32;

	struct PrimStream
	{
		u16		m_offset;
		u16		m_count;
	};

	static	GcShaderHandle			ms_hVertexShader;
	static	GcShaderHandle			ms_hFragmentShader;

	static	FwMatrix44	ms_modelToWorld;					//!<	Points are transformed into world space by this..
	static	FwMatrix44	ms_worldToView;						//!<	..then from world into view space by this..
	static	FwMatrix44	ms_viewToProjection;				//!<	..then into screen space by this.

	static	FwMatrix44	ms_worldView;						//!<	Transform from model space -> view space
	static	FwMatrix44	ms_worldViewProj;					//!<	Transform from model space -> screen space

	static	float		ms_sinTable[kNumCircleSegments];	//!<	Precomputed sin table for circles

	static	bool		ms_matricesInvalid;					//!<	true if our world->view and world->view->proj matrices need updating.
	
	static	u64			ms_lastFlushFrame;					//!<	The last frame that we were flushed on
	static	bool		ms_startedContext;					//!<	True if the context has been started
	static	void*		ms_contextStart;					//!<	The context start address

	static	PrimStream	ms_primStreamInfo[kNumPrecomputedPrimTypes];
	static	GcStreamBufferHandle ms_hPrimStream;			//!<	Vertex stream for all precomputed primitives

	static	uint		ms_matrixConstIdx;					//!<	Shader constant index
	static	uint		ms_localMatrixConstIdx;				//!<	Shader constant index
	static	uint		ms_viewZConstIdx;					//!<	Shader constant index
	static	uint		ms_colourConstIdx;					//!<	Shader constant index
	static	uint		ms_yScaleConstIdx;					//!<	Shader constant index

	static	uint		ms_contextSize;						//!<	Rendering context memory
	static	u8*			ms_pContextMem;						//!<	Rendering context memory
	static	GcContext	ms_context;							//!<	Rendering context
};	


//--------------------------------------------------------------------------------------------------
//  INLINE FUNCTION DEFINITIONS
//--------------------------------------------------------------------------------------------------

inline	void		GpBasicPrims::SetWorldMatrix( const FwMatrix44& world )
{
	ms_modelToWorld		= world;
	ms_matricesInvalid	= true;
}

inline	void		GpBasicPrims::SetViewMatrix( const FwMatrix44& view )
{
	ms_worldToView = view;
	ms_matricesInvalid	= true;
}

inline	void		GpBasicPrims::SetProjectionMatrix( const FwMatrix44& projection )
{
	ms_viewToProjection = projection;
	ms_matricesInvalid	= true;
}

inline	FwMatrix44	GpBasicPrims::GetWorldMatrix( void )
{
	return ms_modelToWorld;
}

inline	FwMatrix44	GpBasicPrims::GetViewMatrix( void )
{
	return ms_worldToView;
}

inline	FwMatrix44	GpBasicPrims::GetProjectionMatrix( void )
{
	return ms_viewToProjection;
}

inline void	GpBasicPrims::SetAllTransforms(	const FwMatrix44&	world,
											const FwMatrix44&	view,
											const FwMatrix44&	projection)
{

	ms_modelToWorld		= world;
	ms_worldToView		= view;
	ms_viewToProjection	= projection;
	ms_matricesInvalid	= true;
}

inline void	GpBasicPrims::GetAllTransforms(	FwMatrix44&	world,
											FwMatrix44&	view,
											FwMatrix44&	projection)
{

	world		= ms_modelToWorld;
	view		= ms_worldToView;
	projection	= ms_viewToProjection;
}

//--------------------------------------------------------------------------------------------------

#endif // GP_BASIC_PRIMS_H
