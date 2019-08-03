/***************************************************************************************************
*
*	DESCRIPTION		This will render the new word and the medalion behind it.
*
*	NOTES			
*
***************************************************************************************************/

#ifndef CSLISTOBJECTNEW_H
#define CSLISTOBJECTNEW_H

#include "cslistobjectbase.h"
#include "effect/screensprite.h"
#include "gui/guitext.h"


class CSListObjectNew : public CSListObjectBase
{
public:
	CSListObjectNew();
	~CSListObjectNew();

	bool Init( void );
	void HasMoved( void );
	void Render( void );

protected:
	CString*		m_pStr;			///< the "new" string
	ScreenSprite	m_Medalion;		///< the gold bit behind it
};

#endif //CSLISTOBJECTNEW_H

