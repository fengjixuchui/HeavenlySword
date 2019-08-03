/***************************************************************************************************
*
*	DESCRIPTION		This is a will render 3 combo list object at once, basically its a wrapper.
*
*	NOTES			
*
***************************************************************************************************/

#ifndef CSLISTOBJECTTRI_H
#define CSLISTOBJECTTRI_H

#include "cslistobjectbase.h"

#define NUMCHILDREN 14

class CSListObjectTri : public CSListObjectBase
{
public:
	CSListObjectTri();
	~CSListObjectTri();

	virtual void Render( void );
	virtual void Update( void );

	virtual void SetFirstControl( CSListObjectBase*	pFirstControl );
	virtual void SetSecondControl( CSListObjectBase* pSecondControl );
	virtual void SetThirdControl( CSListObjectBase*	pThridControl );
	virtual void SetControl( CSListObjectBase*	pControl, int iControl );

	virtual void DeleteControls( void );

protected:

	void SetWidthAndHeightDetails( void );

	CSListObjectBase* m_pChildren[ NUMCHILDREN ];
}; //end class CSListObjectTri

#endif //CSLISTOBJECTTRI_H
