/***************************************************************************************************
*
*	DESCRIPTION		This is a image render in the Combo Screen list.
*
*	NOTES			
*
***************************************************************************************************/
#if 0

#ifndef CSLISTOBJECTCOMBOBUTTONS_H
#define CSLISTOBJECTCOMBOBUTTONS_H

#include "effect/screensprite.h"
#include "cslistobjectbase.h"
#include "objectdatabase/dataobject.h"


class CSListObjectComboButtons : public CSListObjectBase
{
public:
	CSListObjectComboButtons( void );
	~CSListObjectComboButtons( void );

	//overides
	virtual void Render( void );
	virtual void Update( void );

	//Construct combo images
	virtual void SetTexture( const char* ccTextureName );

protected:

    ScreenSprite	m_CrossButtonSprites;

}; //end class CSListObjectComboButtons

#endif //CSLISTOBJECTCOMBOBUTTONS_H

#endif
