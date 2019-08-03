/***************************************************************************************************
*
*	DESCRIPTION
*
*	NOTES
*
***************************************************************************************************/

#ifndef MATRIXTWEAKEREDITOR_H
#define MATRIXTWEAKEREDITOR_H

// Necessary includes
#include "camera/matrixtweaker.h"

/***************************************************************************************************
*
*	CLASS			CMatricTweakerEditor
*
*	DESCRIPTION		A base class that allows matrix tweakers to be created and updated using the 
*					pc editor
*
***************************************************************************************************/
class CMatrixTweakerEditor : public CMatrixTweaker
{
protected:

	// This is the item that we are constructing and actually pretending
	// to be - this way we can be partially set up and still deal with errors etc
	CMatrixTweaker* m_pobMatrixTweaker;

public:

	// Construction Destruction
	CMatrixTweakerEditor( void );
	virtual ~CMatrixTweakerEditor( void );

	// We override all the available virtual interfaces, passing data 
	// through to our actual interface if it has been fully constructed
	virtual void			Reset( void );
	virtual CMatrix			ApplyTweak( const CMatrix& obSrc, float fTimeChange );
	virtual void			Render( void );
	virtual void			RenderInfo( int iX, int iY );
	virtual	const char*		Dump( void );
	virtual CMatrix			GetLastSrc( void );
	virtual CMatrix			GetLastTweaked( void );

	// These editor items are owned by the pc editor - not serialised
	virtual bool IsSerialised( void ) const { return false; }

	// Have a go at building the MT as soon as the values are parsed
	virtual void PostConstruct( void ) { CreateTweaker(); }

	// When a data member is changed - if the data is suitable for tweaker creation then we build one
	virtual bool EditorChangeValue( CallBackParameter, CallBackParameter );

	// Members that may be required by all the MTs
	int		m_iParentPad;
	float	m_fMaxLong;
	float	m_fMaxLat;
	bool	m_bPreserveY;
	bool	m_bReverseY;

protected:

	// This must be overridden in order to instantiate this editor
	virtual void CalculateLatLongOffsets( float ) {}

	// To create a tweaker from the data
	virtual void CreateTweaker( void ) = 0;

	// To create the base definition for a MT
	void CreateTweakerDef( CMatrixTweakerDef& obDef );

private:

	// Use this to clear up the tweaker- make sure it is zeroed
	inline void ClearTweaker( void ) { NT_DELETE_CHUNK(Mem::MC_CAMERA, m_pobMatrixTweaker ); m_pobMatrixTweaker = 0; }

};


/***************************************************************************************************
*
*	CLASS			CMTEditorManual
*
*	DESCRIPTION	
*
***************************************************************************************************/

class CMTEditorManual : public CMatrixTweakerEditor
{
public:

	// Construction Destruction
	CMTEditorManual( void );
	virtual ~CMTEditorManual( void );

protected:

	// If the data is ready - make it
	virtual void CreateTweaker( void );
};

/***************************************************************************************************
*
*	CLASS			CMTEditorElastic
*
*	DESCRIPTION		
*
***************************************************************************************************/

class CMTEditorElastic : public CMatrixTweakerEditor
{
public:

	// Construction Destruction
	CMTEditorElastic( void );
	virtual ~CMTEditorElastic( void );

protected:

	// If the data is ready - make it
	virtual void CreateTweaker( void );
};

#endif // MATRIXTWEAKEREDITOR_H
