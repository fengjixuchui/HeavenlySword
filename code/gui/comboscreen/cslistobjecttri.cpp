/***************************************************************************************************
*
*	DESCRIPTION		This is a will render 3 combo list object at once, basically its a wrapper.
*
*	NOTES			
*
***************************************************************************************************/

#include "cslistobjecttri.h"

CSListObjectTri::CSListObjectTri()
{
	for( int j = 0 ; j < NUMCHILDREN ; ++j )
	{
		m_pChildren[ j ] = NULL;
	}
}

CSListObjectTri::~CSListObjectTri()
{
	DeleteControls();
}

void CSListObjectTri::Render( void )
{
	for( int j = 0 ; j < NUMCHILDREN ; ++j )
	{
		if( NULL != m_pChildren[ j ] )
		{
			m_pChildren[ j ]->Render();
		}
	}
}

void CSListObjectTri::Update( void )
{
	for( int j = 0 ; j < NUMCHILDREN ; ++j )
	{
		if( NULL != m_pChildren[ j ] )
		{
			m_pChildren[ j ]->Update();
		}
	}
}

void CSListObjectTri::SetFirstControl( CSListObjectBase* pFirstControl )
{
	ntAssert( NULL == m_pChildren[ 0 ] );
	m_pChildren[ 0 ] = pFirstControl;
	float fX, fY;
	pFirstControl->GetPosition( fX, fY );
	SetPosition( fX, fY );
}

void CSListObjectTri::SetSecondControl( CSListObjectBase* pSecondControl )
{
	ntAssert( NULL == m_pChildren[ 1 ] );
	m_pChildren[ 1 ] = pSecondControl;
}

void CSListObjectTri::SetThirdControl( CSListObjectBase* pThridControl )
{
	ntAssert( NULL == m_pChildren[ 2 ] );
	m_pChildren[ 2 ] = pThridControl;
}

void CSListObjectTri::DeleteControls( void )
{
	for( int j = 0 ; j < NUMCHILDREN ; ++j )
	{
		if( NULL != m_pChildren[ j ] )
		{
			delete m_pChildren[ j ];
			m_pChildren[ j ] = NULL;
		}
	}
}

void CSListObjectTri::SetWidthAndHeightDetails( void )
{
	//do we have all three items
	if( NULL != m_pChildren[ 0 ] )
	{
		if( NULL != m_pChildren[ 1 ] )
		{
			if( NULL != m_pChildren[ 2 ] )
			{
				//find the extends
				float fHeight1, fHeight2, fHeight3;
				float fWidth;
				m_pChildren[0]->GetSize( fWidth, fHeight1 );
				m_fWidth = fWidth;
				m_pChildren[1]->GetSize( fWidth, fHeight2 );
				m_fWidth += fWidth;
				m_pChildren[2]->GetSize( fWidth, fHeight3 );
				m_fWidth += fWidth;
				if( fHeight1 > fHeight2 && fHeight1 > fHeight3 )
				{
					m_fHeight = fHeight1;
				}
				else if( fHeight2 > fHeight1 && fHeight2 > fHeight3 )
				{
					m_fHeight = fHeight2;
				}
				else if( fHeight3 > fHeight2 && fHeight3 > fHeight1 )
				{
					m_fHeight = fHeight3;
				}
			}
		}
	}
}

void CSListObjectTri::SetControl( CSListObjectBase*	pControl, int iControl )
{
	//this is the interface for beyond the 3 items setting
	ntError( NUMCHILDREN >= iControl );
	m_pChildren[ iControl ] = pControl;
	if( 0 == iControl )
	{
		float fX, fY;
		pControl->GetPosition( fX, fY );
		SetPosition( fX, fY );
	}
}
