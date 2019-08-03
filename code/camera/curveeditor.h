/***************************************************************************************************
*
*	DESCRIPTION		Allows the serialisation of different curve types
*
*	NOTES
*
***************************************************************************************************/

#ifndef _CURVEEDITOR_H
#define _CURVEEDITOR_H

// Necessary includes
#include "camera/curveinterface.h"

/***************************************************************************************************
*
*	CLASS			CCurveEditor
*
*	DESCRIPTION		A base class that allows cureves to be created and updated using the pc editor
*
***************************************************************************************************/
class CCurveEditor :  public CCurveInterface
{
protected:

	// This is the item that we are constructing and actually pretending
	// to be - this way we can be partially set up and still deal with errors etc
	CCurveInterface* m_pobCurve;

public:

	// Construction Destruction - must have a default constructor
	CCurveEditor( void );
	virtual ~CCurveEditor( void ); 

	// We override all the available virtual interfaces to a curve, passing data 
	// through to our actual interface if it has been fully constructed
	virtual CVector			Evaluate( float fU )			const;
	virtual CVector			Derivative1( float fU )			const;
	virtual CVector			Derivative2( float fU )			const;
	virtual float			GetLength( void )				const;
	virtual bool			GetOpen( void )					const;
	virtual u_int			GetNumSpan( void )				const;
	virtual u_int			GetNumCV( void )				const;
	virtual const CVector	GetCV( u_int uiIndex )			const;
	virtual void			Render( void )					const;
	virtual void			RenderPath( void )				const;
	virtual void			RenderTans( void )				const;
	virtual void			RenderCVs( void )				const;

	// These editor items are owned by the pc editor - not serialised
	virtual bool IsSerialised( void ) const { return false; }

	// Override the base debug render functionality - remember to call the base method first
	virtual void DebugRender( void );

	// Have a go at building the curve as soon as the values are parsed
	virtual void PostConstruct( void ) { CreateCurve(); }

	// When a data member is changed - if the data is suitable for curve creation then we build one
	virtual bool EditorChangeValue(CallBackParameter, CallBackParameter);

	// If a child member is changed then we can rebuild again
	virtual void EditorChangeParent( void ) { EditorChangeValue( 0, 0 ); }

	// If a child memeber is deleted then we rebuild
	virtual bool EditorDeleteParent( void ) { return EditorChangeValue( 0, 0 ); }

protected:

	// Another internally used virtual function which we need to override - this is only called from
	// our public virtuals so we done actually need it as we route everything elsewhere
	virtual void GetCoeffs( int, CVector* ) const { ntAssert(0); }

	// To create a curve from the data
	virtual void CreateCurve( void ) = 0;

private:

	// Use this to clear up the curve - make sure it is zeroed
	inline void ClearCurve( void ) { NT_DELETE_CHUNK(Mem::MC_CAMERA, m_pobCurve ); m_pobCurve = 0; }

};


/***************************************************************************************************
*
*	CLASS			CCatmullEditor
*
*	DESCRIPTION		
*
***************************************************************************************************/
class CCatmullEditor : public CCurveEditor
{
	typedef ntstd::Vector<CPoint, Mem::MC_CAMERA> ControlVertContainerType;
public:

	// Construction Destruction
	CCatmullEditor( void );
	virtual ~CCatmullEditor( void );

	// What we need to build a Catmull curve
	bool						m_bOpen; 
	bool						m_bDistanceParameterise; 
	ControlVertContainerType	m_obControlVertList;

	// If the data is ready - make the curve
	virtual void CreateCurve( void );

protected:
};


/***************************************************************************************************
*
*	CLASS			CMrEdCurveEditor
*
*	DESCRIPTION		
*
***************************************************************************************************/
class CMrEdCurveEditor : public CCurveEditor
{
public:

	// Construction Destruction
	CMrEdCurveEditor( void );
	virtual ~CMrEdCurveEditor( void );

	// What we need to build a Catmull curve
	bool					m_bOpen; 
	bool					m_bDistanceParameterise; 
	ntstd::Vector<CPoint, Mem::MC_CAMERA>	m_obControlVertList;

	// If the data is ready - make the curve
	virtual void CreateCurve( void );

protected:
};


#endif // _CURVEEDITOR_H
