//--------------------------------------------------
//!
//!	\file effect_util.h
//!	handy stuff for effects
//!
//--------------------------------------------------

#ifndef _E_UTIL_H
#define _E_UTIL_H

#include "gfx/graphicsdevice.h"
#include "gfx/vertexdeclaration.h"
#include "game/randmanager.h"

class Transform;
class CEntity;

#define I2FLOAT(member,name)								PUBLISH_VAR_AS( member, name )
#define I2INT(member,name)									PUBLISH_VAR_AS( member, name )
#define I2STRING(member,name)								PUBLISH_VAR_AS( member, name )
#define I2VECTOR(member,name)								PUBLISH_VAR_AS( member, name )
#define I2POINT(member,name)								PUBLISH_VAR_AS( member, name )
#define I2QUAT(member,name)									PUBLISH_VAR_AS( member, name )
#define I2LIGHT(member,name)								PUBLISH_VAR_AS( member, name )
#define I2YPR(member,name)									PUBLISH_VAR_AS( member, name )
#define I2COLOUR(member,name)								PUBLISH_VAR_AS( member, name )
#define I2REFERENCE(member,name)							PUBLISH_PTR_AS( member, name )
#define I2BOOL(member,name)									PUBLISH_VAR_AS( member, name )
#define I2ENUM(member,name, enumname)						PUBLISH_GLOBAL_ENUM_AS( member, name, enumname )
#define I2FLIPFLOP(member,name)								PUBLISH_VAR_AS( member, name )

#define I2REFERENCE_WITH_DEFAULT(member,name, defaultT)		PUBLISH_PTR_WITH_DEFAULT_AS( member, name, defaultT )


//--------------------------------------------------
//!
//!	PlaneDef
//! Art friendly plane definition
//! stored internally as a normal and a distance to the plane
//! along that normal
//!
//--------------------------------------------------
class PlaneDef
{
public:
	PlaneDef();
	virtual ~PlaneDef(){};

	CPoint		m_origin;
	CDirection	m_ypr;

	virtual void DebugRender();
	virtual void PostConstruct();
	virtual bool EditorChangeValue( CallBackParameter, CallBackParameter ) { PostConstruct(); return true; }
	virtual void EditorChangeParent() { PostConstruct(); }

	// retrieve the plane
	CVector		GetPlane() const	{ return m_plane; }
	CDirection	GetNormal() const	{ return CDirection(m_plane); }
	float		GetDistance() const	{ return m_plane.W(); }

	// our definition of how this plane is represented
	static inline void SetFromNormalAndOrigin( CVector& plane, const CDirection& normal, const CPoint& origin )
	{
		plane = CVector( normal );
		plane.W() = normal.Dot( CDirection( origin ) );
	}

	// construct plane from 3 points
	static inline void SetFromPoints( CVector& plane, const CPoint& a, const CPoint& b, const CPoint& c )
	{
		CDirection normal = (c ^ a).Cross(b ^ a);
		normal.Normalise();

		plane = CVector(normal);
		plane.W() = normal.Dot(CDirection(a));
	}

	// Checks to see if the ray defined by these two points intersects the plane, and if so
	// returns the hit fraction along the ray that denotes the intersection point (0->1)
	static inline bool Intersects( const CVector& plane, const CPoint& start, const CPoint& end, float& fHitFraction )
	{
		float fHalfSpaceStart = CPoint(plane).Dot(start) - plane.W();
		float fHalfSpaceEnd = CPoint(plane).Dot(end) - plane.W();

		if	(
			((fHalfSpaceStart > 0.0f) && (fHalfSpaceEnd < 0.0f)) ||
			((fHalfSpaceStart < 0.0f) && (fHalfSpaceEnd > 0.0f))
			)
		{
			float fRayAngle = CPoint(plane).Dot(end - start);
			fHitFraction = (plane.W() - CPoint(plane).Dot(start)) / fRayAngle;
			return true;
		}
		return false;
	}

	// reflect a particle that is moving ballistically
	static inline void BallisticReflect(	CPoint& pos,
											CPoint& intersect,
											CDirection& vel,
											const CDirection& acc,
											const CDirection& normal,
											float fTimeDelta,
											float fHitFraction,
											float fRestitution )
	{
		float fIntersectTime = fTimeDelta * fHitFraction;
		float fRemainderTime = fTimeDelta - fIntersectTime;

		intersect = pos + (vel * fIntersectTime);

		CDirection reflectVel = vel + (acc * fIntersectTime);
		reflectVel = (reflectVel - (normal * (2.0f * normal.Dot(reflectVel)) )) * fRestitution;

		pos = intersect + (reflectVel * fRemainderTime);
		vel = reflectVel + (acc * fRemainderTime);
	}

private:
	CVector		m_plane;
	CMatrix		m_orientation;
};

//--------------------------------------------------
//!
//!	EffectUtils
//!
//--------------------------------------------------
class EffectUtils
{
public:
	static const char* GetInterfaceName( void* ptr );
	static const char* GetInterfaceType( void* ptr );
	static void* GetPtrFromAnyString( void* ptr );

	static IBHandle CreateQuadIndexBuffer(	u_int iNumQuads,
											u_int& iNumIndcies,
											bool bUseTriLists = true );

	static const char* GetTechniqueName( u_int iTechnique )
	{
		switch(iTechnique)
		{
		case 0: return "pointsprite";
		case 1: return "pointsprite_notex";
		case 2: return "pointsprite_ldr";
		case 3: return "pointsprite_notex_ldr";
		
		case 4: return "quadsprite";
		case 5: return "quadsprite_notex";
		case 6: return "quadsprite_ldr";
		case 7: return "quadsprite_notex_ldr";
		}
		ntAssert(0);
		return NULL;
	}

	static CPoint RandomPointInCube()
	{
		CPoint result(CONSTRUCT_CLEAR);
		result.X() = erandf( 2.0f ) - 1.0f;
		result.Y() = erandf( 2.0f ) - 1.0f;
		result.Z() = erandf( 2.0f ) - 1.0f;
		return result;
	}

	static CPoint RandomPointInSphere()
	{
		CPoint result(CONSTRUCT_CLEAR);
		do
		{
			result.X() = erandf( 2.0f ) - 1.0f;
			result.Y() = erandf( 2.0f ) - 1.0f;
			result.Z() = erandf( 2.0f ) - 1.0f;
		}
		while (result.LengthSquared() > 1.0f );
		return result;
	}

	static CPoint RandomPointInCylinder()
	{
		CPoint result(CONSTRUCT_CLEAR);
		do
		{
			result.X() = erandf( 2.0f ) - 1.0f;
			result.Z() = erandf( 2.0f ) - 1.0f;
		}
		while (result.LengthSquared() > 1.0f );
		result.Y() = erandf( 2.0f ) - 1.0f;
		return result;
	}	

	static Transform* FindNamedTransform( const CHashedString& pEntName, const CHashedString& pTransformName );
	static Transform* FindNamedTransform( const CEntity* pEnt, const CHashedString& pTransformName );

	static void SetGlobalFXParameters( ID3DXEffect* pFX );
	static void DebugRenderFrame( const CMatrix& frame, float fScale = 1.0f );
};

#endif
