/***************************************************************************************************
*
*	DESCRIPTION		The provides an interface for objects that are to appear in the list
*
*	NOTES			
*
***************************************************************************************************/

//Includes
#include "cslistobjectbase.h"

/***************************************************************************************************
*
*	FUNCTION		CSListObjectBase::CSListObjectBase
*
*	DESCRIPTION		Construction
*
***************************************************************************************************/

CSListObjectBase::CSListObjectBase( void )
: m_pOffsetTransform( NULL )
, m_fX( 0.0f )
, m_fY( 0.0f )
, m_fWidth( 0.0f )
, m_fHeight( 0.0f )
, m_bPosSizeUpdated( false )
{
}

/***************************************************************************************************
*
*	FUNCTION		CSListObjectBase::~CSListObjectBase
*
*	DESCRIPTION		Construction
*
***************************************************************************************************/

CSListObjectBase::~CSListObjectBase( void )
{
	m_pOffsetTransform = NULL;
}

/***************************************************************************************************
*
*	FUNCTION		CSListObjectBase::Render
*
*	DESCRIPTION		Draw children
*
***************************************************************************************************/

void CSListObjectBase::Render( void )
{
}

/***************************************************************************************************
*
*	FUNCTION		CSListObjectBase::Update
*
*	DESCRIPTION		Update children
*
***************************************************************************************************/

void CSListObjectBase::Update( void )
{
}

/***************************************************************************************************
*
*	FUNCTION		CSListObjectBase::HasMoved
*
*	DESCRIPTION		This allows object to move things relative to the transform.
*
***************************************************************************************************/

void CSListObjectBase::HasMoved( void )
{
}

/***************************************************************************************************
*
*	FUNCTION		CSListObjectBase::SetPosition
*
*	DESCRIPTION		This sets the position of the controls
*
***************************************************************************************************/

void CSListObjectBase::SetPosition( const float &fX, const float &fY )
{
	m_fX = fX;
	m_fY = fY;
	m_bPosSizeUpdated = true;
}

/***************************************************************************************************
*
*	FUNCTION		CSListObjectBase::GetPosition
*
*	DESCRIPTION		This gets the position of the controls
*
***************************************************************************************************/

void CSListObjectBase::GetPosition( float &fX, float &fY ) const
{
	fX = m_fX;
	fY = m_fY;
}

/***************************************************************************************************
*
*	FUNCTION		CSListObjectBase::GetWidth
*
*	DESCRIPTION		This sets the size of the control, half size. I.e. control is -w -> w
*
***************************************************************************************************/

void CSListObjectBase::SetSize( const float &fWidth, const float &fHeight )
{
	m_fWidth = fWidth;
	m_fHeight = fHeight;
	m_bPosSizeUpdated = true;
}

/***************************************************************************************************
*
*	FUNCTION		CSListObjectBase::SetWidth
*
*	DESCRIPTION		This gets the size of the controls, half size. I.e. control is -w -> w
*
***************************************************************************************************/

void CSListObjectBase::GetSize( float &fWidth, float &fHeight ) const
{
	fWidth = m_fWidth;
	fHeight = m_fHeight;
}   

/***************************************************************************************************
*
*	FUNCTION		CSListObjectBase::SetTransform
*
*	DESCRIPTION		This set the pointer to the transform the object should base it self upon
*
***************************************************************************************************/

void CSListObjectBase::SetTransform( Transform* pTransform )
{
	m_pOffsetTransform = pTransform;
}

/***************************************************************************************************
*
*	FUNCTION		CSListObjectBase::GetTransform
*
*	DESCRIPTION		This will return the offset pointer.
*
***************************************************************************************************/

Transform* CSListObjectBase::GetTransform( void )  const
{
	return m_pOffsetTransform;
}

/***************************************************************************************************
*
*	FUNCTION		CSListObjectBase::SetSelectableOffset
*
*	DESCRIPTION		This set the offset storage for the selector.
*
***************************************************************************************************/
void CSListObjectBase::SetSelectableOffset( const float &fOffset )
{
	m_fSlecetableOffset = fOffset;
}

/***************************************************************************************************
*
*	FUNCTION		CSListObjectBase::GetSelectableOffset
*
*	DESCRIPTION		This returns the offset storage for the selector.
*
***************************************************************************************************/
float CSListObjectBase::GetSelectableOffset( void ) const
{
	return m_fSlecetableOffset;
}
