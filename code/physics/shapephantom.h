#ifndef _SHAPEPHANTOM_H
#define _SHAPEPHANTOM_H

#include "Physics/config.h"
#include "Physics/collisionbitfield.h"

class hkShape;
class CEntity;
class Transform;



//------------------------------------------------------------------------------------------
//!
//!	CShapePhantom
//!
//------------------------------------------------------------------------------------------

class CShapePhantom
{
public:

	enum SHAPE_TYPE
	{
		SHAPE_BASE,
		SHAPE_OBB,
		SHAPE_SPHERE,
		SHAPE_CAPSULE,
		SHAPE_PORTAL,
	};

	CShapePhantom ();
	CShapePhantom (const CPoint& obPosition, const CPoint& obOrientation);

	virtual ~CShapePhantom() {};

	virtual void PostConstruct ();
	virtual bool EditorChangeValue(CallBackParameter /*pcItem*/, CallBackParameter /*pcValue*/);
	virtual void DebugRender () {}
	virtual void Render (u_long /*ulFrameColour*/,u_long /*ulBodyColour*/) {}

	// Queries
	virtual SHAPE_TYPE GetType () { return CShapePhantom::SHAPE_BASE; }
	virtual void GetCentre (CPoint& obCentre);
	virtual CPoint GetCentre();
	virtual bool IsInside (const CPoint& obPoint);
	virtual bool IsInside (const CPoint& obStart,const CPoint& obEnd);
	virtual bool IsInside (const CPoint& obStart,const CPoint& obEnd, float& fIntersection);
	virtual void GetIntersecting (ntstd::List<CEntity*>& obIntersectingList);

	// Accessors
	hkShape* GetShape() const {return m_pobShape;}
//	const Transform& GetTransform() const {return *m_pobTransform;}
	const CMatrix& GetLocalMatrix() const {return m_obLocalMatrix;}
	const CPoint& GetPosition() const {return m_obPosition;}
	
	// Setters
	void SetPosition(CPoint& obNewPosition);

	// ----- Serialised members	-----
	CHashedString m_obParentEntity;
	CHashedString m_obParentTransform;

protected:

	Transform* m_pobTransform;
	CPoint m_obPosition;	
	CPoint m_obOrientation;

	CMatrix m_obLocalMatrix;
#ifndef _PS3_RUN_WITHOUT_HAVOK_BUILD
	hkShape* m_pobShape;
	Physics::EntityCollisionFlag m_obEntityCFlag;
#endif
};




//------------------------------------------------------------------------------------------
//!
//!	CShapePhantomOBB
//!
//------------------------------------------------------------------------------------------

class CShapePhantomOBB : public CShapePhantom
{
public:

	HAS_INTERFACE( CShapePhantomOBB );

	CShapePhantomOBB (); // Serialised construction
	CShapePhantomOBB (const CPoint& obPosition, const CPoint& obOrientation, const CDirection& obHalfExtents); // Non-serialised construction
	virtual ~CShapePhantomOBB ();

	virtual void PostConstruct ();
	virtual void DebugRender ();
	virtual void Render (u_long ulFrameColour,u_long ulBodyColour);

	virtual SHAPE_TYPE GetType () { return CShapePhantom::SHAPE_OBB; }

	// ----- Serialised members -----

	CDirection m_obDimensions;
};


//------------------------------------------------------------------------------------------
//!
//!	CShapePhantomOBB
//!
//------------------------------------------------------------------------------------------

class CShapePhantomOBBMinMax : public CShapePhantom
{
public:

	HAS_INTERFACE( CShapePhantomOBBMinMax );

	CShapePhantomOBBMinMax (); // Serialised construction
	CShapePhantomOBBMinMax (const CPoint& obPosition, const CPoint& obOrientation, const CDirection& obHalfExtents); // Non-serialised construction
	virtual ~CShapePhantomOBBMinMax ();

	virtual void PostConstruct ();
	virtual void DebugRender ();
	virtual void Render (u_long ulFrameColour,u_long ulBodyColour);

	virtual SHAPE_TYPE GetType () { return CShapePhantom::SHAPE_OBB; }

	// ----- Serialised members -----

	CPoint m_obMinPoint, m_obMaxPoint;
	CQuat m_obOrientation; // Special orientation Quat for MrEd integration
};






//------------------------------------------------------------------------------------------
//!
//!	CShapePhantomSphere
//!
//------------------------------------------------------------------------------------------

class CShapePhantomSphere : public CShapePhantom
{
public:

	HAS_INTERFACE( CShapePhantomSphere );

	CShapePhantomSphere (); // Serialised construction
	CShapePhantomSphere (const CPoint& obPosition,float fRadius); // Non-serialised construction
	virtual ~CShapePhantomSphere ();

	virtual void PostConstruct ();
	virtual void DebugRender ();
	virtual void Render (u_long ulFrameColour,u_long ulBodyColour);

	virtual SHAPE_TYPE GetType () { return CShapePhantom::SHAPE_SPHERE; }

	// ----- Serialised members -----

	CPoint m_obPosition;
	float m_fRadius;
};




//------------------------------------------------------------------------------------------
//!
//!	CShapePhantomCapsule
//!
//------------------------------------------------------------------------------------------

class CShapePhantomCapsule : public CShapePhantom
{
public:

	HAS_INTERFACE( CShapePhantomCapsule );

	CShapePhantomCapsule (); // Serialised construction
	CShapePhantomCapsule (const CPoint& obPosition,const CPoint& obOrientation,float fLength,float fRadius); // Non-serialised construction
	virtual ~CShapePhantomCapsule ();

	virtual void PostConstruct ();
	virtual void DebugRender ();
	virtual void Render (u_long ulFrameColour,u_long ulBodyColour);

	virtual SHAPE_TYPE GetType () { return CShapePhantom::SHAPE_CAPSULE; }

	// ----- Serialised members -----

	CPoint m_obPosition;
	CPoint m_obOrientation;
	float m_fLength;
	float m_fRadius;
};






//------------------------------------------------------------------------------------------
//!
//!	CShapePhantomPortal
//!
//------------------------------------------------------------------------------------------

class CShapePhantomPortal : public CShapePhantom
{
public:

	HAS_INTERFACE( CShapePhantomPortal );

	CShapePhantomPortal ();
	CShapePhantomPortal (const CPoint& obPosition,const CPoint& obOrientation,float fWidth,float fHeight);
	virtual ~CShapePhantomPortal ();

	virtual void PostConstruct ();
	virtual void DebugRender ();
	virtual void Render (u_long ulFrameColour,u_long ulBodyColour);

	virtual SHAPE_TYPE GetType () { return CShapePhantom::SHAPE_PORTAL; }
	
	// ----- Serialised members -----

	CPoint m_obPosition;
	CPoint m_obOrientation;
	float m_fWidth;
	float m_fHeight;
};

#endif // _SHAPEPHANTOM_H
