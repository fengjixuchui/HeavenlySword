/***************************************************************************************************
*
*	DESCRIPTION		This is a text render in the Combo Screen list.
*
*	NOTES			This is a wrapper for CString to fit into the ListObjectBase framework.
*
***************************************************************************************************/

#ifndef CSLISTOBJECTTEXT_H
#define CSLISTOBJECTTEXT_H

#include "cslistobjectbase.h"
#include "gui/guitext.h"

class CSListObjectText : public CSListObjectBase
{
public:
	CSListObjectText( void );
	virtual ~CSListObjectText( void );

	//overides
	virtual void Render( void );
	virtual void Update( void );

	//specifics
	virtual void SetTextString( const char* cDisplayText, const CStringDefinition::STRING_JUSTIFY_HORIZONTAL eHJustify = CStringDefinition::JUSTIFY_LEFT, const CStringDefinition::STRING_JUSTIFY_VERTICAL eVJustify = CStringDefinition::JUSTIFY_MIDDLE );

protected:
	 CString* m_pStr;		///< the string to dispaly.

}; //end CSListObjectText

#endif //CSLISTOBJECTTEXT_H
